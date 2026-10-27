// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "render/opengl/OpenGLRenderBackend.h"
#include "scene/Scene.h"

#include <QFile>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QSurfaceFormat>
#include <QThread>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
// A backend render may be invoked while Qt already owns a current context.
// Always restore it, including when shader compilation or allocation fails.
class CurrentContextScope
{
public:
    CurrentContextScope(QOpenGLContext& context, QSurface& surface)
        : context_(context),
          previous_(QOpenGLContext::currentContext()),
          previousSurface_(previous_ ? previous_->surface() : nullptr)
    {
        if (context.thread() != QThread::currentThread() || !context.makeCurrent(&surface))
        {
            throw std::runtime_error(
                "OpenGL: cannot make the rendering context current on this thread.");
        }
    }

    ~CurrentContextScope()
    {
        context_.doneCurrent();

        if (previous_ && previousSurface_)
        {
            previous_->makeCurrent(previousSurface_);
        }
    }

private:
    QOpenGLContext& context_;
    QOpenGLContext* previous_;
    QSurface* previousSurface_;
};

struct GpuVertex
{
    float position[3];
    float normal[3];
    float uv[2];
};

bool ValidIndex(int index, std::size_t size)
{
    return index >= 0 && static_cast<std::size_t>(index) < size;
}

// QOpenGLFunctions_3_3_Core internally references data owned by an OpenGL
// context. Keep the context in an earlier base class so that C++ destroys
// QOpenGLFunctions_3_3_Core first and the context afterwards.
class OpenGLContextOwner
{
protected:
    QOpenGLContext context;
    QOffscreenSurface surface;
};
} // namespace

namespace buffalo
{
class OpenGLRenderBackend::Impl
    : private OpenGLContextOwner,
      public QOpenGLFunctions_3_3_Core
{
public:
    struct GpuMesh
    {
        GLuint vertexArray = 0;
        GLuint vertexBuffer = 0;
        GLsizei vertexCount = 0;
        ObjectId modelId = 0;
        std::uint64_t revision = 0;
        std::uint64_t textureRevision = 0;
        GLuint textures[2]{};
    };

    void Initialize(QOpenGLContext* shareContext)
    {
        if (!shareContext || !shareContext->isValid() || shareContext->isOpenGLES())
        {
            throw std::runtime_error(
                "OpenGL: a valid desktop OpenGL sharing context is required.");
        }

        if (shareContext->thread() != QThread::currentThread())
        {
            throw std::runtime_error(
                "OpenGL: initialize on the viewport context's thread.");
        }

        QSurfaceFormat format = shareContext->format();
        format.setVersion(3, 3);
        format.setProfile(QSurfaceFormat::CoreProfile);

        context.setFormat(format);
        context.setShareContext(shareContext);

        if (!context.create() ||
            !QOpenGLContext::areSharing(&context, shareContext))
        {
            throw std::runtime_error(
                "OpenGL: failed to create a shared OpenGL 3.3 context.");
        }

        surface.setFormat(context.format());
        surface.create();

        if (!surface.isValid())
        {
            throw std::runtime_error(
                "OpenGL: failed to create the offscreen surface.");
        }

        CurrentContextScope current(context, surface);

        if (!initializeOpenGLFunctions())
        {
            throw std::runtime_error(
                "OpenGL: OpenGL 3.3 core functions are unavailable.");
        }

        functionsReady = true;

        program = CreateProgram();

        modelLocation = glGetUniformLocation(
            program,
            "model");

        viewProjectionLocation = glGetUniformLocation(
            program,
            "viewProjection");

        if (modelLocation < 0 || viewProjectionLocation < 0)
        {
            throw std::runtime_error(
                "OpenGL: mesh shader is missing required matrix uniforms.");
        }

        GLint maxTexture = 0;
        GLint maxRenderbuffer = 0;

        glGetIntegerv(
            GL_MAX_TEXTURE_SIZE,
            &maxTexture);

        glGetIntegerv(
            GL_MAX_RENDERBUFFER_SIZE,
            &maxRenderbuffer);

        maxTargetSize = std::min(
            maxTexture,
            maxRenderbuffer);
    }

    ~Impl()
    {
        if (!functionsReady ||
            !context.isValid() ||
            !surface.isValid())
        {
            return;
        }

        try
        {
            CurrentContextScope current(context, surface);

            ClearMeshes();
            DestroyTarget();

            if (program != 0)
            {
                glDeleteProgram(program);
                program = 0;
            }
        }
        catch (...)
        {
            // Context loss cannot be repaired during destruction.
            // Remaining driver resources disappear with the share group.
        }
    }

    RenderFrame Render(Scene& scene, int width, int height)
    {
        if (width <= 0 || height <= 0)
        {
            return {};
        }

        if (width > maxTargetSize || height > maxTargetSize)
        {
            throw std::runtime_error(
                "OpenGL: requested viewport exceeds the device's render target limit.");
        }

        CurrentContextScope current(context, surface);

        ResizeTarget(width, height);

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            framebuffer);

        glViewport(
            0,
            0,
            width,
            height);

        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_FRAMEBUFFER_SRGB);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glClearColor(
            37.0f / 255.0f,
            40.0f / 255.0f,
            45.0f / 255.0f,
            1.0f);

