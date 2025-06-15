// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\EntityManager.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameManager.h"

#include "Components/PushConstants.h"

VESPERENGINE_USING_NAMESPACE

GameManager::GameManager(
	VesperApp& _app,
	EntityHandlerSystem& _entityHandlerSystem,
	GameEntitySystem& _gameEntitySystem,
	ModelSystem& _modelSystem,
	MaterialSystem& _materialSystem,
	CameraSystem& _cameraSystem,
	ObjLoader& _objLoader,
	TextureSystem& _textureSystem)
	: m_app(_app)
	, m_entityHandlerSystem(_entityHandlerSystem)
	, m_gameEntitySystem(_gameEntitySystem)
	, m_modelSystem(_modelSystem)
	, m_materialSystem(_materialSystem)
	, m_cameraSystem(_cameraSystem)
	, m_objLoader(_objLoader)
	, m_textureSystem(_textureSystem)
{
	m_app.GetComponentManager().RegisterComponent<RotationComponent>();
}

GameManager::~GameManager()
{
	m_app.GetComponentManager().UnregisterComponent<RotationComponent>();
}

void GameManager::Update(const FrameInfo& _frameInfo)
{
	ecs::EntityManager& entityManager = m_app.GetEntityManager();
	ecs::ComponentManager& componentManager = m_app.GetComponentManager();

	for (auto gameEntity : ecs::IterateEntitiesWithAll<TransformComponent, RotationComponent>(entityManager, componentManager))
	{
		RotationComponent& rotateComponent = componentManager.GetComponent<RotationComponent>(gameEntity);
		TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);

		// Add random rotation, for testing UBO
		const glm::quat& prevRot = transformComponent.Rotation;
		glm::quat currRot = glm::angleAxis(rotateComponent.RadiantPerFrame, rotateComponent.RotationAxis);
		transformComponent.Rotation = prevRot * currRot;
	}
}

void GameManager::LoadCameraEntities()
{
	{
		ecs::Entity camera = m_gameEntitySystem.CreateGameEntity(EntityType::Camera);

		CameraTransformComponent& transformComponent = m_app.GetComponentManager().GetComponent<CameraTransformComponent>(camera);
		transformComponent.Position = { 0.0f, -0.5f, -3.5f };

		CameraComponent& cameraComponent = m_app.GetComponentManager().GetComponent<CameraComponent>(camera);

		m_cameraSystem.SetViewRotation(cameraComponent, transformComponent);
		m_cameraSystem.SetPerspectiveProjection(cameraComponent, glm::radians(50.0f), 1.0f, 0.1f, 100.0f);

		// Active this one by default
		m_cameraSystem.SetCurrentActiveCamera(camera);
	}
}

