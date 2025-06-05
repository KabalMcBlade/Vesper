#pragma once

#include "Core/core_defines.h"
#include "Core/glm_config.h"


VESPERENGINE_NAMESPACE_BEGIN

class VesperApp;
struct FrameInfo;

struct ColorTintPushConstantData
{
    glm::vec3 ColorTint{1.0f};
};

class VESPERENGINE_API ColorTintSystem final
{
public:
    explicit ColorTintSystem(VesperApp& _app);
    ~ColorTintSystem();

    ColorTintSystem(const ColorTintSystem&) = delete;
    ColorTintSystem& operator=(const ColorTintSystem&) = delete;

    void AddColorComponents();
    void Update(const FrameInfo& _frameInfo);

private:
    VesperApp& m_app;
};

VESPERENGINE_NAMESPACE_END
