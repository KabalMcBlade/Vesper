// Copyright (c) 2025-2025 Michele Condo'
// File: C:\Projects\Vesper\Viewer\EntityManager.cpp
// Licensed under the MIT License. See LICENSE file in the project root for full license information.

#include "GameManager.h"

#include "Components/PushConstants.h"
#include <random>

VESPERENGINE_USING_NAMESPACE

GameManager::GameManager(
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
	BlendShapeAnimationSystem& _blendShapeAnimationSystem)
	: m_app(_app)
	, m_entityHandlerSystem(_entityHandlerSystem)
	, m_gameEntitySystem(_gameEntitySystem)
	, m_modelSystem(_modelSystem)
	, m_materialSystem(_materialSystem)
	, m_cameraSystem(_cameraSystem)
	, m_objLoader(_objLoader)
	, m_gltfLoader(_gltfLoader)
	, m_textureSystem(_textureSystem)
	, m_lightSystem(_lightSystem)
	, m_blendShapeAnimationSystem(_blendShapeAnimationSystem)
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

	// Entities Rotation on Self (no DirectionalLightComponent)
	for (auto gameEntity : ecs::IterateEntitiesWithAll<TransformComponent, RotationComponent>(entityManager, componentManager))
	{
		RotationComponent& rotateComponent = componentManager.GetComponent<RotationComponent>(gameEntity);
		TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);

		// Add random rotation, for testing UBO
		const glm::quat& prevRot = transformComponent.Rotation;
		glm::quat currRot = glm::angleAxis(rotateComponent.RadiantPerFrame, rotateComponent.RotationAxis);
		transformComponent.Rotation = prevRot * currRot;
	}

	// PointLightComponent rotation around center
	for (auto gameEntity : ecs::IterateEntitiesWithAll<PointLightComponent, RotationComponent>(entityManager, componentManager))
	{
		PointLightComponent& pointLightComponent = componentManager.GetComponent<PointLightComponent>(gameEntity);
		RotationComponent& rotateComponent = componentManager.GetComponent<RotationComponent>(gameEntity);
		TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);

		glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f); // orbit center (can be made variable later)
		glm::quat rot = glm::angleAxis(rotateComponent.RadiantPerFrame, glm::normalize(rotateComponent.RotationAxis));

		glm::vec3 relativePos = transformComponent.Position - center;
		relativePos = glm::rotate(rot, relativePos);
		transformComponent.Position = relativePos + center;

		// Sync light position with TransformComponent
		pointLightComponent.Position = transformComponent.Position;
	}

	// SpotLightComponent rotation around center
	for (auto gameEntity : ecs::IterateEntitiesWithAll<SpotLightComponent, RotationComponent>(entityManager, componentManager))
	{
		SpotLightComponent& spotLightComponent = componentManager.GetComponent<SpotLightComponent>(gameEntity);
		RotationComponent& rotateComponent = componentManager.GetComponent<RotationComponent>(gameEntity);
		TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);

		glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f); // orbit center
		glm::quat rot = glm::angleAxis(rotateComponent.RadiantPerFrame, glm::normalize(rotateComponent.RotationAxis));

		glm::vec3 relativePos = transformComponent.Position - center;
		relativePos = glm::rotate(rot, relativePos);
		transformComponent.Position = relativePos + center;

		// Rotate direction
		spotLightComponent.Direction = glm::rotate(rot, spotLightComponent.Direction);

		// Sync light position
		spotLightComponent.Position = transformComponent.Position;
	}

	// DirectionalLightComponent
	for (auto gameEntity : ecs::IterateEntitiesWithAll<DirectionalLightComponent, RotationComponent>(entityManager, componentManager))
	{
		DirectionalLightComponent& directionalLightComponent = componentManager.GetComponent<DirectionalLightComponent>(gameEntity);
		RotationComponent& rotateComponent = componentManager.GetComponent<RotationComponent>(gameEntity);
		TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);

		glm::quat rot = glm::angleAxis(rotateComponent.RadiantPerFrame, glm::normalize(rotateComponent.RotationAxis));
		directionalLightComponent.Direction = glm::normalize(glm::rotate(rot, directionalLightComponent.Direction));

		const glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);
		transformComponent.Rotation = glm::rotation(forward, directionalLightComponent.Direction);
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
	//////////////////////////////////////////////////////////////////////////
	// BRDF LUT TEXTURE
	LOG(Logger::INFO, "Generating BRDF LUT texture");
	const std::string brdfLutPath = m_app.GetConfig().TexturesPath + "brdf_lut.png";
	VkExtent2D extent;
	extent.width = 512;
	extent.height = 512;
	m_brdfLut = m_textureSystem.GenerateBRDFLutTexture(brdfLutPath, extent);
	LOG(Logger::INFO, "BRDF LUT texture generated for ", brdfLutPath);

	LOG_NL();

	//////////////////////////////////////////////////////////////////////////
	// CUBEMAP TEXTURE
	/*
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
	*/

	//////////////////////////////////////////////////////////////////////////
	// CUBEMAP HDR TEXTURE TEST
	const std::string cubemapHdrTexturesPath = m_app.GetConfig().TexturesPath + "misty_pines_4k.hdr";
	LOG(Logger::INFO, "Loading HDR Cubemap texture: ", cubemapHdrTexturesPath);

	std::shared_ptr<TextureData> cubeMapHdr = m_textureSystem.LoadCubemap(cubemapHdrTexturesPath);
	LOG(Logger::INFO, "Cubemap HDR loaded!");

	LOG_NL();

	//////////////////////////////////////////////////////////////////////////
	// IRRADIANCE CONVOLUTION MAP TEXTURE
	LOG(Logger::INFO, "Generating Irradiance Convolution texture");
	const std::string irradianceConvolutionPath = m_app.GetConfig().TexturesPath + "irradiance_convolution_map.png";
	m_irradianceMap = m_textureSystem.GenerateIrradianceCubemap(irradianceConvolutionPath, 64, cubeMapHdr);
	LOG(Logger::INFO, "Irradiance Convolution texture generated for ", irradianceConvolutionPath);

	LOG_NL();

	//////////////////////////////////////////////////////////////////////////
	// PRE FILTERED ENVIRONMENT MAP TEXTURE
	LOG(Logger::INFO, "Generating Pre Filtered Environment texture");
	const std::string preFilteredEnvironmentPath = m_app.GetConfig().TexturesPath + "pre_filtered_environment_map.png";
	m_prefilteredEnvMap = m_textureSystem.GeneratePreFilteredEnvironmentMap(preFilteredEnvironmentPath, 512, cubeMapHdr);
	LOG(Logger::INFO, "Pre Filtered Environment texture generated for ", preFilteredEnvironmentPath);

	LOG_NL();


	//////////////////////////////////////////////////////////////////////////
    // Skybox
    {
		std::unique_ptr<ModelData> skyboxData = PrimitiveFactory::GenerateCube(
			m_materialSystem,
			{ 0.0f, 0.0f, 0.0f },
			glm::vec3(1.0f, 1.0f, 1.0f)
		);
     
        ecs::Entity skybox = m_gameEntitySystem.CreateGameEntity(EntityType::Renderable);

        m_modelSystem.LoadSkyboxModel(skybox, std::move(skyboxData), cubeMapHdr);

        TransformComponent& transformComponent = m_app.GetComponentManager().GetComponent<TransformComponent>(skybox);
        transformComponent.Scale = { 50.0f, 50.0f, 50.0f };

        m_entityHandlerSystem.RegisterRenderableEntity(skybox);
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
		transformComponent.Position = { -1.5f, -1.0f, 1.0f };
		transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
		transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

		m_entityHandlerSystem.RegisterRenderableEntity(cubeNoIndices);

		// test for custom render systems: 
		// if PhongCustomTransparentRenderSystem and/or PhongCustomOpaqueRenderSystem are used, they will push constant
		// to change the tint of the entities having this component registered, otherwise nothing.
		if (m_app.GetComponentManager().IsComponentRegistered<ColorTintPushConstantData>())
		{
			m_app.GetComponentManager().AddComponent<ColorTintPushConstantData>(cubeNoIndices);
		}

		m_app.GetComponentManager().AddComponent<RotationComponent>(cubeNoIndices);

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
			transformComponent.Position = { 1.5f, -1.5f, 1.0f };
			transformComponent.Scale = { 0.5f, 0.5f, 0.5f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem.RegisterRenderableEntity(coloredCube);

			// test for custom render systems: 
			// if PhongCustomTransparentRenderSystem and/or PhongCustomOpaqueRenderSystem are used, they will push constant
			// to change the tint of the entities having this component registered, otherwise nothing.
			if (m_app.GetComponentManager().IsComponentRegistered<ColorTintPushConstantData>())
			{
				m_app.GetComponentManager().AddComponent<ColorTintPushConstantData>(coloredCube);
			}

			m_app.GetComponentManager().AddComponent<RotationComponent>(coloredCube);

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
			transformComponent.Position = { -1.5f, 0.5f, 1.0f };
			transformComponent.Scale = { 3.0f, 3.0f, 3.0f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem.RegisterRenderableEntity(flatVase);

			// test for custom render systems: 
			// if PhongCustomTransparentRenderSystem and/or PhongCustomOpaqueRenderSystem are used, they will push constant
			// to change the tint of the entities having this component registered, otherwise nothing.
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
			transformComponent.Position = { 1.5f, 0.5f, 1.0f };
			transformComponent.Scale = { 3.0f, 3.0f, 3.0f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem.RegisterRenderableEntity(smoothVase);

			// test for custom render systems: 
			// if PhongCustomTransparentRenderSystem and/or PhongCustomOpaqueRenderSystem are used, they will push constant
			// to change the tint of the entities having this component registered, otherwise nothing.
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
			transformComponent.Scale = { 5.0f, 1.0f, 5.0f };
			transformComponent.Rotation = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };

			m_entityHandlerSystem.RegisterRenderableEntity(quad);

			// test for custom render systems: 
			// if PhongCustomTransparentRenderSystem and/or PhongCustomOpaqueRenderSystem are used, they will push constant
			// to change the tint of the entities having this component registered, otherwise nothing.
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
			transformComponent.Position = { 0.0f, 0.0f, 3.0f };
			transformComponent.Scale = { 1.0f, 1.0f, 1.0f };
			transformComponent.Rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

			m_entityHandlerSystem.RegisterRenderableEntity(character);

			// test for custom render systems: 
			// if PhongCustomTransparentRenderSystem and/or PhongCustomOpaqueRenderSystem are used, they will push constant
			// to change the tint of the entities having this component registered, otherwise nothing.
			if (m_app.GetComponentManager().IsComponentRegistered<ColorTintPushConstantData>())
			{
				m_app.GetComponentManager().AddComponent<ColorTintPushConstantData>(character);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// GLTF: DamagedHelmet
	{
		const std::string gltfModelName = "DamagedHelmet.gltf";
		std::vector<std::unique_ptr<ModelData>> damagedHelmetDataList = m_gltfLoader.LoadModel(gltfModelName);
		for (auto& damagedHelmetData : damagedHelmetDataList)
		{
			ecs::Entity damagedHelmet = m_gameEntitySystem.CreateGameEntity(EntityType::Renderable);

			m_modelSystem.LoadModel(damagedHelmet, std::move(damagedHelmetData));

			TransformComponent& transformComponent = m_app.GetComponentManager().GetComponent<TransformComponent>(damagedHelmet);
			transformComponent.Position = { 0.0f, 0.0f, 1.0f };
			transformComponent.Scale = { 1.0f, 1.0f, 1.0f };
			transformComponent.Rotation = glm::angleAxis(glm::radians(-180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

			m_entityHandlerSystem.RegisterRenderableEntity(damagedHelmet);

			// test for custom render systems: 
			// if PhongCustomTransparentRenderSystem and/or PhongCustomOpaqueRenderSystem are used, they will push constant
			// to change the tint of the entities having this component registered, otherwise nothing.
			if (m_app.GetComponentManager().IsComponentRegistered<ColorTintPushConstantData>())
			{
				m_app.GetComponentManager().AddComponent<ColorTintPushConstantData>(damagedHelmet);
			}

			int32 animationCount = m_blendShapeAnimationSystem.GetAnimationCount(damagedHelmet);

			LOG(Logger::INFO, "Animation count for ", gltfModelName, " is ", animationCount);

			LOG_NL();

			//m_blendShapeAnimationSystem.SetAnimation(damagedHelmet, 0);

			/*
			m_app.GetComponentManager().AddComponent<RotationComponent>(damagedHelmet);

			static const float radPerFrame = 0.00174533f;     // 0.1 deg
			RotationComponent& rotateComponent = m_app.GetComponentManager().GetComponent<RotationComponent>(damagedHelmet);
			rotateComponent.RotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
			rotateComponent.RadiantPerFrame = radPerFrame;
			*/
		}
	}
}

void GameManager::LoadLights()
{
	glm::vec3 sceneLightDir = { 0.7071f, -0.7071f, 0.0f };
	ecs::Entity sceneLight = m_lightSystem.CreateDirectionalLight(sceneLightDir, { 1.0f, 1.0f, 1.0f }, 1.0f);

	const float radPerFrame = 0.00174533f; // 0.1 deg

	m_app.GetComponentManager().AddComponent<RotationComponent>(sceneLight);
	RotationComponent& rotComp = m_app.GetComponentManager().GetComponent<RotationComponent>(sceneLight);
	rotComp.RotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
	rotComp.RadiantPerFrame = radPerFrame;

#ifdef _DEBUG
	m_app.GetComponentManager().AddComponent<UpdateComponent>(sceneLight);
	m_app.GetComponentManager().AddComponent<DynamicOffsetComponent>(sceneLight);
	//m_app.GetComponentManager().AddComponent<VisibilityComponent>(sceneLight);	// to add/remove dynamically

	m_app.GetComponentManager().AddComponent<TransformComponent>(sceneLight);
	TransformComponent& transformComp = m_app.GetComponentManager().GetComponent<TransformComponent>(sceneLight);
	transformComp.Position = { 0.0f, -5.0f, 0.0f };
	transformComp.Scale = { 0.25f, 0.25f, 0.25f };

	glm::quat rotation = glm::rotation(glm::vec3(0, 0, -1), sceneLightDir);
	transformComp.Rotation = rotation;

	std::unique_ptr<ModelData> dirModel = PrimitiveFactory::GenerateCone(
		m_materialSystem,
		{ 0.0f, 0.0f, 0.0f },
		glm::vec3(1.0f, 1.0f, 1.0f));
	m_modelSystem.LoadModel(sceneLight, std::move(dirModel));

	m_entityHandlerSystem.RegisterRenderableEntity(sceneLight);
#endif

	std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<int32> countDist(2, 5);
	std::uniform_real_distribution<float> posDistXZ(-5.0f, 5.0f);
	std::uniform_real_distribution<float> posDistY(-3.0f, -0.5f);
	std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);
	std::uniform_real_distribution<float> intensityDist(0.5f, 3.0f);
	std::uniform_real_distribution<float> attenuationDist(0.0f, 1.0f);
	std::uniform_real_distribution<float> cutoffDist(0.7f, 0.95f);

	int32 pointCount = countDist(rng);
	for (int32 i = 0; i < pointCount; ++i)
	{
		glm::vec3 position{ posDistXZ(rng), posDistY(rng), posDistXZ(rng) };
		glm::vec3 color{ colorDist(rng), colorDist(rng), colorDist(rng) };
		float intensity = intensityDist(rng);
		glm::vec3 attenuation{ 1.0f, attenuationDist(rng), attenuationDist(rng) };

		ecs::Entity pointLight = m_lightSystem.CreatePointLight(position, color, intensity, attenuation);

		m_app.GetComponentManager().AddComponent<RotationComponent>(pointLight);
		RotationComponent& rotComp = m_app.GetComponentManager().GetComponent<RotationComponent>(pointLight);
		rotComp.RotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		rotComp.RadiantPerFrame = radPerFrame;

#ifdef _DEBUG
		m_app.GetComponentManager().AddComponent<UpdateComponent>(pointLight);
		m_app.GetComponentManager().AddComponent<DynamicOffsetComponent>(pointLight);
		//m_app.GetComponentManager().AddComponent<VisibilityComponent>(pointLight);	// to add/remove dynamically

		m_app.GetComponentManager().AddComponent<TransformComponent>(pointLight);
		TransformComponent& transformComp = m_app.GetComponentManager().GetComponent<TransformComponent>(pointLight);
		transformComp.Position = position;
		transformComp.Scale = { 0.2f, 0.2f, 0.2f };

		std::unique_ptr<ModelData> sphereModel = PrimitiveFactory::GenerateSphere(
			m_materialSystem,
			{ 0.0f, 0.0f, 0.0f },
			color);
		m_modelSystem.LoadModel(pointLight, std::move(sphereModel));

		m_entityHandlerSystem.RegisterRenderableEntity(pointLight);
#endif
	}

	int32 spotCount = countDist(rng);
	for (int32 i = 0; i < spotCount; ++i)
	{
		glm::vec3 position{ posDistXZ(rng), posDistY(rng), posDistXZ(rng) };
		glm::vec3 dir = glm::normalize(-position);
		glm::vec3 color{ colorDist(rng), colorDist(rng), colorDist(rng) };
		float intensity = intensityDist(rng);
		float inner = cutoffDist(rng);
		float outer = glm::min(0.99f, inner + 0.05f);

		ecs::Entity spotLight = m_lightSystem.CreateSpotLight(position, dir, color, intensity, inner, outer);

		m_app.GetComponentManager().AddComponent<RotationComponent>(spotLight);
		RotationComponent& rotComp = m_app.GetComponentManager().GetComponent<RotationComponent>(spotLight);
		rotComp.RotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		rotComp.RadiantPerFrame = radPerFrame;

#ifdef _DEBUG
		m_app.GetComponentManager().AddComponent<UpdateComponent>(spotLight);
		m_app.GetComponentManager().AddComponent<DynamicOffsetComponent>(spotLight);
		//m_app.GetComponentManager().AddComponent<VisibilityComponent>(spotLight);	// to add/remove dynamically

		m_app.GetComponentManager().AddComponent<TransformComponent>(spotLight);
		TransformComponent& transformComp = m_app.GetComponentManager().GetComponent<TransformComponent>(spotLight);
		transformComp.Position = position;
		transformComp.Scale = { 0.2f, 0.2f, 0.2f };

		glm::quat rotation = glm::rotation(glm::vec3(0, 0, -1), dir);
		transformComp.Rotation = rotation;

		std::unique_ptr<ModelData> parallelepipedModel = PrimitiveFactory::GenerateParallelepiped(
			m_materialSystem,
			{ 0.0f, 0.0f, 0.0f },
			1.0f, // bottomSize
			0.4f, // topSize
			1.0f, // height
			color);
		m_modelSystem.LoadModel(spotLight, std::move(parallelepipedModel));

		m_entityHandlerSystem.RegisterRenderableEntity(spotLight);
#endif
	}
}

void GameManager::UnloadGameEntities()
{
	m_entityHandlerSystem.Cleanup();
	m_modelSystem.UnloadModels();
	m_gameEntitySystem.DestroyGameEntities();
}