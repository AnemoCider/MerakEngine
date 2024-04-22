#pragma once

#include "common/VulkanCommon.h"
#include "resource/VulkanResource.h"
#include <string>

#include "tinyglTF/tiny_gltf.h"
#include "tinyglTF/stb_image.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace vki {

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoord;
	};

	struct Model {
		std::vector<std::vector<Vertex>> vertexData;
		std::vector<std::vector<uint16_t>> indexData;
		std::vector<vki::Buffer> vertexBuffers;
		std::vector<vki::Buffer> indexBuffers;
	};

	constexpr const char* modelPath = "./resources/models";
	constexpr const char* shaderPath = "./resources/shaders";

	/*
		read a file to string, in binary format
	*/
	std::string readFile(const std::string& filepath);
	tinygltf::Model readGLTF(tinygltf::TinyGLTF& loader, const std::string& filepath);
	vki::Model loadGLTFModel(tinygltf::TinyGLTF& loader, const std::string& filepath);
};
