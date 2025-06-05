namespace vesper {

template <typename PipelineTagComponent>
void PhongRenderSystem::MaterialBinding()
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    for (auto gameEntity : ecs::IterateEntitiesWithAll<PhongMaterialComponent>(entityManager, componentManager))
    {
        PhongMaterialComponent& materialComponent = componentManager.GetComponent<PhongMaterialComponent>(gameEntity);
        materialComponent.BoundDescriptorSet.resize(SwapChain::kMaxFramesInFlight);

        if (m_device.IsBindlessResourcesSupported())
        {
            for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
            {
                m_bindlessBindingMaterialIndexUbos.emplace_back(m_buffer->Create<BufferComponent>(
                    sizeof(PhongBindlessMaterialIndexUBO),
                    1,
                    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
                    VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
                    1,
                    true));

                BufferComponent& currentBuffer = m_bindlessBindingMaterialIndexUbos.back();

                PhongBindlessMaterialIndexUBO phongIndexUBO;
                phongIndexUBO.MaterialIndex = materialComponent.Index;

                currentBuffer.MappedMemory = &phongIndexUBO;
                m_buffer->WriteToBuffer(currentBuffer);

                VkDescriptorBufferInfo bufferInfo{};
                bufferInfo.buffer = currentBuffer.Buffer;
                bufferInfo.offset = 0;
                bufferInfo.range = currentBuffer.AlignedSize;

                DescriptorWriter(*m_materialSetLayout, *m_renderer.GetDescriptorPool())
                    .WriteBuffer(kPhongUniformBufferOnlyBindingIndex, &bufferInfo)
                    .Build(materialComponent.BoundDescriptorSet[i]);
            }
        }
        else
        {
            for (int32 i = 0; i < SwapChain::kMaxFramesInFlight; ++i)
            {
                DescriptorWriter(*m_materialSetLayout, *m_renderer.GetDescriptorPool())
                    .WriteImage(kPhongAmbientTextureBindingIndex, &materialComponent.AmbientImageInfo)
                    .WriteImage(kPhongDiffuseTextureBindingIndex, &materialComponent.DiffuseImageInfo)
                    .WriteImage(kPhongSpecularTextureBindingIndex, &materialComponent.SpecularImageInfo)
                    .WriteImage(kPhongNormalTextureBindingIndex, &materialComponent.NormalImageInfo)
                    .WriteBuffer(kPhongUniformBufferBindingIndex, &materialComponent.UniformBufferInfo)
                    .Build(materialComponent.BoundDescriptorSet[i]);
            }
        }
    }

    for (auto gameEntity : ecs::IterateEntitiesWithAll<PipelineTagComponent>(entityManager, componentManager))
    {
        componentManager.AddComponent<PhongRenderSystem::ColorTintPushConstantData>(gameEntity);
    }
}

template <typename PipelineTagComponent>
void PhongRenderSystem::Update(const FrameInfo& _frameInfo)
{
    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    for (auto gameEntity : ecs::IterateEntitiesWithAll<RenderComponent, TransformComponent, PipelineTagComponent>(entityManager, componentManager))
    {
        TransformComponent& transformComponent = componentManager.GetComponent<TransformComponent>(gameEntity);
        RenderComponent& renderComponent = componentManager.GetComponent<RenderComponent>(gameEntity);

        renderComponent.ModelMatrix = glm::translate(glm::mat4{1.0f}, transformComponent.Position);
        renderComponent.ModelMatrix = renderComponent.ModelMatrix * glm::toMat4(transformComponent.Rotation);
        renderComponent.ModelMatrix = glm::scale(renderComponent.ModelMatrix, transformComponent.Scale);

        PhongRenderSystem::ColorTintPushConstantData& pushComponent = componentManager.GetComponent<PhongRenderSystem::ColorTintPushConstantData>(gameEntity);

        static const float speed = 1.0f;
        static float frameTimeUpdated = 0.0f;
        const float r = 0.5f * (std::sin(speed * frameTimeUpdated) + 1.0f);
        const float g = 0.5f * (std::sin(speed * frameTimeUpdated + glm::pi<float>() / 3.0f) + 1.0f);
        const float b = 0.5f * (std::sin(speed * frameTimeUpdated + 2.0f * glm::pi<float>() / 3.0f) + 1.0f);
        frameTimeUpdated += _frameInfo.FrameTime;
        pushComponent.ColorTint = glm::vec3(r, g, b);
    }
}

