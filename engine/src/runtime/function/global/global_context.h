#pragma once

#include "runtime/resource/asset_manager.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/framework/component/component_system.h"
#include "runtime/function/render/render_system.h"
#include "runtime/function/input/input_system.h"
// #include "runtime/function/physics/physics_manager.h"
#include "runtime/function/framework/event/event_manager.h"

namespace Merak {
	class GlobalContext{
	public:
		AssetManager assetManager;
		ComponentSystem componentSystem;
		WorldManager worldManager;
		RenderSystem renderSystem;
		InputSystem inputSystem;
		// PhysicsManager physicsManager;
		EventManager eventManager;
	};

	extern GlobalContext g_context;
};