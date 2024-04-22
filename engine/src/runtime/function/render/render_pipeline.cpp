#include "render_pipeline.h"
#include "runtime/function/render/render_pipeline.h"
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <cstdio>

namespace {

    constexpr uint32_t NUM_GBUFFER_TEXTURES = 4;

    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        auto app = reinterpret_cast<RenderPipeline*>(glfwGetWindowUserPointer(window));
        app->activeCamera->zoomIn(static_cast<float>(yoffset));
    }
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        auto app = reinterpret_cast<RenderPipeline*>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            app->activeCamera->startDrag();
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
            app->activeCamera->disableDrag();
    }
    void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        auto app = reinterpret_cast<RenderPipeline*>(glfwGetWindowUserPointer(window));
        app->activeCamera->mouseDrag(static_cast<float>(xpos), static_cast<float>(ypos));
    }

    void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto app = reinterpret_cast<RenderPipeline*>(glfwGetWindowUserPointer(window));
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_C) {
                app->switchCamera();
            }
            else if (key == GLFW_KEY_X) {
                app->switchShadowQuality();
            }
        }
    }
}

DrawCallData::DrawCallData(std::string_view model)
    : modelPath(model) {}

DrawCallData::DrawCallData(std::string_view model, const glm::mat4& modelMat)
    : modelPath(model), modelMatrix(modelMat){}

DrawCallData::DrawCallData(std::string_view model, std::string_view texture, const glm::mat4& modelMat)
    : modelPath(model), texturePath(texture), modelMatrix(modelMat) {}

void RenderPipeline::init() {
	RHIBase::init();
	
}

void RenderPipeline::prepare() {
    RHIBase::prepare();

    camera = { 5.0f, 1.4f, -6.0f };
    camera.yaw = 127.0f;
    /* lighting a square */
    light.position = { -4.1393897533416748f,
                10.0f,
                -2.0f };
    light.front = { 0.42f, -0.90f, 0.10f };
    light.zoom = 45.0f;
    light.right = glm::cross(light.front, light.up);
    light.yaw = 12.90f;
    light.pitch = -64.3f;
    light.nearPlane = 5.0f;
    light.farPlane = 100.0f;

    /* lighting the whole cave*/
    /*light.position = { 7.0f,
        2.0f,
        -6.0f };
    light.front = glm::normalize(glm::vec3(-0.7f, 0.0f, 0.7f));
    light.zoom = 45.0f;
    light.right = glm::cross(light.front, light.up);*/

    glfwSetScrollCallback(instance.window, scroll_callback);
    glfwSetMouseButtonCallback(instance.window, mouse_button_callback);
    glfwSetCursorPosCallback(instance.window, mouse_callback);
    glfwSetKeyCallback(instance.window, keyCallBack);

    prepareGBuffers();

    createDescriptorSetLayout();
    createUniformBuffers();
    prepareDrawCallResources();

    createOffscreenPass();
    createOffscreenFramebuffer();
    createShaders();
    createPipelines();
}

void RenderPipeline::clear() {
	device.getDevice().waitIdle();
}

