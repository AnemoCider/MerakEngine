#include "asset/VulkanAsset.h"
#include <fstream>
#include <sstream>
#include <filesystem>

std::string vki::readFile(const std::string& filepath) {
	std::ifstream file(filepath, std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	std::stringstream buffer{};
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

tinygltf::Model vki::readGLTF(tinygltf::TinyGLTF& loader, const std::string& filepath) {
    tinygltf::Model model;
    std::string err;
    std::string warn;
    auto ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath);
    if (!warn.empty()) {
        std::cout << "TinyglTF [warning] when loading model: " <<
            warn.c_str() << '\n';
    }
    if (!err.empty()) {
        std::cout << "TinyglTF [error] when loading model: " <<
            err.c_str() << '\n';
        std::cout << "abs path is: " << std::filesystem::absolute(filepath) << '\n';
    }
    assert(ret);
    return model;
}

vki::Model vki::loadGLTFModel(tinygltf::TinyGLTF& loader, const std::string& filepath) {
    auto model = readGLTF(loader, filepath);
    Model modelData;
    for (uint32_t i = 0; i < model.meshes.size(); i++) {
        for (auto& primitive: model.meshes[i].primitives) {
            
            std::vector<Vertex> vertexBuffer{};
            std::vector<std::remove_reference_t<decltype(modelData.indexData[0][0])>> indexBuffer{};
            const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];

            // Indices
            const tinygltf::BufferView& bufferView = model.bufferViews[indexAccessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
            const uint16_t* indicesBuffer = reinterpret_cast<const uint16_t*>(&buffer.data[bufferView.byteOffset + indexAccessor.byteOffset]);
            indexBuffer.insert(indexBuffer.end(), indicesBuffer, indicesBuffer + indexAccessor.count);

            // Get the accessor indices for position, normal, and texcoords
            int posAccessorIndex = primitive.attributes.find("POSITION")->second;
            int normAccessorIndex = primitive.attributes.find("NORMAL")->second;
            int texCoordAccessorIndex = primitive.attributes.find("TEXCOORD_0")->second;

            // Obtain the buffer views corresponding to the accessors
            const tinygltf::Accessor& posAccessor = model.accessors[posAccessorIndex];
            const tinygltf::Accessor& normAccessor = model.accessors[normAccessorIndex];
            const tinygltf::Accessor& texCoordAccessor = model.accessors[texCoordAccessorIndex];

            // Obtain buffer data for positions, normals, and texcoords
            const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
            const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
            const tinygltf::BufferView& texCoordView = model.bufferViews[texCoordAccessor.bufferView];

            vertexBuffer.reserve(posAccessor.count);

            const float* bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
            const float* bufferNormals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
            const float* bufferTexCoords = reinterpret_cast<const float*>(&(model.buffers[texCoordView.buffer].data[texCoordAccessor.byteOffset + texCoordView.byteOffset]));

            for (size_t v = 0; v < posAccessor.count; v++) {
                Vertex vert{};

                vert.pos = glm::make_vec3(&bufferPos[v * 3]);
                vert.normal = glm::normalize(glm::make_vec3(&bufferNormals[v * 3]));
                vert.texCoord = glm::make_vec2(&bufferTexCoords[v * 2]);
                vertexBuffer.emplace_back(std::move(vert));
            }
            modelData.vertexData.emplace_back(std::move(vertexBuffer));
            modelData.indexData.emplace_back(std::move(indexBuffer));
        }
    }
    return modelData;
}