template <typename PipelineTagComponent>
void PhongRenderSystem::Render(const FrameInfo& _frameInfo)
{
    m_pipeline->Bind(_frameInfo.CommandBuffer);

    ecs::EntityManager& entityManager = m_app.GetEntityManager();
    ecs::ComponentManager& componentManager = m_app.GetComponentManager();

    auto entitiesGroupedAndCollected = ecs::EntityCollector::CollectAndGroupEntitiesWithAllByField<
        PhongMaterialComponent,
        PipelineTagComponent,
        DynamicOffsetComponent,
        VertexBufferComponent,
        IndexBufferComponent,
        PhongRenderSystem::ColorTintPushConstantData>(entityManager, componentManager, &PhongMaterialComponent::Index);

    for (const auto& [key, entities] : entitiesGroupedAndCollected)
    {
        const PhongMaterialComponent& phongMaterialComponent = componentManager.GetComponent<PhongMaterialComponent>(entities[0]);

        vkCmdBindDescriptorSets(
            _frameInfo.CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            m_materialSetIndex,
            1,
            &phongMaterialComponent.BoundDescriptorSet[_frameInfo.FrameIndex],
            0,
            nullptr);

        for (const auto& entityCollected : entities)
        {
            const DynamicOffsetComponent& dynamicOffsetComponent = componentManager.GetComponent<DynamicOffsetComponent>(entityCollected);
            const VertexBufferComponent& vertexBufferComponent = componentManager.GetComponent<VertexBufferComponent>(entityCollected);
            const IndexBufferComponent& indexBufferComponent = componentManager.GetComponent<IndexBufferComponent>(entityCollected);
            const PhongRenderSystem::ColorTintPushConstantData& pushComponent = componentManager.GetComponent<PhongRenderSystem::ColorTintPushConstantData>(entityCollected);

            vkCmdBindDescriptorSets(
                _frameInfo.CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineLayout,
                m_entitySetIndex,
                1,
                &_frameInfo.EntityDescriptorSet,
                1,
                &dynamicOffsetComponent.DynamicOffset);

            PushConstants(_frameInfo.CommandBuffer, 0, &pushComponent);
            Bind(vertexBufferComponent, indexBufferComponent, _frameInfo.CommandBuffer);
            Draw(indexBufferComponent, _frameInfo.CommandBuffer);
        }
    }

    entitiesGroupedAndCollected.clear();

    entitiesGroupedAndCollected = ecs::EntityCollector::CollectAndGroupEntitiesWithAllByField<
        PhongMaterialComponent,
        DynamicOffsetComponent,
        VertexBufferComponent,
        NotIndexBufferComponent,
        PhongRenderSystem::ColorTintPushConstantData>(entityManager, componentManager, &PhongMaterialComponent::Index);

    for (const auto& [key, entities] : entitiesGroupedAndCollected)
    {
        const PhongMaterialComponent& phongMaterialComponent = componentManager.GetComponent<PhongMaterialComponent>(entities[0]);

        vkCmdBindDescriptorSets(
            _frameInfo.CommandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_pipelineLayout,
            m_materialSetIndex,
            1,
            &phongMaterialComponent.BoundDescriptorSet[_frameInfo.FrameIndex],
            0,
            nullptr);

        for (const auto& entityCollected : entities)
        {
            const DynamicOffsetComponent& dynamicOffsetComponent = componentManager.GetComponent<DynamicOffsetComponent>(entityCollected);
            const VertexBufferComponent& vertexBufferComponent = componentManager.GetComponent<VertexBufferComponent>(entityCollected);
            const PhongRenderSystem::ColorTintPushConstantData& pushComponent = componentManager.GetComponent<PhongRenderSystem::ColorTintPushConstantData>(entityCollected);

            vkCmdBindDescriptorSets(
                _frameInfo.CommandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_pipelineLayout,
                m_entitySetIndex,
                1,
                &_frameInfo.EntityDescriptorSet,
                1,
                &dynamicOffsetComponent.DynamicOffset);

            PushConstants(_frameInfo.CommandBuffer, 0, &pushComponent);
            Bind(vertexBufferComponent, _frameInfo.CommandBuffer);
            Draw(vertexBufferComponent, _frameInfo.CommandBuffer);
        }
    }

    entitiesGroupedAndCollected.clear();
}

} // namespace vesper
