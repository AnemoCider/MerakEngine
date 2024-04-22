#include "runtime/engine.h"
#include "runtime/function/global/global_context.h"
//#include "course/Helper.h"
#include <chrono>

#include <glm/glm.hpp>
#include <iostream>
#include <functional>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

void Merak::MerakEngine::gameStart() {
	g_context.componentSystem.startUp();
	g_context.renderSystem.startUp();
	g_context.assetManager.startUp();
	g_context.worldManager.startUp();
	g_context.inputSystem.startUp();
	// g_context.physicsManager.startUp();
}

void Merak::MerakEngine::tickOneFrame() {
	static constexpr int targetFPS = 120;
	static constexpr std::chrono::nanoseconds timePerFrame = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)) / targetFPS;
	static std::chrono::steady_clock::time_point lastFrameTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	auto duration = now - lastFrameTime;
	if (duration < timePerFrame) {
		return;
	}
	lastFrameTime = now;
	beginOfFrame();
	logicTick();
	renderTick();
	endOfFrame();
}

void Merak::MerakEngine::beginOfFrame() {
	g_context.worldManager.processNewScene();
}

void Merak::MerakEngine::endOfFrame() {
	g_context.inputSystem.processEndFrame();
	g_context.worldManager.processManipulation();
}

void Merak::MerakEngine::gameEnd() {
	// cleanUps are done in destructors.
	// g_context.inputSystem.cleanUp();
	// g_context.worldManager.cleanUp();
	// g_context.assetManager.cleanUp();
	// g_context.renderSystem.cleanUp();
	// g_context.componentSystem.cleanUp();
}

void Merak::MerakEngine::logicTick() {
	g_context.inputSystem.tick();
	g_context.worldManager.tick();
	g_context.eventManager.process();
	// g_context.physicsManager.tick();
}

void Merak::MerakEngine::renderTick() {
	g_context.renderSystem.tick();
}


bool Merak::MerakEngine::shouldContinue() {
	return !g_context.renderSystem.shouldClose();
}
