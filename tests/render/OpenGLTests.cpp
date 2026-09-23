// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

#include "core/EngineRuntime.h"
#include "render/opengl/OpenGLRenderBackend.h"

#include <QGuiApplication>
#include <QImage>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QSurfaceFormat>

#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

using namespace buffalo;

namespace
{
void Require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

struct Image
{
    int width = 0;
    int height = 0;
    // Validation-only readback, RGBA bottom-to-top. Production frames remain on the GPU.
    std::vector<unsigned char> rgba;

    std::uint64_t Hash() const
    {
        std::uint64_t result = 1469598103934665603ull;
        for (const auto channel : rgba)
        {
            result ^= channel;
            result *= 1099511628211ull;
        }
        return result;
    }

    std::size_t ShadedPixels() const
    {
        std::size_t count = 0;
        for (std::size_t pixel = 0; pixel < rgba.size(); pixel += 4)
        {
            if (rgba[pixel] != 37 || rgba[pixel + 1] != 40 || rgba[pixel + 2] != 45)
            {
                ++count;
            }
        }
        return count;
    }

    void Save(const QString& path) const
    {
        const QImage bottomUp(rgba.data(), width, height, width * 4, QImage::Format_RGBA8888);
        Require(bottomUp.mirrored().save(path), "Could not save the requested GPU capture image");
    }
};

Image ReadFrame(QOpenGLFunctions_3_3_Core& gl, const RenderFrame& frame, QOpenGLContext& context)
{
    Require(frame.IsValid() && frame.storage == FrameStorage::OpenGLTexture && frame.pixels == nullptr,
        "OpenGL must deliver a GPU texture without CPU frame pixels");
    Require(frame.graphicsDevice == context.shareGroup(), "Returned texture must belong to the presentation share group");
    Require(gl.glIsTexture(frame.textureId) == GL_TRUE, "Borrowed output texture must be alive while consumed");

    GLint oldTexture = 0;
    gl.glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
    gl.glBindTexture(GL_TEXTURE_2D, frame.textureId);
    GLint width = 0;
    GLint height = 0;
    gl.glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    gl.glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
    gl.glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(oldTexture));
    Require(width == frame.width && height == frame.height, "Output metadata must match actual GPU texture dimensions");

    GLint previousFramebuffer = 0;
    gl.glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);
    GLuint framebuffer = 0;
    gl.glGenFramebuffers(1, &framebuffer);
    gl.glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    gl.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame.textureId, 0);
    const bool complete = gl.glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    Image result;
    if (complete)
    {
        result.width = width;
        result.height = height;
        result.rgba.resize(static_cast<std::size_t>(width) * height * 4);
        gl.glReadBuffer(GL_COLOR_ATTACHMENT0);
        gl.glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, result.rgba.data());
    }
    gl.glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previousFramebuffer));
    gl.glDeleteFramebuffers(1, &framebuffer);
    Require(complete, "Shared output texture must be attachable in the presentation context");
    Require(gl.glGetError() == GL_NO_ERROR, "GPU frame validation must not generate OpenGL errors");
    return result;
}

void CheckCurrent(QOpenGLContext& context, QOffscreenSurface& surface)
{
    Require(QOpenGLContext::currentContext() == &context && context.surface() == &surface,
        "Backend operations must restore the caller's context and surface");
}
} // namespace

