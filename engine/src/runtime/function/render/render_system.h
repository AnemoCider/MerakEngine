#pragma once

#include <string>
//#include "course/AudioHelper.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/render/render_math.h"
#include "runtime/function/render/render_pipeline.h"

namespace Merak {

	class RenderSystem {
	private:

		std::unique_ptr<RenderPipeline> rhi = nullptr;

	public:

		void startUp();
		void tick();
		void cleanUp();
		bool shouldClose();

		void addDrawCall(std::string_view model, std::string_view texture, glm::mat4&& modelTransform);

		~RenderSystem();
	};
};