void RenderPipeline::createOffscreenPass() {

    std::array<vk::AttachmentDescription, 4> attachmentDescs{};
    std::array<vk::AttachmentReference, attachmentDescs.size() - 1> colorRefs;

    /* position */
    attachmentDescs[0] = vk::AttachmentDescription{
        .format = gBuffer.position.format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eColorAttachmentOptimal
    };
    /* normal */
    attachmentDescs[1] = vk::AttachmentDescription{
        .format = gBuffer.normal.format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eColorAttachmentOptimal
    };
    /* albedo */
    attachmentDescs[2] = vk::AttachmentDescription{
        .format = gBuffer.albedo.format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eColorAttachmentOptimal
    };
    /* depth */
    attachmentDescs[3] = vk::AttachmentDescription{
        .format = gBuffer.depth.format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal
    };

    colorRefs[0] = vk::AttachmentReference{
        .attachment = 0,
        .layout = vk::ImageLayout::eColorAttachmentOptimal
    };

    colorRefs[1] = vk::AttachmentReference{
        .attachment = 1,
        .layout = vk::ImageLayout::eColorAttachmentOptimal
    };

    colorRefs[2] = vk::AttachmentReference{
        .attachment = 2,
        .layout = vk::ImageLayout::eColorAttachmentOptimal
    };

    vk::AttachmentReference depthRef{
        .attachment = 3,
        .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal
    };

    vk::SubpassDescription subPass{
        .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = static_cast<uint32_t>(colorRefs.size()),
        .pColorAttachments = colorRefs.data(),
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = &depthRef,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr,
    };

    std::vector<vk::SubpassDependency> dependencies;

    dependencies.push_back({
        .srcSubpass = vk::SubpassExternal,
        .dstSubpass = 0,
        .srcStageMask =
            vk::PipelineStageFlagBits::eFragmentShader,
        .dstStageMask =
            vk::PipelineStageFlagBits::eEarlyFragmentTests,
        .srcAccessMask =
            vk::AccessFlagBits::eShaderRead,
        .dstAccessMask =
            vk::AccessFlagBits::eDepthStencilAttachmentWrite
        });

    dependencies.push_back({
        .srcSubpass = vk::SubpassExternal,
        .dstSubpass = 0,
        .srcStageMask =
            vk::PipelineStageFlagBits::eFragmentShader,
        .dstStageMask =
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
        .srcAccessMask =
            vk::AccessFlagBits::eShaderRead,
        .dstAccessMask =
            vk::AccessFlagBits::eColorAttachmentWrite
        });

    vk::RenderPassCreateInfo renderPassCI{
        .attachmentCount = static_cast<uint32_t>(attachmentDescs.size()),
        .pAttachments = attachmentDescs.data(),
        .subpassCount = 1,
        .pSubpasses = &subPass,
        .dependencyCount = static_cast<uint32_t>(dependencies.size()),
        .pDependencies = dependencies.data()
    };

    offscreenPass = device.getDevice().createRenderPass(renderPassCI, nullptr);
}

void RenderPipeline::createOffscreenFramebuffer() {
    assert(gBuffer.depth.image && "Cannot create g buffer before creating the image!");
    assert(offscreenPass && "Cannot create offscreen frame buffer before creating the offscreen render pass!");

    std::array<vk::ImageView, NUM_GBUFFER_TEXTURES> attachments{};
    attachments[0] = gBuffer.position.view;
    attachments[1] = gBuffer.normal.view;
    attachments[2] = gBuffer.albedo.view;
    attachments[3] = gBuffer.depth.view;

    vk::FramebufferCreateInfo frameBufferCI{
        .renderPass = offscreenPass,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .width = OFFSCREEN_DIM,
        .height = OFFSCREEN_DIM,
        .layers = 1
    };
    offscreenFramebuffer = device.getDevice().createFramebuffer(frameBufferCI, nullptr);
}

//void compileShader(const std::string& shaderName) {
//#if defined(_WIN32) || defined(_WIN64)
//    std::string compiler = "glslc.exe";
//#elif defined(__APPLE__)
//    std::string compiler = "glslc_macos";
//#else
//    #error "Platform not supported!"
//#endif
//    auto path = std::string(vki::shaderPath) + shaderName;
//    auto command = (std::string(vki::shaderPath) + compiler + " " + path + ".vert -o " + path + ".vert.spv");
//    assert(system((std::string(vki::shaderPath) + compiler + " " + path + ".vert -o " + path + ".vert.spv").c_str()) != -1);
//    assert(system((std::string(vki::shaderPath) + compiler + " " + path + ".frag -o " + path + ".frag.spv").c_str()) != -1);
//}

void RenderPipeline::createShaders() {
    auto vertCode = vki::readFile(std::string(vki::shaderPath) + "/offscreen.vert.spv");
    offscreenVertShader = vki::Shader(device, vertCode);
    auto fragCode = vki::readFile(std::string(vki::shaderPath) + "/offscreen.frag.spv");
    offscreenFragShader = vki::Shader(device, fragCode);
    vertCode = vki::readFile(std::string(vki::shaderPath) + "/light.vert.spv");
    lightVertShader = vki::Shader(device, vertCode);
    fragCode = vki::readFile(std::string(vki::shaderPath) + "/light.frag.spv");
    lightFragShader = vki::Shader(device, fragCode);
    fragCode = vki::readFile(std::string(vki::shaderPath) + "/pcss.frag.spv");
    pcssFragShader = vki::Shader(device, fragCode);
}