        glClearDepth(1.0);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

        std::unordered_set<const Mesh*> usedMeshes;

        Camera* camera = scene.getCurrentCamera();

        if (camera)
        {
            glUseProgram(program);
            const auto lights = scene.Lighting();
            const int lightCount = static_cast<int>(std::min<std::size_t>(lights.size(), 64));
            glUniform1i(glGetUniformLocation(program, "lightCount"), lightCount);
            for (int i = 0; i < lightCount; ++i)
            {
                const auto& light = lights[i];
                const auto suffix = "[" + std::to_string(i) + "]";
                glUniform3f(glGetUniformLocation(program, ("lightDirections" + suffix).c_str()), light.directionToLight.x, light.directionToLight.y, light.directionToLight.z);
                glUniform3f(glGetUniformLocation(program, ("lightRadiance" + suffix).c_str()), light.radiance.x, light.radiance.y, light.radiance.z);
            }

            const Matrix4 viewProjection =
                camera->projectionMatrix *
                camera->viewMatrix;

            // Matrix4 is row-major; GLSL matrices use column-major storage.
            glUniformMatrix4fv(
                viewProjectionLocation,
                1,
                GL_TRUE,
                viewProjection._data);

            auto* models = scene.GetVisibleModels();

            while (!models->empty())
            {
                Model* model = models->front();
                models->pop();

                if (!model || !model->GetMesh())
                {
                    continue;
                }

                const Mesh* mesh = model->GetMesh();

                usedMeshes.insert(mesh);

                auto it = meshes.find(mesh);
                if (it != meshes.end() && (it->second.revision != model->MeshRevision() || it->second.modelId != model->Id()))
                {
                    DeleteMesh(it->second);
                    meshes.erase(it);
                    it = meshes.end();
                }

                if (it == meshes.end())
                {
                    GpuMesh uploaded = UploadMesh(*mesh);
                    uploaded.revision = model->MeshRevision();
                    uploaded.modelId = model->Id();

                    try
                    {
                        it = meshes.emplace(
                                       mesh,
                                       uploaded)
                                 .first;
                    }
                    catch (...)
                    {
                        DeleteMesh(uploaded);
                        throw;
                    }
                }

                BindMaterial(it->second, *model->GetMaterial());
                glUniformMatrix4fv(
                    modelLocation,
                    1,
                    GL_TRUE,
                    model->GetModelMatrix()._data);

                glBindVertexArray(
                    it->second.vertexArray);

                glDrawArrays(
                    GL_TRIANGLES,
                    0,
                    it->second.vertexCount);
            }

            glBindVertexArray(0);
            glUseProgram(0);
        }

