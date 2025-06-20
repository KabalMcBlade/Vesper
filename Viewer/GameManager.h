// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\EntityManager.h
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#pragma once

#include "vesper.h"
#include <memory>

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
		MaterialSystem& _materialSystem,
		CameraSystem& _cameraSystem,
		ObjLoader& _objLoader,
		GltfLoader& _gltfLoader,
		TextureSystem& _textureSystem,
		LightSystem& _lightSystem,
		BlendShapeAnimationSystem& _blendShapeAnimationSystem);
	~GameManager();

public:
	void Update(const FrameInfo& _frameInfo);

	void LoadCameraEntities();
	void LoadGameEntities();
	void LoadLights();
	void UnloadGameEntities();

	VESPERENGINE_INLINE std::shared_ptr<TextureData> GetIrradianceMap() const { return m_irradianceMap; }
	VESPERENGINE_INLINE std::shared_ptr<TextureData> GetPrefilteredEnvMap() const { return m_prefilteredEnvMap; }
	VESPERENGINE_INLINE std::shared_ptr<TextureData> GetBrdfLut() const { return m_brdfLut; }

private:
	friend class ViewerApp;
	VesperApp& m_app;
	EntityHandlerSystem& m_entityHandlerSystem;
	GameEntitySystem& m_gameEntitySystem;
	ModelSystem& m_modelSystem;
	MaterialSystem& m_materialSystem;
	CameraSystem& m_cameraSystem;
	ObjLoader& m_objLoader;
	GltfLoader& m_gltfLoader;
	TextureSystem& m_textureSystem;
	LightSystem& m_lightSystem;
	BlendShapeAnimationSystem& m_blendShapeAnimationSystem;

	std::shared_ptr<TextureData> m_brdfLut{};
	std::shared_ptr<TextureData> m_irradianceMap{};
	std::shared_ptr<TextureData> m_prefilteredEnvMap{};
};