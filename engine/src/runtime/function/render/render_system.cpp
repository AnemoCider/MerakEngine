#include "runtime/function/render/render_system.h"
//#include "course/Helper.h"
//#include "SDL_image/SDL_image.h"
#include <cassert>
#include <cmath>
#include "runtime/function/render/render_math.h"
#include "runtime/function/global/global_context.h"
#include <map>

void Merak::RenderSystem::startUp() {
	rhi = std::make_unique<RenderPipeline>();
	rhi->init();
	rhi->prepare();
}

void Merak::RenderSystem::tick() {
	glfwPollEvents();
    //addDrawCall("/cave/scene.gltf", "/cave/siEoZ_2K_Albedo.jpg", glm::mat4(
    //    glm::vec4(1, 0, 0, 0),  // First column
    //    glm::vec4(0, 0, -1, 0), // Second column
    //    glm::vec4(0, -1, 0, 0), // Third column
    //    glm::vec4(0, 0, 0, 1)   // Fourth column
    //));


    //addDrawCall("/sphere.gltf", "/default/grey.png", glm::mat4(
    //    glm::vec4(0.3f, 0, 0, 0),  // First column
    //    glm::vec4(0, 0.3f, 0, 0), // Second column
    //    glm::vec4(0, 0, 0.3f, 0), // Third column
    //    glm::vec4(0, 1, 0, 1)   // Fourth column
    //));
	rhi->nextFrame();
}

void Merak::RenderSystem::cleanUp() {
	rhi->clear();
}

Merak::RenderSystem::~RenderSystem() {
	cleanUp();
}

bool Merak::RenderSystem::shouldClose() {
	return rhi->shouldClose();
}

void Merak::RenderSystem::addDrawCall(std::string_view model, std::string_view texture, glm::mat4&& modelTransform) {
	rhi->addDrawCall(model, texture, modelTransform);
}
