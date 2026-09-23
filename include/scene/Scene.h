#pragma once

#include <string>
#include <queue>
#include <vector>
#include <memory>

#include "assets/model/Model.h"
#include "scene/Camera.h"
#include "math/BoundingVolume.h"


class Scene
{
public:
    Scene(std::string& currentSceneID);

    ~Scene();


    Camera* getCurrentCamera();


    // 获取当前可见 Model。
    //
    // 当前版本每次调用都会重新建立 visibleModels queue。
    std::queue<Model*>* GetVisibleModels();

    Bounds3 GetWorldBounds() const;


    bool CheckIfEmpty();


private:
    Camera mainCamera;


    bool mNotEmptyScene = false;


    std::queue<Model*> visibleModels;


    // Scene owns Models.
    std::vector<std::unique_ptr<Model>> mModelsInScene;
};