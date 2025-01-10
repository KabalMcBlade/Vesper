// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\EntityManager.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vesper.h"

#include "GameData.h"

VESPERENGINE_USING_NAMESPACE

class GameManager final
{
public:
	GameManager(
		VesperApp& _app,
		EntityHandlerSystem& _entityHandlerSystem, 
		GameEntitySystem& _gameEntitySystem,
		ModelSystem& _modelSystem,
		CameraSystem& _cameraSystem,
		ObjLoader& _objLoader);
	~GameManager();

public:
	void Update(const FrameInfo& _frameInfo);

	void LoadCameraEntities();
	void LoadGameEntities();
	void UnloadGameEntities();

private:
	VesperApp& m_app;
	EntityHandlerSystem& m_entityHandlerSystem;
	GameEntitySystem& m_gameEntitySystem;
	ModelSystem& m_modelSystem;
	CameraSystem& m_cameraSystem;
	ObjLoader& m_objLoader;
};