void RenderPipeline::createModelBuffers(vki::Model& model) {
    if (!model.vertexBuffers.empty()) return;
    for (auto& vertBuffer : model.vertexData) {
        auto sz = sizeof(vertBuffer[0]) * vertBuffer.size();
        vki::StagingBuffer vertStaging{ device, sz };

        void* data;

        data = device.getDevice().mapMemory(vertStaging.mem, 0, sz);
        memcpy(data, vertBuffer.data(), (size_t)sz);
        device.getDevice().unmapMemory(vertStaging.mem);

        vki::Buffer vertexBuffer{
            device, sz, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                vk::MemoryPropertyFlagBits::eDeviceLocal
        };

        copyBuffer(vertStaging.buffer, vertexBuffer.buffer, sz);

        model.vertexBuffers.push_back(std::move(vertexBuffer));
    }
        
    for (auto& indBuffer : model.indexData) {
        auto sz = sizeof(indBuffer[0]) * indBuffer.size();
        vki::StagingBuffer indStaging{ device, sz };

        void* data;

        data = device.getDevice().mapMemory(indStaging.mem, 0, sz);
        memcpy(data, indBuffer.data(), (size_t)sz);
        device.getDevice().unmapMemory(indStaging.mem);

        vki::Buffer indexBuffer{
            device, sz, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
                vk::MemoryPropertyFlagBits::eDeviceLocal
        };

        copyBuffer(indStaging.buffer, indexBuffer.buffer, sz);

        model.indexBuffers.push_back(std::move(indexBuffer));
    }
}

void RenderPipeline::createUniformBuffers() {
    vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
    uniformBuffers.reserve(drawCmdBuffers.size());

    for (size_t i = 0; i < drawCmdBuffers.size(); i++) {
        uniformBuffers.emplace_back(device, bufferSize);
    }

    /*bufferSize = sizeof(LightUbo);
    lightUbos.reserve(drawCmdBuffers.size());

    for (size_t i = 0; i < drawCmdBuffers.size(); i++) {
        lightUbos.emplace_back(device, bufferSize);
    }*/
}

void RenderPipeline::createDescriptorPool() {
    std::vector<vk::DescriptorPoolSize> poolSizes(2);
    poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(drawCmdBuffers.size() * 2);
    poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(drawCmdBuffers.size() * 128);

    vk::DescriptorPoolCreateInfo poolInfo{
        .maxSets = static_cast<uint32_t>(drawCmdBuffers.size() * 64),
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data()
    };
    descriptorPool = device.getDevice().createDescriptorPool(poolInfo);
};

void RenderPipeline::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
    uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

    vk::DescriptorSetLayoutBinding textureLayoutBinding{};
    textureLayoutBinding.binding = 1;
    textureLayoutBinding.descriptorCount = 1;
    textureLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    textureLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    vk::DescriptorSetLayoutBinding gBufferLayoutBinding{
        .binding = 2,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .descriptorCount = NUM_GBUFFER_TEXTURES,
        .stageFlags = vk::ShaderStageFlagBits::eFragment
    };

    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{ uboLayoutBinding , textureLayoutBinding, gBufferLayoutBinding };

    vk::DescriptorSetLayoutCreateInfo layoutInfo{
        .bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
        .pBindings = setLayoutBindings.data()
    };

    lightDescLayout = device.getDevice().createDescriptorSetLayout(layoutInfo, nullptr);

    textureLayoutBinding.setBinding(0);
    layoutInfo.setBindingCount(1).setPBindings(&textureLayoutBinding);
    offscreenDescLayout = device.getDevice().createDescriptorSetLayout(layoutInfo, nullptr);
}