void GameManager::LoadGameEntities()
{
    // Skybox
    {
		// CUBEMAP TEXTURE TEST
		const std::string cubemapTexturesDirectoryPath = m_app.GetConfig().TexturesPath + "Yokohama3_CubeMap/";
		LOG(Logger::INFO, "Loading Cubemap texture: ", cubemapTexturesDirectoryPath);

		std::array<std::string, 6> cubemapTexturesDirectoryFilepaths;
        cubemapTexturesDirectoryFilepaths[0] = cubemapTexturesDirectoryPath + "posx.jpg";
        cubemapTexturesDirectoryFilepaths[1] = cubemapTexturesDirectoryPath + "negx.jpg";
        cubemapTexturesDirectoryFilepaths[2] = cubemapTexturesDirectoryPath + "posy.jpg";
        cubemapTexturesDirectoryFilepaths[3] = cubemapTexturesDirectoryPath + "negy.jpg";
        cubemapTexturesDirectoryFilepaths[4] = cubemapTexturesDirectoryPath + "posz.jpg";
        cubemapTexturesDirectoryFilepaths[5] = cubemapTexturesDirectoryPath + "negz.jpg";
		
		std::shared_ptr<TextureData> cubeMap = m_textureSystem.LoadCubemap(cubemapTexturesDirectoryFilepaths);
		LOG(Logger::INFO, "Cubemap loaded!");

		// CUBEMAP HDR TEXTURE TEST
		const std::string cubemapHdrTexturesPath = m_app.GetConfig().TexturesPath + "misty_pines_4k.hdr";
		LOG(Logger::INFO, "Loading Cubemap texture: ", cubemapHdrTexturesPath);

		std::shared_ptr<TextureData> cubeMapHdr = m_textureSystem.LoadCubemap(cubemapHdrTexturesPath);
		LOG(Logger::INFO, "Cubemap HDR loaded!");

		LOG_NL();

        std::vector<std::unique_ptr<ModelData>> skyboxDataList = m_objLoader.LoadModel("cube.obj");
        if (!skyboxDataList.empty())
        {
            ecs::Entity skybox = m_gameEntitySystem.CreateGameEntity(EntityType::Renderable);

            m_modelSystem.LoadSkyboxModel(skybox, std::move(skyboxDataList.front()), cubeMap);

            TransformComponent& transformComponent = m_app.GetComponentManager().GetComponent<TransformComponent>(skybox);
            transformComponent.Scale = { 50.0f, 50.0f, 50.0f };

            m_entityHandlerSystem.RegisterRenderableEntity(skybox);
        }
    }

	//////////////////////////////////////////////////////////////////////////
	// Cube no Indices
	{
		std::unique_ptr<ModelData> cubeNoIndicesData = PrimitiveFactory::GenerateCubeNoIndices(
			m_materialSystem,
			{ 0.0f, 0.0f, 0.0f },
			{ glm::vec3(.9f, .9f, .9f), glm::vec3(.8f, .8f, .1f), glm::vec3(.9f, .6f, .1f), glm::vec3(.8f, .1f, .1f), glm::vec3(.1f, .1f, .8f), glm::vec3(.1f, .8f, .1f) }
		);

		ecs::Entity cubeNoIndices = m_gameEntitySystem.CreateGameEntity(EntityType::Renderable);

		m_modelSystem.LoadModel(cubeNoIndices, std::move(cubeNoIndicesData));

		TransformComponent& transformComponent = m_app.GetComponentManager().GetComponent<TransformComponent>(cubeNoIndices);
		transformComponent.Position = { -1.0f, -1.0f, 0.0f };
		transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

		m_entityHandlerSystem.RegisterRenderableEntity(cubeNoIndices);

		m_app.GetComponentManager().AddComponent<RotationComponent>(cubeNoIndices);

		// test for custom render systems
		if (m_app.GetComponentManager().IsComponentRegistered<ColorTintPushConstantData>())
		{
			m_app.GetComponentManager().AddComponent<ColorTintPushConstantData>(cubeNoIndices);
		}

		static const float radPerFrame = 0.00174533f;     // 0.1 deg
		RotationComponent& rotateComponent = m_app.GetComponentManager().GetComponent<RotationComponent>(cubeNoIndices);
		rotateComponent.RotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		rotateComponent.RadiantPerFrame = radPerFrame;
	}


	//////////////////////////////////////////////////////////////////////////
	// Cube
	{
		std::vector<std::unique_ptr<ModelData>> coloredCubeDataList = m_objLoader.LoadModel("colored_cube.obj");
		for (auto& coloredCubeData : coloredCubeDataList)
		{
			ecs::Entity coloredCube = m_gameEntitySystem.CreateGameEntity(EntityType::Renderable);

			m_modelSystem.LoadModel(coloredCube, std::move(coloredCubeData));

			TransformComponent& transformComponent = m_app.GetComponentManager().GetComponent<TransformComponent>(coloredCube);
			transformComponent.Position = { 1.0f, -1.5f, 0.0f };
			transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem.RegisterRenderableEntity(coloredCube);

			m_app.GetComponentManager().AddComponent<RotationComponent>(coloredCube);

			// test for custom render systems
			if (m_app.GetComponentManager().IsComponentRegistered<ColorTintPushConstantData>())
			{
				m_app.GetComponentManager().AddComponent<ColorTintPushConstantData>(coloredCube);
			}

			static const float radPerFrame = 0.00174533f;     // 0.1 deg
			RotationComponent& rotateComponent = m_app.GetComponentManager().GetComponent<RotationComponent>(coloredCube);
			rotateComponent.RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
			rotateComponent.RadiantPerFrame = radPerFrame;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Flat Vase
	{
		std::vector<std::unique_ptr<ModelData>> flatVaseDataList = m_objLoader.LoadModel("flat_vase.obj");
		for (auto& flatVaseData : flatVaseDataList)
		{
			ecs::Entity flatVase = m_gameEntitySystem.CreateGameEntity(EntityType::Renderable);

			m_modelSystem.LoadModel(flatVase, std::move(flatVaseData));

			TransformComponent& transformComponent = m_app.GetComponentManager().GetComponent<TransformComponent>(flatVase);
			transformComponent.Position = { -1.0f, 0.5f, 0.0f };
			transformComponent.Scale = { 3.0f, 3.0f, 3.0f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem.RegisterRenderableEntity(flatVase);

			// test for custom render systems
			if (m_app.GetComponentManager().IsComponentRegistered<ColorTintPushConstantData>())
			{
				m_app.GetComponentManager().AddComponent<ColorTintPushConstantData>(flatVase);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Smooth Vase
	{
		std::vector<std::unique_ptr<ModelData>> smoothVaseDataList = m_objLoader.LoadModel("smooth_vase.obj");
		for (auto& smoothVaseData : smoothVaseDataList)
		{
			ecs::Entity smoothVase = m_gameEntitySystem.CreateGameEntity(EntityType::Renderable);

			m_modelSystem.LoadModel(smoothVase, std::move(smoothVaseData));

			TransformComponent& transformComponent = m_app.GetComponentManager().GetComponent<TransformComponent>(smoothVase);
			transformComponent.Position = { 1.0f, 0.5f, 0.0f };
			transformComponent.Scale = { 3.0f, 3.0f, 3.0f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem.RegisterRenderableEntity(smoothVase);

			// test for custom render systems
			if (m_app.GetComponentManager().IsComponentRegistered<ColorTintPushConstantData>())
			{
				m_app.GetComponentManager().AddComponent<ColorTintPushConstantData>(smoothVase);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Plane / Quad
	{
		std::vector<std::unique_ptr<ModelData>> quadDataList = m_objLoader.LoadModel("quad.obj");
		for (auto& quadData : quadDataList)
		{
			ecs::Entity quad = m_gameEntitySystem.CreateGameEntity(EntityType::Renderable);

			m_modelSystem.LoadModel(quad, std::move(quadData));

			TransformComponent& transformComponent = m_app.GetComponentManager().GetComponent<TransformComponent>(quad);
			transformComponent.Position = { 0.0f, 1.0f, 0.0f };
			transformComponent.Scale = { 3.0f, 1.0f, 3.0f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem.RegisterRenderableEntity(quad);

			// test for custom render systems
			if (m_app.GetComponentManager().IsComponentRegistered<ColorTintPushConstantData>())
			{
				m_app.GetComponentManager().AddComponent<ColorTintPushConstantData>(quad);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// A_blonde_twintailed_g_1228205950_texture or Laboratory_cabinet_NakedSingularity
	{
		std::vector<std::unique_ptr<ModelData>> characterDataList = m_objLoader.LoadModel("Laboratory_cabinet_NakedSingularity.obj");
		for (auto& characterData : characterDataList)
		{
			ecs::Entity character = m_gameEntitySystem.CreateGameEntity(EntityType::Renderable);

			m_modelSystem.LoadModel(character, std::move(characterData));

			TransformComponent& transformComponent = m_app.GetComponentManager().GetComponent<TransformComponent>(character);
			transformComponent.Position = { 0.0f, 0.0f, 1.0f };
			transformComponent.Scale = { 1.0f, 1.0f, 1.0f };
			transformComponent.Rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

			m_entityHandlerSystem.RegisterRenderableEntity(character);

			// test for custom render systems
			if (m_app.GetComponentManager().IsComponentRegistered<ColorTintPushConstantData>())
			{
				m_app.GetComponentManager().AddComponent<ColorTintPushConstantData>(character);
			}
		}
	}
}

void GameManager::UnloadGameEntities()
{
	m_entityHandlerSystem.Cleanup();
	m_modelSystem.UnloadModels();
	m_gameEntitySystem.DestroyGameEntities();
}