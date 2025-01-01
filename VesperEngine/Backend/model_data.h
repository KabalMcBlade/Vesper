#pragma once

#include "Core/core_defines.h"

#include "vulkan/vulkan.h"

#define GLM_FORCE_INTRINSICS
//#define GLM_FORCE_SSE2		// or GLM_FORCE_SSE42 or else, but the above one use compiler to find out which one is enabled
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "vma/vk_mem_alloc.h"

#include <vector>
#include <memory>


VESPERENGINE_NAMESPACE_BEGIN

struct Vertex
{
	glm::vec3 Position{};
	glm::vec3 Color{};
	glm::vec3 Normal{};
	glm::vec2 UV{};

	static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

	bool operator==(const Vertex& _other) const 
	{
		return Position == _other.Position && Color == _other.Color && Normal == _other.Normal && UV == _other.UV;
	}
};

struct TextureData
{
	VmaAllocation AllocationMemory{ VK_NULL_HANDLE };	// Memory allocation handled by VMA
	VkImage Image{ VK_NULL_HANDLE };					// The Vulkan image
	VkImageView ImageView{ VK_NULL_HANDLE };			// The image view for shader access
	VkSampler Sampler{ VK_NULL_HANDLE };				// The sampler for texture filtering
};

enum class MaterialType : uint32
{
	Invalid,
	Phong,
	PBR
};

struct MaterialData
{
	MaterialType Type;
	std::string Name;
	bool bIsBound;

	MaterialData() : Type(MaterialType::Invalid), Name(""), bIsBound(false) {}
	MaterialData(MaterialType _type) : Type(_type), Name(""), bIsBound(false) {}
	MaterialData(MaterialType _type, const std::string& _name) : Type(_type), Name(_name), bIsBound(false) {}

	// only to force polymorphisms
	virtual ~MaterialData() = default;
};

struct ModelData
{
	std::vector<Vertex> Vertices{};
	std::vector<uint32> Indices{};
	std::unique_ptr<MaterialData> Material;
	bool IsStatic{ false };
};


// This is used only to load from disk, has the texture paths
struct MaterialDataPhong : public MaterialData
{
	MaterialDataPhong() : MaterialData(MaterialType::Phong, "Phong") {}
	MaterialDataPhong(const std::string& _name) : MaterialData(MaterialType::Phong, _name) {}

	glm::vec4 AmbientColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 DiffuseColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 SpecularColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 EmissionColor{ 1.0f, 1.0f, 1.0f, 1.0f };

	float Shininess{ 0.0f };

	std::string AmbientTexturePath;	
	std::string DiffuseTexturePath;	
	std::string SpecularTexturePath; 
	std::string NormalTexturePath;
};

// This is used only to load from disk, has the texture paths
struct MaterialDataPBR : public MaterialData
{
	MaterialDataPBR() : MaterialData(MaterialType::PBR, "PBR") {}
	MaterialDataPBR(const std::string& _name) : MaterialData(MaterialType::PBR, _name) {}

	float Roughness{ 0.0f };
	float Metallic{ 0.0f };
	float Sheen{ 0.0f };
	float ClearcoatThickness{ 0.0f };
	float ClearcoatRoughness{ 0.0f };
	float Anisotropy{ 0.0f };
	float AnisotropyRotation{ 0.0f };

	std::string RoughnessTexturePath;
	std::string MetallicTexturePath;
	std::string SheenTexturePath;
	std::string EmissiveTexturePath;
	std::string NormalMapTexturePath;
};

struct MaterialPhong : public MaterialData
{
	MaterialPhong() : MaterialData(MaterialType::Phong) {}

	glm::vec4 AmbientColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 DiffuseColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 SpecularColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	glm::vec4 EmissionColor{ 1.0f, 1.0f, 1.0f, 1.0f };

	float Shininess{ 0.0f };

	TextureData AmbientTexture;
	TextureData DiffuseTexture;
	TextureData SpecularTexture;
	TextureData NormalTexture;
};

struct MaterialPBR : public MaterialData
{
	MaterialPBR() : MaterialData(MaterialType::PBR) {}

	float Roughness{ 0.0f };
	float Metallic{ 0.0f };
	float Sheen{ 0.0f };
	float ClearcoatThickness{ 0.0f };
	float ClearcoatRoughness{ 0.0f };
	float Anisotropy{ 0.0f };
	float AnisotropyRotation{ 0.0f };

	TextureData RoughnessTexture;
	TextureData MetallicTexture;
	TextureData SheenTexture;
	TextureData EmissiveTexture;		// Should be combined already with DiffuseTexture
	TextureData NormalMapTexture;
};


VESPERENGINE_NAMESPACE_END
