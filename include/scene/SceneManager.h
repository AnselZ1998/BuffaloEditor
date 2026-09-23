#pragma once

#include "scene/Scene.h"
#include <string>

class SceneManager
{
public:
	SceneManager() = default;
	~SceneManager() = default;

	bool StartUp();
	void ShutDown();
	Scene* GetCurrentScene();
	//Todo: we can change Current Scene~
	// void ChangeCurrentScene(std::string sceneID);

private:
	bool LoadScene(std::string sceneID);
	// Todo
	bool ShutDowmScene(std::string sceneID);

	std::string currentSceneID;
	bool mIsStarted = false;
	// SceneManager owns Scene lifetime; callers must not track allocation or delete it.
	Scene* mScene = nullptr;
};

