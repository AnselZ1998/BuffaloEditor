#include "scene/Scene.h"

#ifndef BUFFALO_DEFAULT_MODEL_PATH
#define BUFFALO_DEFAULT_MODEL_PATH "model/toy.obj"
#endif

Scene::Scene(std::string& currentSceneID)
{
    auto model = std::make_unique<Model>(
        BUFFALO_DEFAULT_MODEL_PATH,
        Matrix4::Identity()
    );


    if (!model->MeshValid())
    {
        return;
    }


    mModelsInScene.push_back(
        std::move(model)
    );


    mNotEmptyScene = true;
}


Scene::~Scene()
{
}


Camera* Scene::getCurrentCamera()
{
    return &mainCamera;
}


std::queue<Model*>* Scene::GetVisibleModels()
{
    while (!visibleModels.empty())
    {
        visibleModels.pop();
    }

    for (const std::unique_ptr<Model>& model : mModelsInScene)
    {
        if (model != nullptr)
        {
            visibleModels.push(
                model.get()
            );
        }
    }


    return &visibleModels;
}


Bounds3 Scene::GetWorldBounds() const
{
    Bounds3 bounds;

    for (const std::unique_ptr<Model>& model : mModelsInScene)
    {
        if (!model)
        {
            continue;
        }


        const Mesh* mesh = model->GetMesh();

        if (mesh == nullptr || !mesh->IsValid())
        {
            continue;
        }


        const Matrix4& modelMatrix =
            model->GetModelMatrix();

        for (const Vector3& vertex : mesh->vertices)
        {

            Vector3 worldPosition =
                (
                    modelMatrix *
                    Vector4::FromPoint(vertex)
                ).XYZ();


            bounds.Expand(
                worldPosition
            );
        }
    }


    return bounds;
}


bool Scene::CheckIfEmpty()
{
    return mNotEmptyScene;
}
