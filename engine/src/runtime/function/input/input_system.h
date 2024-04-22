#pragma once

//#include "SDL2/SDL.h"
#include <unordered_map>
#include <vector>
#include <string>
#include "glm/glm.hpp"

namespace Merak {
	enum InputStates {
		// not pressed
		UP = 0,
		// pressed at the current frame
		JUSTDOWN,
		// pressed before the current frame and not yet released
		DOWN,
		// released at the current frame
		JUSTUP
	};

	class InputSystem {
	private:
		/*static inline std::unordered_map<SDL_Scancode, InputStates> keyboard_states;
		static inline std::vector<SDL_Scancode> just_became_down_scancodes;
		static inline std::vector<SDL_Scancode> just_became_up_scancodes;*/
		static inline glm::vec2 mousePos{0.0f, 0.0f};
		static inline auto mouseButtonStates = std::vector<InputStates>(4);

	public:

		bool signalQuit = false;

		static inline float mouseScrollY = 0.0f;

		void startUp();
		void cleanUp();
		~InputSystem();

		/*static void processKeyDown(const SDL_Scancode& code);
		static void processKeyUp(const SDL_Scancode& code);*/
		static void processEndFrame();

		/*static void  processMouseButtonDown(Uint8 button);
		static void processMouseButtonUp(Uint8 button);*/

		// Get whether certain key is down currently.
		// This is a continuous operation.
		static bool getKey(const std::string& key);

		// Get whether certain key was pressed down in the current frame.
		// This operation is transient.
		static bool getKeyDown(const std::string& key);

		// Get whether certain key was released in the current frame
		// Transient
		static bool getKeyUp(const std::string& key);

		static bool getMouseButton(int button);

		static bool getMouseButtonDown(int button);

		static bool getMouseButtonUp(int button);

		static void updateMousePos(int x, int y);

		static glm::vec2 getMousePos();

		void tick();
	};
};