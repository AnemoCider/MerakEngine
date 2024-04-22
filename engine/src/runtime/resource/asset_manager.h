#pragma once

//#include "SDL2/SDL.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <filesystem>
#define RAPIDJSON_NOMEMBERITERATORCLASS
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <vector>
//#include "course/AudioHelper.h"
#include <optional>
//#include "SDL2/SDL_ttf.h"

namespace Merak {
	inline std::string obtain_word_after_phrase(const std::string& input, const std::string& target) {
		size_t pos = input.find(target);
		if (pos == std::string::npos) return "";
		pos += target.size();
		while (pos < input.size() && std::isspace(input[pos]))
			pos++;
		if (pos == input.size()) return "";
		size_t endPos = pos;
		while (endPos < input.size() && !std::isspace(input[endPos]))
			endPos++;
		return input.substr(pos, endPos - pos);
	}

	extern const std::string resPath;

	class AssetManager {

	public:

		// whether we have called Mix_OpenAudio
		// load font, on the contrary, has been initialized during initialization
		bool audioInitialized = false;
		/*
			@brief check whether resources/game.config presents
		*/
		static void checkAssetPath();

		static void ReadJsonFile(const std::string& path, rapidjson::Document& out_document);

		static bool checkJsonKey(rapidjson::Document& doc, const char* mesName, const char* type = "string");

		std::string gameTitle = "";

		rapidjson::Document gameConfig;

		void startUp();

		void cleanUp();

		~AssetManager();

		// void loadIntroData();
		void loadGameConfig();
		// void loadAudio();

		// get font from fontname + size
		// load if not already
		/*TTF_Font* getFont(const std::string&, int);

		Mix_Chunk* getAudio(const std::string&);

		SDL_Texture* getImage(const std::string&);*/
	};

};