int main(int argc, char* argv[])
{
#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
    std::cerr << "Creating QGuiApplication...\n";
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QSurfaceFormat requested;
    requested.setRenderableType(QSurfaceFormat::OpenGL);
    requested.setVersion(3, 3);
    requested.setProfile(QSurfaceFormat::CoreProfile);
    requested.setDepthBufferSize(24);
    requested.setSamples(0);
    QSurfaceFormat::setDefaultFormat(requested);
    QGuiApplication application(argc, argv);
    std::cerr << "QGuiApplication ready.\n";
    try
    {
        QString capturePath;
        const auto arguments = application.arguments();
        const int captureIndex = arguments.indexOf("--capture");
        if (captureIndex >= 0)
        {
            Require(captureIndex + 1 < arguments.size(), "--capture requires a PNG output path");
            capturePath = arguments[captureIndex + 1];
        }

        std::cerr << "Creating desktop OpenGL context...\n";
        QOpenGLContext context;
        context.setFormat(requested);
        Require(context.create() && context.isValid() && !context.isOpenGLES(),
            "GPU test requires an available desktop OpenGL 3.3 context; context creation failed");
        std::cerr << "Creating offscreen surface...\n";
        QOffscreenSurface surface;
        surface.setFormat(context.format());
        surface.create();
        Require(surface.isValid() && context.makeCurrent(&surface),
            "GPU test requires a valid offscreen surface and current context");
        QOpenGLFunctions_3_3_Core gl;
        Require(gl.initializeOpenGLFunctions(), "GPU test requires OpenGL 3.3 core functions");
        std::cout << "OpenGL device: " << reinterpret_cast<const char*>(gl.glGetString(GL_RENDERER))
                  << " / " << reinterpret_cast<const char*>(gl.glGetString(GL_VERSION)) << '\n';

        std::cerr << "Starting Runtime and loading the scene...\n";
        EngineRuntime runtime;
        Require(runtime.StartUp(), "Runtime must start for GPU scene rendering");
        Scene* scene = runtime.GetSceneManager().GetCurrentScene();
        Require(scene != nullptr, "Runtime must own a scene");
        const auto bounds = scene->GetWorldBounds();
        Require(bounds.valid, "GPU test requires the actual repository model to load");
        Camera* camera = scene->getCurrentCamera();
        Require(camera != nullptr, "Scene must supply a camera");
        camera->FrameSphere(bounds.Center(), bounds.Radius(), Vector3(1, 0.4f, 2).Normalized(), 4.0f / 3.0f);
        const Camera originalCamera = *camera;

        std::cerr << "Initializing shared OpenGL backend...\n";
        auto backend = std::make_unique<OpenGLRenderBackend>(&context);
        CheckCurrent(context, surface);
        std::cerr << "Rendering first GPU frame...\n";
        auto first = backend->Render(*scene, 640, 480);
        CheckCurrent(context, surface);
        std::cerr << "Reading GPU frame for validation...\n";
        const auto image = ReadFrame(gl, first, context);
        Require(image.ShadedPixels() > 1000, "OpenGL must shade actual model pixels, not only clear the target");
        Require(image.ShadedPixels() < 640u * 480u, "Framed scene must retain background pixels");
        if (!capturePath.isEmpty())
        {
            image.Save(capturePath);
        }

        std::cerr << "Checking camera and simulation updates...\n";
        camera->position.x += bounds.Radius() * 0.5f;
        camera->UpdateMatrices();
        const auto movedCamera = ReadFrame(gl, backend->Render(*scene, 640, 480), context);
        Require(movedCamera.Hash() != image.Hash(), "Camera changes must affect actual GPU pixels");
        *camera = originalCamera;
        Require(runtime.GetSimulationSystem().HasBodies(), "The CPU simulation must bind the existing model");
        for (int step = 0; step < 24; ++step)
        {
            runtime.StepSimulation();
        }
        const auto movedModel = ReadFrame(gl, backend->Render(*scene, 640, 480), context);
        Require(movedModel.Hash() != image.Hash(), "CPU simulation must update the model seen by OpenGL");
        runtime.ResetSimulation();
        const auto restored = ReadFrame(gl, backend->Render(*scene, 640, 480), context);
        Require(restored.Hash() == image.Hash(), "Reset restores the exact original GPU image");

        std::cerr << "Checking mesh invalidation and target resize...\n";
        backend->InvalidateMeshCache();
        CheckCurrent(context, surface);
        Require(ReadFrame(gl, backend->Render(*scene, 640, 480), context).Hash() == image.Hash(),
            "Re-uploading unchanged geometry preserves rendering");
        const auto resized = backend->Render(*scene, 377, 213);
        CheckCurrent(context, surface);
        const auto resizedImage = ReadFrame(gl, resized, context);
        Require(resizedImage.width == 377 && resizedImage.height == 213 && resizedImage.ShadedPixels() > 100,
            "Resize must create and render an appropriately sized GPU target");
        Require(!backend->Render(*scene, 0, 213).IsValid() && !backend->Render(*scene, 377, -1).IsValid(),
            "Collapsed dimensions cannot return stale GPU frames");
        CheckCurrent(context, surface);

        std::cerr << "Checking context restoration and resource release...\n";
        context.doneCurrent();
        const auto noCallerContext = backend->Render(*scene, 320, 240);
        Require(noCallerContext.IsValid() && QOpenGLContext::currentContext() == nullptr,
            "Backend rendering also restores the absence of a caller context");
        Require(context.makeCurrent(&surface), "Presentation context must remain usable after rendering");
        ReadFrame(gl, noCallerContext, context);
        const auto borrowedTexture = noCallerContext.textureId;
        backend.reset();
        CheckCurrent(context, surface);
        Require(gl.glIsTexture(borrowedTexture) == GL_FALSE, "Backend destruction releases its borrowed output texture");
        Require(gl.glGetError() == GL_NO_ERROR, "Shutdown must leave the caller context free of GL errors");
        runtime.ShutDown();
        context.doneCurrent();
        std::cout << "Passed: real GPU pixels, camera, CPU simulation, resize, shared texture lifetime and context restoration.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "OpenGL test failed: " << error.what() << '\n';
        return 1;
    }
}