vki::Texture2D RenderPipeline::createTextureImage(const std::string& file) {
    int width, height, nrChannels;
    vki::Texture2D texture;
    unsigned char* textureData = stbi_load(file.c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
    assert(textureData);
    // 4 bytes a pixel: R8G8B8A8
    auto bufferSize = width * height * 4;
    texture.width = width;
    texture.height = height;
    texture.mipLevels = 1;
    // image format
    vk::Format format = vk::Format::eR8G8B8A8Unorm;

    vki::StagingBuffer staging{ device, static_cast<vk::DeviceSize>(bufferSize) };

    void* data;

    data = device.getDevice().mapMemory(staging.mem, 0, bufferSize);
    memcpy(data, textureData, bufferSize);
    device.getDevice().unmapMemory(staging.mem);


    vk::BufferImageCopy bufferCopyRegion = {
        .bufferOffset = 0,
        .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
        .imageExtent = {texture.width, texture.height, 1},
    };

    vk::ImageCreateInfo imageCI{
        .imageType = vk::ImageType::e2D,
        .format = format,
        .extent = { texture.width, texture.height, 1 },
        .mipLevels = texture.mipLevels,
        .arrayLayers = 1,
        .samples = vk::SampleCountFlagBits::e1,
        .tiling = vk::ImageTiling::eOptimal,
        .usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        .sharingMode = vk::SharingMode::eExclusive,
        .initialLayout = vk::ImageLayout::eUndefined
    };

    texture.image = device.getDevice().createImage(imageCI);
    auto memReqs = device.getDevice().getImageMemoryRequirements(texture.image);

    vk::MemoryAllocateInfo memAI{
        .allocationSize = memReqs.size,
        .memoryTypeIndex = device.getMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
    };

    texture.mem = device.getDevice().allocateMemory(memAI);
    device.getDevice().bindImageMemory(texture.image, texture.mem, 0);

    // Image memory barriers for the texture image

    // The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
    vk::ImageSubresourceRange subresourceRange = {
        // Image only contains color data
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        // Start at first mip level
        .baseMipLevel = 0,
        // We will transition on all mip levels
        .levelCount = texture.mipLevels,
        // The 2D texture only has one layer
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    // Transition the texture image layout to transfer target, so we can safely copy our buffer data to it.
    vk::ImageMemoryBarrier imageMemoryBarrier = {
        .srcAccessMask = vk::AccessFlagBits::eNone,
        .dstAccessMask = vk::AccessFlagBits::eTransferWrite,
        .oldLayout = vk::ImageLayout::eUndefined,
        .newLayout = vk::ImageLayout::eTransferDstOptimal,
        .image = texture.image,
        .subresourceRange = subresourceRange,
    };

    vk::CommandBuffer commandBuffer = beginSingleTimeCommands();
    // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
    // Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
    // Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eHost,
        vk::PipelineStageFlagBits::eTransfer,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier
    );

    // Copy mip levels from staging buffer
    commandBuffer.copyBufferToImage(
        staging.buffer,
        texture.image,
        vk::ImageLayout::eTransferDstOptimal,
        1,
        &bufferCopyRegion
    );


    // Once the data has been uploaded we transfer to the texture image to the shader read layout, so it can be sampled from
    imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    imageMemoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    imageMemoryBarrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    // Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
    // Source pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
    // Destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eFragmentShader,
        vk::DependencyFlags(),
        0, nullptr,
        0, nullptr,
        1, &imageMemoryBarrier
    );

    endSingleTimeCommands(commandBuffer);
    // Store current layout for later reuse
    // texture.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

    stbi_image_free(textureData);

    // Create a texture sampler
    // In Vulkan textures are accessed by samplers
    // This separates all the sampling information from the texture data. This means you could have multiple sampler objects for the same texture with different settings
    // Note: Similar to the samplers available with OpenGL 3.3
    vk::SamplerCreateInfo samplerCI{
        .magFilter = vk::Filter::eLinear,
        .minFilter = vk::Filter::eLinear,
        .mipmapMode = vk::SamplerMipmapMode::eLinear,
        .addressModeU = vk::SamplerAddressMode::eRepeat,
        .addressModeV = vk::SamplerAddressMode::eRepeat,
        .addressModeW = vk::SamplerAddressMode::eRepeat,
        .mipLodBias = 0.0f,
        // Enable anisotropic filtering
        // This feature is optional, so we must check if it's supported on the device
        // TODO: Check it
        // Use max. level of anisotropy for this example
        .anisotropyEnable = vk::True,
        .maxAnisotropy = instance.supports.properties.limits.maxSamplerAnisotropy,
        .compareOp = vk::CompareOp::eNever,
        .minLod = 0.0f,
        // Set max level-of-detail to mip level count of the texture
        .maxLod = (float)texture.mipLevels,
        .borderColor = vk::BorderColor::eFloatOpaqueWhite
    };

    texture.sampler = device.getDevice().createSampler(samplerCI);

    vk::ImageViewCreateInfo viewCI{
        .image = texture.image,
        .viewType = vk::ImageViewType::e2D,
        .format = format,
        // The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
        // It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
        .subresourceRange = {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = texture.mipLevels,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    texture.view = device.getDevice().createImageView(viewCI);
    return texture;
}

void RenderPipeline::prepareGBuffers() {
    gBuffer.position = { vki::Texture2D(device, OFFSCREEN_DIM, OFFSCREEN_DIM,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled) };

    gBuffer.normal = { vki::Texture2D(device, OFFSCREEN_DIM, OFFSCREEN_DIM,
        vk::Format::eR16G16B16A16Sfloat,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled) };

    gBuffer.albedo = { vki::Texture2D(device, OFFSCREEN_DIM, OFFSCREEN_DIM,
        vk::Format::eR8G8B8A8Unorm,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled) };

    gBuffer.depth = { vki::Texture2D(device, OFFSCREEN_DIM, OFFSCREEN_DIM,
        DEPTH_FORMAT,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled) };
}

void RenderPipeline::updateUniformBuffer(uint32_t frame) {
    // camera.front = glm::normalize(glm::vec3(0.0f, 1.5f, 0.0f) - camera.position);
    // camera = light;
    UniformBufferObject ubo{};
    //ubo.model = glm::mat4(
    //    glm::vec4(1, 0, 0, 0),  // First column
    //    glm::vec4(0, 0, -1, 0), // Second column
    //    glm::vec4(0, -1, 0, 0), // Third column
    //    glm::vec4(0, 0, 0, 1)   // Fourth column
    //);
    // ubo.normalRot = glm::mat3(glm::transpose(glm::inverse(ubo.model)));
    ubo.view = activeCamera->view();
    ubo.proj = activeCamera->projection((float)instance.width, (float)instance.height);
    ubo.lightPos = { light.position, 1.0f };
    ubo.viewPos = { activeCamera->position, 1.0f };
    ubo.lightFov = { glm::radians(light.zoom), 0.0f, 0.0f, 0.0f };
    // glm is originally for OpenGL, whose y coord of the clip space is inverted
    ubo.proj[1][1] *= -1;

    auto proj = light.projection((float)OFFSCREEN_DIM, (float)OFFSCREEN_DIM, light.nearPlane, light.farPlane);
    proj[1][1] *= -1;

    ubo.lightPV = proj * light.view();

    /*LightUbo lightUbo{
        .lightMVP =
        ubo.lightPV
        * ubo.model
    };*/

    memcpy(uniformBuffers[frame].mapped, &ubo, sizeof(ubo));
    // memcpy(lightUbos[frame].mapped, &lightUbo, sizeof(lightUbo));
}

void RenderPipeline::createDescSetFromTexture(std::string_view texture) {
    std::string textName{ texture };
    if (lightDescSets.find(textName) != lightDescSets.end()) return;
    assert(textures.find(textName) != textures.end());

    // offscreen descriptor sets
    {
        std::vector<vk::DescriptorSetLayout> setLayouts(drawCmdBuffers.size(), offscreenDescLayout);
        vk::DescriptorSetAllocateInfo descriptorSetAI{
            .descriptorPool = descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(setLayouts.size()),
            .pSetLayouts = setLayouts.data()
        };

        offscreenDescSets[textName] = device.getDevice().allocateDescriptorSets(descriptorSetAI);

        std::vector<vk::DescriptorImageInfo> imageInfos(1);
        imageInfos[0] = {
            .sampler = textures[textName].sampler,
            .imageView = textures[textName].view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };

        for (size_t i = 0; i < drawCmdBuffers.size(); i++) {
            std::array<vk::WriteDescriptorSet, 1> descriptorWrites{};
            // texture
            descriptorWrites[0] = {
                .dstSet = offscreenDescSets[textName][i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = static_cast<uint32_t>(imageInfos.size()),
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = imageInfos.data(),
                .pBufferInfo = nullptr
            };
            device.getDevice().updateDescriptorSets(
                static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    // light descriptor sets
    {
        std::vector<vk::DescriptorSetLayout> setLayouts(drawCmdBuffers.size(), lightDescLayout);
        vk::DescriptorSetAllocateInfo descriptorSetAI{
            .descriptorPool = descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(setLayouts.size()),
            .pSetLayouts = setLayouts.data()
        };

        lightDescSets[textName] = device.getDevice().allocateDescriptorSets(descriptorSetAI);

        std::vector<vk::DescriptorImageInfo> imageInfos(1);
        imageInfos[0] = {
            .sampler = textures[textName].sampler,
            .imageView = textures[textName].view,
            .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
        };

        std::array<vk::DescriptorImageInfo, NUM_GBUFFER_TEXTURES> offscreenImageInfos{};
        offscreenImageInfos[0] = { gBuffer.position.sampler, gBuffer.position.view, vk::ImageLayout::eShaderReadOnlyOptimal };
        offscreenImageInfos[1] = { gBuffer.normal.sampler, gBuffer.normal.view, vk::ImageLayout::eShaderReadOnlyOptimal };
        offscreenImageInfos[2] = { gBuffer.albedo.sampler, gBuffer.albedo.view, vk::ImageLayout::eShaderReadOnlyOptimal };
        offscreenImageInfos[3] = { gBuffer.depth.sampler, gBuffer.depth.view, vk::ImageLayout::eDepthStencilReadOnlyOptimal };

        for (size_t i = 0; i < drawCmdBuffers.size(); i++) {
            vk::DescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            std::array<vk::WriteDescriptorSet, 3> descriptorWrites{};
            // uniform buffer
            descriptorWrites[0] = {
                .dstSet = lightDescSets[textName][i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .pImageInfo = nullptr,
                .pBufferInfo = &bufferInfo
            };
            // texture
            descriptorWrites[1] = {
                .dstSet = lightDescSets[textName][i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = static_cast<uint32_t>(imageInfos.size()),
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = imageInfos.data(),
                .pBufferInfo = nullptr
            };
            // gBuffer
            descriptorWrites[2] = {
                .dstSet = lightDescSets[textName][i],
                .dstBinding = 2,
                .dstArrayElement = 0,
                .descriptorCount = static_cast<uint32_t>(offscreenImageInfos.size()),
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .pImageInfo = offscreenImageInfos.data(),
                .pBufferInfo = nullptr
            };
            device.getDevice().updateDescriptorSets(
                static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }
    
}

struct OffscreenPushConstants {
    glm::mat4 model;
    glm::mat4 lightVP;
};

struct LightPushConstants {
    glm::mat4 model;
    glm::mat4 normalRot;
};

void RenderPipeline::createPipelines() {
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eVertex,
        .module = offscreenVertShader.shaderModule,
        .pName = "main"
    };

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
        .stage = vk::ShaderStageFlagBits::eFragment,
        .module = offscreenFragShader.shaderModule,
        .pName = "main"
    };

    vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    vk::VertexInputBindingDescription vertexInputBinding{
        .binding = 0,
        .stride = sizeof(vki::Vertex),
        .inputRate = vk::VertexInputRate::eVertex
    };

    std::array<vk::VertexInputAttributeDescription, 3> vertexInputAttribs{
        vk::VertexInputAttributeDescription {
            .location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(vki::Vertex, pos)
        },
        vk::VertexInputAttributeDescription {
            .location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(vki::Vertex, normal)
        },
        vk::VertexInputAttributeDescription {
            .location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(vki::Vertex, texCoord)
        }
    };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertexInputBinding,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttribs.size()),
        .pVertexAttributeDescriptions = vertexInputAttribs.data()
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        .topology = vk::PrimitiveTopology::eTriangleList,
        .primitiveRestartEnable = vk::False
    };

    vk::PipelineViewportStateCreateInfo viewportState{
        .viewportCount = 1,
        .scissorCount = 1
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .depthClampEnable = vk::False,
        .rasterizerDiscardEnable = vk::False,
        .polygonMode = vk::PolygonMode::eFill,
        .cullMode = vk::CullModeFlagBits::eNone ,
        .frontFace = vk::FrontFace::eClockwise,
        .depthBiasEnable = vk::False,
        .lineWidth = 1.0f,
    };

    vk::PipelineDepthStencilStateCreateInfo depthStencil{
        .depthTestEnable = vk::True,
        .depthWriteEnable = vk::True,
        .depthCompareOp = vk::CompareOp::eLess,
        .stencilTestEnable = vk::False
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{
        .rasterizationSamples = vk::SampleCountFlagBits::e1,
        .sampleShadingEnable = vk::False
    };

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = vk::False,
        .colorWriteMask =
        vk::ColorComponentFlagBits::eR |
        vk::ColorComponentFlagBits::eG |
        vk::ColorComponentFlagBits::eB |
        vk::ColorComponentFlagBits::eA
    };

    std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments;
    for (int i = 0; i + 1 < NUM_GBUFFER_TEXTURES; i++) {
        colorBlendAttachments.push_back(colorBlendAttachment);
    }

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .logicOpEnable = vk::False,
        .attachmentCount = static_cast<uint32_t>(colorBlendAttachments.size()),
        .pAttachments = colorBlendAttachments.data()
    };

    std::vector<vk::DynamicState> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    vk::PipelineDynamicStateCreateInfo dynamicState{
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    vk::PushConstantRange offscreenPushConstRange{
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .offset = 0,
        .size = sizeof(OffscreenPushConstants)
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutCI{
        .setLayoutCount = 1,
        .pSetLayouts = &offscreenDescLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &offscreenPushConstRange
    };

    offscreenPipelineLayout = device.getDevice().createPipelineLayout(pipelineLayoutCI);

    vk::GraphicsPipelineCreateInfo pipelineCI{
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = offscreenPipelineLayout,
        .renderPass = offscreenPass,
        .subpass = 0
    };

    offscreenPipeline = device.getDevice().createGraphicsPipeline(nullptr, pipelineCI).value;

    vertShaderStageInfo.setModule(lightVertShader.shaderModule);
    fragShaderStageInfo.setModule(lightFragShader.shaderModule);

    shaderStages[0] = vertShaderStageInfo;
    shaderStages[1] = fragShaderStageInfo;

    vk::PushConstantRange lightPushConstRange{
        .stageFlags = vk::ShaderStageFlagBits::eVertex,
        .offset = 0,
        .size = sizeof(LightPushConstants)
    };

    pipelineLayoutCI
        .setSetLayoutCount(1)
        .setPSetLayouts(&lightDescLayout)
        .setPushConstantRanges(lightPushConstRange);

    lightPipelineLayout = device.getDevice().createPipelineLayout(pipelineLayoutCI);

    pipelineCI.layout = lightPipelineLayout;
    pipelineCI.renderPass = renderPass;
    colorBlending.setAttachmentCount(1).setPAttachments(&colorBlendAttachment);
    lightPipeline = device.getDevice().createGraphicsPipeline(nullptr, pipelineCI).value;

    fragShaderStageInfo.setModule(pcssFragShader.shaderModule);

    shaderStages[1] = fragShaderStageInfo;
    pcssLightPipeline = device.getDevice().createGraphicsPipeline(nullptr, pipelineCI).value;
}

void RenderPipeline::prepareDrawCallResources() {
    for (auto& drawCall : drawCalls) {
        if (models.find(drawCall.modelPath) == models.end()) {
            models[drawCall.modelPath] = vki::loadGLTFModel(loader, vki::modelPath + drawCall.modelPath);
        }
        createModelBuffers(models[drawCall.modelPath]);
        if (textures.find(drawCall.texturePath) == textures.end()) {
            textures[drawCall.texturePath] = createTextureImage(std::string(vki::modelPath) + drawCall.texturePath);
            createDescSetFromTexture(drawCall.texturePath);
        }
    }
}

void RenderPipeline::prepareFrame() {
    prepareDrawCallResources();
    RHIBase::prepareFrame();
}

void RenderPipeline::buildCommandBuffer() {
    const auto& commandBuffer = drawCmdBuffers[currentBuffer];

    commandBuffer.reset();

    vk::CommandBufferBeginInfo cmdBufBeginInfo{};
    commandBuffer.begin(cmdBufBeginInfo);

    vk::RenderPassBeginInfo renderPassBeginInfo{
        .renderPass = offscreenPass,
        .framebuffer = offscreenFramebuffer,
        .renderArea {.offset = { 0, 0 }, .extent = {OFFSCREEN_DIM, OFFSCREEN_DIM}}
    };

    std::array<vk::ClearValue, 4> clearValues{};
    clearValues[0].color.setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
    clearValues[1].color.setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
    clearValues[2].color.setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
    clearValues[3].depthStencil = { 1.0f, 0 };

    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, offscreenPipeline);

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(OFFSCREEN_DIM);
    viewport.height = static_cast<float>(OFFSCREEN_DIM);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    commandBuffer.setViewport(0, viewport);

    vk::Rect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { OFFSCREEN_DIM, OFFSCREEN_DIM };
    commandBuffer.setScissor(0, scissor);

    auto proj = light.projection((float)OFFSCREEN_DIM, (float)OFFSCREEN_DIM, light.nearPlane, light.farPlane);
    proj[1][1] *= -1;
    auto lightVP = proj * light.view();
    for (auto& drawCall : drawCalls) {
        OffscreenPushConstants constants{
            .model = drawCall.modelMatrix,
            .lightVP = lightVP
        };
        commandBuffer.pushConstants(offscreenPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(constants), &constants);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, offscreenPipelineLayout, 0, 1, &offscreenDescSets.at(drawCall.texturePath)[currentBuffer], 0, nullptr);
        for (size_t i = 0; i < models[drawCall.modelPath].vertexBuffers.size(); i++) {
            commandBuffer.bindVertexBuffers(0, models[drawCall.modelPath].vertexBuffers[i].buffer, {0});
            commandBuffer.bindIndexBuffer(models[drawCall.modelPath].indexBuffers[i].buffer, 0, vk::IndexType::eUint16);
            commandBuffer.drawIndexed(static_cast<uint32_t>(models[drawCall.modelPath].indexData[i].size()), 1, 0, 0, 0);
        }
    }
    
    commandBuffer.endRenderPass();

    std::array<vk::ImageMemoryBarrier, NUM_GBUFFER_TEXTURES> imageMemoryBarriers{};

    imageMemoryBarriers[0] = {
        .srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        .dstAccessMask = vk::AccessFlagBits::eShaderRead,
        .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        .image = gBuffer.position.image,
        .subresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    imageMemoryBarriers[1] = {
        .srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        .dstAccessMask = vk::AccessFlagBits::eShaderRead,
        .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        .image = gBuffer.normal.image,
        .subresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    imageMemoryBarriers[2] = {
        .srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
        .dstAccessMask = vk::AccessFlagBits::eShaderRead,
        .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
        .image = gBuffer.albedo.image,
        .subresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    imageMemoryBarriers[3] = {
        .srcAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite,
        .dstAccessMask = vk::AccessFlagBits::eShaderRead,
        .oldLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
        .newLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal,
        .image = gBuffer.depth.image,
        .subresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eDepth,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    commandBuffer.pipelineBarrier(
        vk::PipelineStageFlagBits::eEarlyFragmentTests 
        | vk::PipelineStageFlagBits::eLateFragmentTests
        | vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eFragmentShader,
        vk::DependencyFlags(),
        nullptr,
        nullptr,
        imageMemoryBarriers
    );

    clearValues[0].color.setFloat32({ 0.0f, 0.0f, 0.0f, 1.0f });
    clearValues[1].depthStencil = vk::ClearDepthStencilValue({ 1.0f, 0 });

    renderPassBeginInfo.setRenderPass(renderPass)
        .setFramebuffer(frameBuffers[currentBuffer])
        .setRenderArea({ .offset = { 0, 0 }, .extent = {instance.width, instance.height} })
        .setClearValueCount(static_cast<uint32_t>(clearValues.size()))
        .setPClearValues(clearValues.data());

    commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
    if (!pcssEnabled) 
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, lightPipeline);
    else 
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pcssLightPipeline);
    viewport.setWidth(static_cast<float>(instance.width)).setHeight(static_cast<float>(instance.height));
    scissor.setExtent({ instance.width, instance.height });
    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, scissor);

    for (auto& drawCall : drawCalls) {
        LightPushConstants constants{
            .model = drawCall.modelMatrix,
            .normalRot = glm::mat4(glm::transpose(glm::inverse(drawCall.modelMatrix)))
        };
        commandBuffer.pushConstants(lightPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(constants), &constants);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, lightPipelineLayout, 0, 1, &lightDescSets.at(drawCall.texturePath)[currentBuffer], 0, nullptr);
        for (size_t i = 0; i < models[drawCall.modelPath].vertexBuffers.size(); i++) {
            commandBuffer.bindVertexBuffers(0, models[drawCall.modelPath].vertexBuffers[i].buffer, { 0 });
            commandBuffer.bindIndexBuffer(models[drawCall.modelPath].indexBuffers[i].buffer, 0, vk::IndexType::eUint16);
            commandBuffer.drawIndexed(static_cast<uint32_t>(models[drawCall.modelPath].indexData[i].size()), 1, 0, 0, 0);
        }
    }

    commandBuffer.endRenderPass();

    commandBuffer.end();
}

void RenderPipeline::render() {
    static int count = 0;
    auto result = device.getDevice().waitForFences(1, &fences[currentBuffer], VK_TRUE, UINT64_MAX);
    assert(result == vk::Result::eSuccess);
    prepareFrame();

    // Set fence to unsignaled
    // Must delay this to after recreateSwapChain to avoid deadlock
    result = device.getDevice().resetFences(1, &fences[currentBuffer]);
    assert(result == vk::Result::eSuccess);

    buildCommandBuffer();

    activeCamera->update(instance.window);
    updateUniformBuffer(currentBuffer);

    vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::SubmitInfo submitInfo{
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &semaphores.presentComplete,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &drawCmdBuffers[currentBuffer],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &semaphores.renderComplete
    };

    graphicsQueue.submit(submitInfo, fences[currentBuffer]);
    presentFrame();

    drawCalls.clear();
}

void RenderPipeline::switchCamera() {
    if (activeCamera != &camera) {
        activeCamera = &camera;
    } else {
        activeCamera = &light;
    }
}

void RenderPipeline::switchShadowQuality() {
    pcssEnabled = !pcssEnabled;
}

