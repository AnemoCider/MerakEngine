#pragma once

#include "runtime/function/render/backend/VulkanBase.h"
#include "runtime/function/render/backend/resource/VulkanResource.h"
#include "runtime/function/render/backend/asset/VulkanAsset.h"

#include <unordered_map>
#include <string_view>
#include <array>

constexpr uint32_t OFFSCREEN_DIM = 2048;
constexpr vk::Format DEPTH_FORMAT = vk::Format::eD16Unorm;

struct DrawCallData {
	// /cave/light.vert corresponds to ./resources/shaders/cave/light.vert
	std::string vertShaderPath;
	std::string fragShaderPath;
	// /cave/scene.gltf refers to ./resources/models/cave/scene.gltf
	std::string modelPath;
	// textures use the same root dir as models
	std::string texturePath = "/default/grey.png";
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	DrawCallData(std::string_view model);
	DrawCallData(std::string_view model, const glm::mat4& modelMat);
	DrawCallData(std::string_view model, std::string_view texture, const glm::mat4& modelMat);
};

class RenderPipeline : public RHIBase{
private:

	tinygltf::TinyGLTF loader;

	struct alignas(16) UniformBufferObject {
		// glm::mat4 model; move to push constants
		glm::mat4 view;
		glm::mat4 proj;
		// glm::mat4 normalRot;
		glm::vec4 lightPos;
		glm::vec4 viewPos;
		glm::mat4 lightPV;
		glm::vec4 lightFov;
	};

	// move to push constants
	/*struct alignas(16) LightUbo {
		glm::mat4 lightMVP;
	};*/

	std::vector<DrawCallData> drawCalls;

	// std::vector<vki::Model> scene;
	std::unordered_map<std::string, vki::Model> models;
	vki::Camera light;

	std::vector<vki::UniformBuffer> uniformBuffers;
	// std::vector<vki::UniformBuffer> lightUbos;
	
	struct GBuffer {
		vki::Texture2D position;
		vki::Texture2D normal;
		vki::Texture2D albedo;
		vki::Texture2D depth;
	} gBuffer;

	std::unordered_map<std::string, vki::Texture2D> textures;

	vk::RenderPass offscreenPass;
	vk::Framebuffer offscreenFramebuffer;

	vk::DescriptorSetLayout lightDescLayout;
	std::unordered_map<std::string, std::vector<vk::DescriptorSet>> lightDescSets;
	vk::DescriptorSetLayout offscreenDescLayout;
	std::unordered_map<std::string, std::vector<vk::DescriptorSet>> offscreenDescSets;
	/*vk::DescriptorSetLayout shadowDescLayout;
	std::vector<vk::DescriptorSet> shadowDescSets;*/

	vki::Shader offscreenVertShader;
	vki::Shader offscreenFragShader;
	vki::Shader lightVertShader;
	vki::Shader lightFragShader;
	vki::Shader pcssFragShader;

	vk::PipelineLayout lightPipelineLayout;
	vk::Pipeline lightPipeline;
	vk::Pipeline pcssLightPipeline;
	vk::PipelineLayout offscreenPipelineLayout;
	vk::Pipeline offscreenPipeline;

	void createOffscreenPass();
	void createOffscreenFramebuffer();
	
	void createModelBuffers(vki::Model& model);
	
	void createDescriptorPool() override;
	void createDescriptorSetLayout();
	void createDescSetFromTexture(std::string_view texture);
	void createUniformBuffers();

	void createShaders();
	void createPipelines();
	void buildCommandBuffer();

	void updateUniformBuffer(uint32_t frame);

	vki::Texture2D createTextureImage(const std::string& file);

	void prepareFrame() override;

	void prepareDrawCallResources();

	void prepareGBuffers();
	
public:

	vki::Camera* activeCamera = &camera;
	bool pcssEnabled = false;

	void init() override;
	void prepare() override;
	void clear() override;
	void render() override;

	template<class... Types>
	void addDrawCall(Types&&... Args) {
		drawCalls.emplace_back(std::forward<Types>(Args)...);
	}
	void switchCamera();
	void switchShadowQuality();
};