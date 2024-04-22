
#include "resource/VulkanResource.h"

vki::VulkanResource::~VulkanResource() {}

vki::VulkanResource::VulkanResource(vki::Device device)
    : device(device) {}

vki::Texture2D::Texture2D(vki::Device& device, uint32_t width, uint32_t height, 
	vk::Format format, vk::ImageUsageFlags usage) 
	: VulkanResource(device), width(width), height(height), format(format), usage(usage) {
    createImage();
    createView();
    createSampler();
}

vki::Texture2D::Texture2D(Texture2D &&other)
    : VulkanResource(std::exchange(other.device, {})){
    width = other.width;
    height = other.height;
    mipLevels = other.mipLevels;
    format = other.format;
    usage = other.usage;
    std::swap(image, other.image);
    std::swap(view, other.view);
    std::swap(sampler, other.sampler);
    std::swap(mem, other.mem);
}

vki::Texture2D& vki::Texture2D::operator=(Texture2D&& other) {
    std::swap(device, other.device);
    width = other.width;
    height = other.height;
    mipLevels = other.mipLevels;
    format = other.format;
    usage = other.usage;
    std::swap(image, other.image);
    std::swap(view, other.view);
    std::swap(sampler, other.sampler);
    std::swap(mem, other.mem);
    return *this;
}

void vki::Texture2D::createImage() {
    vk::ImageCreateInfo imageCI{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = {width, height, 1},
        .mipLevels = mipLevels,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = usage
    };
    image = device.getDevice().createImage(imageCI, nullptr);
    vk::MemoryRequirements memReq = device.getDevice().getImageMemoryRequirements(image);
    vk::MemoryAllocateInfo memAI{
        .allocationSize = memReq.size,
        .memoryTypeIndex = device.getMemoryType(memReq.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    };
    mem = device.getDevice().allocateMemory(memAI);
    device.getDevice().bindImageMemory(image, mem, 0);
}

void vki::Texture2D::createView() {
    assert(image && "Cannot create imageView before creating the image");
    vk::ImageSubresourceRange range{
        // .aspectMask = vk::ImageAspectFlagBits::eDepth,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
    };
    if (usage & vk::ImageUsageFlagBits::eColorAttachment) {
        range.aspectMask = vk::ImageAspectFlagBits::eColor;
    } else if (usage & vk::ImageUsageFlagBits::eDepthStencilAttachment) {
        range.aspectMask = vk::ImageAspectFlagBits::eDepth;
    } else {
        std::cerr << "Invalid image usage\n";
        exit(0);
    }

    vk::ImageViewCreateInfo imageViewCI{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        .subresourceRange = range
    };

    view = device.getDevice().createImageView(imageViewCI);
}

void vki::Texture2D::createSampler() {
    assert(image && "Cannot create the sampler before the image");
    vk::SamplerCreateInfo samplerCI{
        .magFilter = vk::Filter::eNearest,
        .minFilter = vk::Filter::eNearest,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eClampToEdge,
        .addressModeV = vk::SamplerAddressMode::eClampToBorder,
        .addressModeW = vk::SamplerAddressMode::eClampToBorder,
        .mipLodBias = 0.0f,
        .anisotropyEnable = vk::False,
        .maxAnisotropy = 1.0f,
        .minLod = 0.0f,
        // Set max level-of-detail to mip level count of the texture
        .maxLod = 1.0f,
        .borderColor = vk::BorderColor::eFloatOpaqueWhite
    };

    sampler = device.getDevice().createSampler(samplerCI);
}

vki::Texture2D::~Texture2D() {
    if (!device.getDevice()) return;
    if (sampler != nullptr) {
        device.getDevice().destroySampler(sampler);
    }
    if (view != nullptr) {
        device.getDevice().destroyImageView(view);
    }
    if (mem != nullptr) {
        device.getDevice().freeMemory(mem);
    }
    if (image != nullptr) {
        device.getDevice().destroyImage(image);
    }
}

vki::Buffer::Buffer(vki::Device& device, vk::DeviceSize sz,
    const vk::BufferUsageFlags usage, 
    const vk::MemoryPropertyFlags memoryProps) 
    : VulkanResource(device) {
    vk::BufferCreateInfo bufferCI{
        .size = sz,
        .usage = usage
    };

    buffer = device.getDevice().createBuffer(bufferCI);
    auto memReq = device.getDevice().getBufferMemoryRequirements(buffer);
    vk::MemoryAllocateInfo memAI{
        .allocationSize = memReq.size,
        .memoryTypeIndex = device.getMemoryType(memReq.memoryTypeBits, memoryProps)
    };
    mem = device.getDevice().allocateMemory(memAI);
    device.getDevice().bindBufferMemory(buffer, mem, 0);
}

vki::Buffer::Buffer(Buffer&& other) noexcept
    : VulkanResource(std::exchange(other.device, {})),
    buffer(std::exchange(other.buffer, VK_NULL_HANDLE)),
    mem(std::exchange(other.mem, VK_NULL_HANDLE)) {}

vki::Buffer::~Buffer() {
    if (mem) {
        device.getDevice().freeMemory(mem);
        mem = VK_NULL_HANDLE;
    }
    if (buffer) {
        device.getDevice().destroyBuffer(buffer);
        buffer = VK_NULL_HANDLE;
    }
}

vki::UniformBuffer::UniformBuffer(vki::Device& device, vk::DeviceSize sz) 
    : Buffer(device, sz, vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent){
    mapped = device.getDevice().mapMemory(mem, 0, sz);
}

vki::UniformBuffer::UniformBuffer(UniformBuffer&& other) noexcept{
    std::swap(this->buffer, other.buffer);
    std::swap(this->mapped, other.mapped);
    std::swap(this->device, other.device);
    std::swap(this->mem, other.mem);
    other.buffer = VK_NULL_HANDLE;
    other.mapped = nullptr;
    other.device = vki::Device();
    other.mem = VK_NULL_HANDLE;
}

vki::UniformBuffer::~UniformBuffer() {
    if (!device.getDevice()) return;
    if (mem)
        device.getDevice().unmapMemory(mem);
}

vki::StagingBuffer::StagingBuffer(vki::Device& device, vk::DeviceSize sz)
    : Buffer(device, sz, vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) { }

vki::StagingBuffer::~StagingBuffer() {

}

vki::Shader::Shader(vki::Device& device, const std::string& shaderCode) {
    vk::ShaderModuleCreateInfo shaderCI{
            .codeSize = shaderCode.size(),
            .pCode = reinterpret_cast<const uint32_t*>(shaderCode.data())
    };

    shaderModule = device.getDevice().createShaderModule(
        shaderCI
    );
}


vki::Shader::~Shader() {
    if (device.getDevice() && shaderModule != nullptr)
        device.getDevice().destroyShaderModule(shaderModule);
}