        // Bound cache size to the current scene.
        // Transform-only rigid bodies do not re-upload vertices.
        for (auto it = meshes.begin(); it != meshes.end();)
        {
            if (usedMeshes.find(it->first) == usedMeshes.end())
            {
                DeleteMesh(it->second);
                it = meshes.erase(it);
            }
            else
            {
                ++it;
            }
        }

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            0);

        // Synchronous v1 contract. The producer texture is complete before
        // the presentation context consumes it.
        glFinish();

        const GLenum error = glGetError();

        if (error != GL_NO_ERROR)
        {
            throw std::runtime_error(
                "OpenGL: rendering failed, error " +
                std::to_string(error));
        }

        RenderFrame output;

        output.width = width;
        output.height = height;
        output.format = PixelFormat::RGBA8;
        output.storage = FrameStorage::OpenGLTexture;
        output.textureId = colorTexture;
        output.graphicsDevice = context.shareGroup();

        return output;
    }

    void InvalidateMeshCache()
    {
        CurrentContextScope current(context, surface);
        ClearMeshes();
    }

private:
    GLuint CompileShader(
        GLenum stage,
        const char* resourcePath)
    {
        QFile file(
            QString::fromUtf8(resourcePath));

        if (!file.open(QIODevice::ReadOnly))
        {
            throw std::runtime_error(
                std::string("OpenGL: cannot load shader ") +
                resourcePath);
        }

        const QByteArray source = file.readAll();
        const GLchar* data = source.constData();

        const GLuint shader =
            glCreateShader(stage);

        glShaderSource(
            shader,
            1,
            &data,
            nullptr);

        glCompileShader(shader);

        GLint success = GL_FALSE;

        glGetShaderiv(
            shader,
            GL_COMPILE_STATUS,
            &success);

        if (!success)
        {
            GLint size = 0;

            glGetShaderiv(
                shader,
                GL_INFO_LOG_LENGTH,
                &size);

            std::vector<char> log(
                static_cast<std::size_t>(
                    std::max(size, 1)),
                '\0');

            glGetShaderInfoLog(
                shader,
                static_cast<GLsizei>(log.size()),
                nullptr,
                log.data());

            glDeleteShader(shader);

            throw std::runtime_error(
                std::string("OpenGL shader ") +
                resourcePath +
                ": " +
                log.data());
        }

        return shader;
    }

    GLuint CreateProgram()
    {
        GLuint vertex = 0;
        GLuint fragment = 0;
        GLuint result = 0;

        try
        {
            vertex = CompileShader(
                GL_VERTEX_SHADER,
                ":/buffalo/shaders/opengl/mesh.vert");

            fragment = CompileShader(
                GL_FRAGMENT_SHADER,
                ":/buffalo/shaders/opengl/diffuse.frag");

            result = glCreateProgram();

            glAttachShader(
                result,
                vertex);

            glAttachShader(
                result,
                fragment);

            glLinkProgram(result);

            GLint success = GL_FALSE;

            glGetProgramiv(
                result,
                GL_LINK_STATUS,
                &success);

            if (!success)
            {
                GLint size = 0;

                glGetProgramiv(
                    result,
                    GL_INFO_LOG_LENGTH,
                    &size);

                std::vector<char> log(
                    static_cast<std::size_t>(
                        std::max(size, 1)),
                    '\0');

                glGetProgramInfoLog(
                    result,
                    static_cast<GLsizei>(log.size()),
                    nullptr,
                    log.data());

                throw std::runtime_error(
                    std::string(
                        "OpenGL mesh program link: ") +
                    log.data());
            }

            glDetachShader(
                result,
                vertex);

            glDetachShader(
                result,
                fragment);

            glDeleteShader(vertex);
            glDeleteShader(fragment);

            return result;
        }
        catch (...)
        {
            if (vertex != 0)
            {
                glDeleteShader(vertex);
            }

            if (fragment != 0)
            {
                glDeleteShader(fragment);
            }

            if (result != 0)
            {
                glDeleteProgram(result);
            }

            throw;
        }
    }

    GpuMesh UploadMesh(const Mesh& mesh)
    {
        std::vector<GpuVertex> vertices;

        const std::size_t faceCount = std::min(
            mesh.vertexIndices.size(),
            static_cast<std::size_t>(
                std::max(mesh.numFaces, 0)));

        if (faceCount >
            static_cast<std::size_t>(
                std::numeric_limits<GLsizei>::max()) /
                3)
        {
            throw std::runtime_error(
                "OpenGL: mesh is too large for one draw.");
        }

        vertices.reserve(
            faceCount * 3);

        for (std::size_t face = 0;
            face < faceCount;
            ++face)
        {
            const Vector3i indices =
                mesh.vertexIndices[face];

            if (!ValidIndex(indices.x, mesh.vertices.size()) ||
                !ValidIndex(indices.y, mesh.vertices.size()) ||
                !ValidIndex(indices.z, mesh.vertices.size()))
            {
                continue;
            }

            const Vector3 p0 =
                mesh.vertices[indices.x];

            const Vector3 p1 =
                mesh.vertices[indices.y];

            const Vector3 p2 =
                mesh.vertices[indices.z];

            const Vector3 faceNormal =
                Vector3::Cross(
                    p1 - p0,
                    p2 - p0)
                    .Normalized();

            for (int corner = 0;
                corner < 3;
                ++corner)
            {
                const Vector3& position =
                    mesh.vertices[indices[corner]];

                Vector3 normal =
                    faceNormal;

                if (face < mesh.normalsIndices.size() &&
                    ValidIndex(
                        mesh.normalsIndices[face][corner],
                        mesh.normals.size()))
                {
                    normal =
                        mesh.normals[mesh.normalsIndices[face][corner]];
                }

                Vector3 uv;
                if (face < mesh.uvIndices.size() && ValidIndex(mesh.uvIndices[face][corner], mesh.uv.size()))
                {
                    uv = mesh.uv[mesh.uvIndices[face][corner]];
                }
                vertices.push_back(
                    {{position.x,
                         position.y,
                         position.z},
                        {normal.x,
                            normal.y,
                            normal.z},
                        {uv.x,
                            uv.y}});
            }
        }

        GpuMesh result;

        glGenVertexArrays(
            1,
            &result.vertexArray);

        glGenBuffers(
            1,
            &result.vertexBuffer);

        glBindVertexArray(
            result.vertexArray);

        glBindBuffer(
            GL_ARRAY_BUFFER,
            result.vertexBuffer);

        glBufferData(
            GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(
                vertices.size() *
                sizeof(GpuVertex)),
            vertices.data(),
            GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);

        glVertexAttribPointer(
            0,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(GpuVertex),
            nullptr);

        glEnableVertexAttribArray(1);

        glVertexAttribPointer(
            1,
            3,
            GL_FLOAT,
            GL_FALSE,
            sizeof(GpuVertex),
            reinterpret_cast<const void*>(
                offsetof(
                    GpuVertex,
                    normal)));

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GpuVertex), reinterpret_cast<const void*>(offsetof(GpuVertex, uv)));
        glBindVertexArray(0);

        glBindBuffer(
            GL_ARRAY_BUFFER,
            0);

        const GLenum error =
            glGetError();

        if (error != GL_NO_ERROR)
        {
            DeleteMesh(result);

            throw std::runtime_error(
                "OpenGL: mesh upload failed, error " +
                std::to_string(error));
        }

        result.vertexCount =
            static_cast<GLsizei>(
                vertices.size());

        return result;
    }

    void BindMaterial(GpuMesh& mesh, Material& material)
    {
        const auto params = material.GetShaderParameters();
        const Texture2D* sources[2]{params.baseColorTexture, params.normalTexture};
        if (mesh.textureRevision != material.TextureRevision())
        {
            glDeleteTextures(2, mesh.textures);
            mesh.textures[0] = mesh.textures[1] = 0;
            for (int i = 0; i < 2; ++i)
            {
                if (!sources[i])
                {
                    continue;
                }
                glActiveTexture(GL_TEXTURE0 + i);
                glGenTextures(1, &mesh.textures[i]);
                glBindTexture(GL_TEXTURE_2D, mesh.textures[i]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, sources[i]->GetWidth(), sources[i]->GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, sources[i]->Data());
            }
            mesh.textureRevision = material.TextureRevision();
        }
        for (int i = 0; i < 2; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, mesh.textures[i]);
        }
        glUniform1i(glGetUniformLocation(program, "baseTexture"), 0);
        glUniform1i(glGetUniformLocation(program, "normalTexture"), 1);
        glUniform1i(glGetUniformLocation(program, "hasBaseTexture"), sources[0] != nullptr);
        glUniform1i(glGetUniformLocation(program, "hasNormalTexture"), sources[1] != nullptr);
        glUniform1i(glGetUniformLocation(program, "shaderKind"), static_cast<int>(material.GetShaderKind()));
        glUniform3f(glGetUniformLocation(program, "baseColor"), material.baseColor.x, material.baseColor.y, material.baseColor.z);
        glActiveTexture(GL_TEXTURE0);
    }

    void DeleteMesh(const GpuMesh& mesh)
    {
        glDeleteTextures(2, mesh.textures);
        if (mesh.vertexBuffer != 0)
        {
            glDeleteBuffers(
                1,
                &mesh.vertexBuffer);
        }

        if (mesh.vertexArray != 0)
        {
            glDeleteVertexArrays(
                1,
                &mesh.vertexArray);
        }
    }

    void ClearMeshes()
    {
        for (const auto& item : meshes)
        {
            DeleteMesh(
                item.second);
        }

        meshes.clear();
    }

    void ResizeTarget(int width, int height)
    {
        if (width == targetWidth &&
            height == targetHeight)
        {
            return;
        }

        DestroyTarget();

        glGenTextures(
            1,
            &colorTexture);

        glBindTexture(
            GL_TEXTURE_2D,
            colorTexture);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_LINEAR);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_LINEAR);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE);

        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA8,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr);

        glGenRenderbuffers(
            1,
            &depthBuffer);

        glBindRenderbuffer(
            GL_RENDERBUFFER,
            depthBuffer);

        glRenderbufferStorage(
            GL_RENDERBUFFER,
            GL_DEPTH_COMPONENT24,
            width,
            height);

        glGenFramebuffers(
            1,
            &framebuffer);

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            framebuffer);

        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            colorTexture,
            0);

        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            depthBuffer);

        const GLenum status =
            glCheckFramebufferStatus(
                GL_FRAMEBUFFER);

        glBindTexture(
            GL_TEXTURE_2D,
            0);

        glBindRenderbuffer(
            GL_RENDERBUFFER,
            0);

        glBindFramebuffer(
            GL_FRAMEBUFFER,
            0);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            DestroyTarget();

            throw std::runtime_error(
                "OpenGL: incomplete framebuffer, status " +
                std::to_string(status));
        }

        targetWidth = width;
        targetHeight = height;
    }

    void DestroyTarget()
    {
        if (framebuffer != 0)
        {
            glDeleteFramebuffers(
                1,
                &framebuffer);
        }

        if (depthBuffer != 0)
        {
            glDeleteRenderbuffers(
                1,
                &depthBuffer);
        }

        if (colorTexture != 0)
        {
            glDeleteTextures(
                1,
                &colorTexture);
        }

        framebuffer = 0;
        depthBuffer = 0;
        colorTexture = 0;

        targetWidth = 0;
        targetHeight = 0;
    }

    bool functionsReady = false;

    GLuint program = 0;

    GLint modelLocation = -1;
    GLint viewProjectionLocation = -1;
    GLint maxTargetSize = 0;

    GLuint framebuffer = 0;
    GLuint colorTexture = 0;
    GLuint depthBuffer = 0;

    int targetWidth = 0;
    int targetHeight = 0;

    std::unordered_map<const Mesh*, GpuMesh> meshes;
};

OpenGLRenderBackend::OpenGLRenderBackend(QOpenGLContext* shareContext)
    : impl_(std::make_unique<Impl>())
{
    impl_->Initialize(
        shareContext);
}

OpenGLRenderBackend::~OpenGLRenderBackend() = default;

const char* OpenGLRenderBackend::Name() const
{
    return "OpenGL";
}

RenderFrame OpenGLRenderBackend::Render(
    Scene& scene,
    int width,
    int height)
{
    return impl_->Render(
        scene,
        width,
        height);
}

void OpenGLRenderBackend::InvalidateMeshCache()
{
    impl_->InvalidateMeshCache();
}
} // namespace buffalo
