#include "scene/SceneManager.h"
#include <iostream>

bool SceneManager::StartUp()
{
    if (mIsStarted)
    {
        return true;
    }

    currentSceneID = "defaultScene";
    // LoadScene reports Model availability; an empty Scene is still valid.
    if (!LoadScene(currentSceneID))
    {
        std::clog << "Scene has no renderable Model; starting with an empty Scene.\n";
    }

    mIsStarted = true;
    return true;
}

void SceneManager::ShutDown()
{
    delete mScene;
    mScene = nullptr;
    mIsStarted = false;
}

Scene* SceneManager::GetCurrentScene()
{
    return mScene;
}

bool SceneManager::LoadScene(std::string sceneID)
{
    // Release any previous Scene here; outer systems need no allocation guard.
    ShutDown();
    mScene = new Scene(sceneID);
    return mScene->CheckIfEmpty();
}
