#include "input_system.h"
//#include "course/Helper.h"

#include <unordered_map>
#include <string>
//#include "SDL.h"

//const std::unordered_map<std::string, SDL_Scancode> __keycode_to_scancode = {
//	// Directional (arrow) Keys
//	{"up", SDL_SCANCODE_UP},
//	{"down", SDL_SCANCODE_DOWN},
//	{"right", SDL_SCANCODE_RIGHT},
//	{"left", SDL_SCANCODE_LEFT},
//
//	// Misc Keys
//	{"escape", SDL_SCANCODE_ESCAPE},
//
//	// Modifier Keys
//	{"lshift", SDL_SCANCODE_LSHIFT},
//	{"rshift", SDL_SCANCODE_RSHIFT},
//	{"lctrl", SDL_SCANCODE_LCTRL},
//	{"rctrl", SDL_SCANCODE_RCTRL},
//	{"lalt", SDL_SCANCODE_LALT},
//	{"ralt", SDL_SCANCODE_RALT},
//
//	// Editing Keys
//	{"tab", SDL_SCANCODE_TAB},
//	{"return", SDL_SCANCODE_RETURN},
//	{"enter", SDL_SCANCODE_RETURN},
//	{"backspace", SDL_SCANCODE_BACKSPACE},
//	{"delete", SDL_SCANCODE_DELETE},
//	{"insert", SDL_SCANCODE_INSERT},
//
//	// Character Keys
//	{"space", SDL_SCANCODE_SPACE},
//	{"a", SDL_SCANCODE_A},
//	{"b", SDL_SCANCODE_B},
//	{"c", SDL_SCANCODE_C},
//	{"d", SDL_SCANCODE_D},
//	{"e", SDL_SCANCODE_E},
//	{"f", SDL_SCANCODE_F},
//	{"g", SDL_SCANCODE_G},
//	{"h", SDL_SCANCODE_H},
//	{"i", SDL_SCANCODE_I},
//	{"j", SDL_SCANCODE_J},
//	{"k", SDL_SCANCODE_K},
//	{"l", SDL_SCANCODE_L},
//	{"m", SDL_SCANCODE_M},
//	{"n", SDL_SCANCODE_N},
//	{"o", SDL_SCANCODE_O},
//	{"p", SDL_SCANCODE_P},
//	{"q", SDL_SCANCODE_Q},
//	{"r", SDL_SCANCODE_R},
//	{"s", SDL_SCANCODE_S},
//	{"t", SDL_SCANCODE_T},
//	{"u", SDL_SCANCODE_U},
//	{"v", SDL_SCANCODE_V},
//	{"w", SDL_SCANCODE_W},
//	{"x", SDL_SCANCODE_X},
//	{"y", SDL_SCANCODE_Y},
//	{"z", SDL_SCANCODE_Z},
//	{"0", SDL_SCANCODE_0},
//	{"1", SDL_SCANCODE_1},
//	{"2", SDL_SCANCODE_2},
//	{"3", SDL_SCANCODE_3},
//	{"4", SDL_SCANCODE_4},
//	{"5", SDL_SCANCODE_5},
//	{"6", SDL_SCANCODE_6},
//	{"7", SDL_SCANCODE_7},
//	{"8", SDL_SCANCODE_8},
//	{"9", SDL_SCANCODE_9},
//	{"/", SDL_SCANCODE_SLASH},
//	{";", SDL_SCANCODE_SEMICOLON},
//	{"=", SDL_SCANCODE_EQUALS},
//	{"-", SDL_SCANCODE_MINUS},
//	{".", SDL_SCANCODE_PERIOD},
//	{",", SDL_SCANCODE_COMMA},
//	{"[", SDL_SCANCODE_LEFTBRACKET},
//	{"]", SDL_SCANCODE_RIGHTBRACKET},
//	{"\\", SDL_SCANCODE_BACKSLASH},
//	{"'", SDL_SCANCODE_APOSTROPHE}
//};

void Merak::InputSystem::startUp() {
	
}

void Merak::InputSystem::cleanUp() {

}

Merak::InputSystem::~InputSystem() {
	cleanUp();
}

//void Merak::InputSystem::processKeyDown(const SDL_Scancode& code) {
//	keyboard_states[code] = JUSTDOWN;
//	just_became_down_scancodes.push_back(code);
//}
//
//void Merak::InputSystem::processKeyUp(const SDL_Scancode& code) {
//	keyboard_states[code] = JUSTUP;
//	just_became_up_scancodes.push_back(code);
//}
//
//void Merak::InputSystem::processMouseButtonDown(Uint8 button) {
//	mouseButtonStates[button] = JUSTDOWN;
//}
//
//void Merak::InputSystem::processMouseButtonUp(Uint8 button) {
//	mouseButtonStates[button] = JUSTUP;
//}

