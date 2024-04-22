#pragma once

#include "common/VulkanCommon.h"
#include "initialization/VulkanDevice.h"

namespace vki {

    class VulkanResource {
    protected:
        vki::Device device;
    public:
        VulkanResource() = default;
        VulkanResource(vki::Device device);
        virtual ~VulkanResource();
    };

	class Texture2D : public VulkanResource{
    public:
        uint32_t width, height;
        uint32_t mipLevels = 1;
        vk::Image image{ nullptr };
        vk::ImageView view{ nullptr };
        vk::Sampler sampler{ nullptr };
        vk::Format format;
        vk::DeviceMemory mem{ nullptr };
        vk::ImageUsageFlags usage;

        void createImage();
        void createView();
        void createSampler();

    public:
        Texture2D() = default;
        Texture2D(vki::Device& device, uint32_t width, uint32_t height, vk::Format format, vk::ImageUsageFlags usage);
        Texture2D(Texture2D&& other);
        Texture2D& operator=(Texture2D&& other);
        ~Texture2D();
	};

    class Buffer : public VulkanResource {
    public:
        vk::Buffer buffer{ VK_NULL_HANDLE };
        vk::DeviceMemory mem{ VK_NULL_HANDLE };
        Buffer() = default;
        Buffer(vki::Device& device, vk::DeviceSize sz, const vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags memoryProps);
        // Buffer(const Buffer& other);
        Buffer(Buffer&& other) noexcept;
        // vki::Buffer& operator=(const vki::Buffer& other);
        ~Buffer();
    };

    class UniformBuffer : public Buffer {
    public:
        UniformBuffer() = default;
        UniformBuffer(vki::Device& device, vk::DeviceSize sz);
        UniformBuffer(UniformBuffer&& other) noexcept;
        ~UniformBuffer();
        void* mapped{ nullptr };
    };

    class StagingBuffer : public Buffer {
    public:
        StagingBuffer() = default;
        StagingBuffer(vki::Device& device, vk::DeviceSize sz);
        ~StagingBuffer();
    };


    class Shader : public VulkanResource {
    public:
        vk::ShaderModule shaderModule{ nullptr };
        Shader() = default;
        Shader(vki::Device& device, const std::string& shaderCode);
        ~Shader();
    };
};