void Merak::InputSystem::processEndFrame() {
	/*for (auto& code : just_became_down_scancodes) {
		keyboard_states[code] = DOWN;
	}
	just_became_down_scancodes.clear();
	for (auto& code : just_became_up_scancodes) {
		keyboard_states[code] = UP;
	}
	just_became_up_scancodes.clear();*/
	for (size_t i = 1; i <= 3; i++) {
		if (mouseButtonStates[i] == JUSTDOWN) mouseButtonStates[i] = DOWN;
		if (mouseButtonStates[i] == JUSTUP) mouseButtonStates[i] = UP;
	}
	mouseScrollY = 0.0f;
}


bool Merak::InputSystem::getKey(const std::string& key) {
	/*if (__keycode_to_scancode.find(key) == __keycode_to_scancode.end()) return false;
	auto code = __keycode_to_scancode.at(key);
	return keyboard_states[code] == DOWN ||
		keyboard_states[code] == JUSTDOWN;*/
	return true;
}

bool Merak::InputSystem::getKeyDown(const std::string& key) {
	/*if (__keycode_to_scancode.find(key) == __keycode_to_scancode.end()) return false;
	auto code = __keycode_to_scancode.at(key);
	return keyboard_states[code] == JUSTDOWN;*/
	return true;
}

bool Merak::InputSystem::getKeyUp(const std::string& key) {
	/*if (__keycode_to_scancode.find(key) == __keycode_to_scancode.end()) return false;
	auto code = __keycode_to_scancode.at(key);
	return keyboard_states[code] == JUSTUP;*/
	return true;
}

bool Merak::InputSystem::getMouseButton(int button) {
	return button > 0 && button < 4 &&
		(mouseButtonStates[button] == DOWN || mouseButtonStates[button] == JUSTDOWN);
}

bool Merak::InputSystem::getMouseButtonDown(int button) {
	return button > 0 && button < 4 &&
		mouseButtonStates[button] == JUSTDOWN;
}

bool Merak::InputSystem::getMouseButtonUp(int button) {
	return button > 0 && button < 4 &&
		mouseButtonStates[button] == JUSTUP;
}

void Merak::InputSystem::updateMousePos(int x, int y) {
	mousePos = { x, y };
}

glm::vec2 Merak::InputSystem::getMousePos() {
	return mousePos;
}

void Merak::InputSystem::tick() {
	/*SDL_Event sdlEvent{};
	while (Helper::SDL_PollEvent498(&sdlEvent)) {
		if (sdlEvent.type == SDL_QUIT) {
			signalQuit = true;
			break;
		} else if (sdlEvent.type == SDL_KEYDOWN) {
			InputSystem::processKeyDown(sdlEvent.key.keysym.scancode);
		} else if (sdlEvent.type == SDL_KEYUP) {
			InputSystem::processKeyUp(sdlEvent.key.keysym.scancode);
		} else if (sdlEvent.type == SDL_MOUSEMOTION) {
			updateMousePos(sdlEvent.motion.x, sdlEvent.motion.y);
		} else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN) {
			InputSystem::processMouseButtonDown(sdlEvent.button.button);
		} else if (sdlEvent.type == SDL_MOUSEBUTTONUP) {
			InputSystem::processMouseButtonUp(sdlEvent.button.button);
		} else if (sdlEvent.type == SDL_MOUSEWHEEL) {
			mouseScrollY = sdlEvent.wheel.preciseY;
		}
	}*/
	//if (InputSystem::getKey(SDL_SCANCODE_SPACE) ||
	//	InputSystem::getKey(SDL_SCANCODE_RETURN)) {
	//	perFrameEvents.nextIntro = true;
	//}
	/*if (InputSystem::getKey(SDL_SCANCODE_W) || InputSystem::getKey(SDL_SCANCODE_UP)) {
		playerDir.y -= 1;
	}
	if (InputSystem::getKey(SDL_SCANCODE_S) || InputSystem::getKey(SDL_SCANCODE_DOWN)) {
		playerDir.y += 1;
	}
	if (InputSystem::getKey(SDL_SCANCODE_D) || InputSystem::getKey(SDL_SCANCODE_RIGHT)) {
		playerDir.x += 1;
	}
	if (InputSystem::getKey(SDL_SCANCODE_A) || InputSystem::getKey(SDL_SCANCODE_LEFT)) {
		playerDir.x -= 1;
	}*/
}