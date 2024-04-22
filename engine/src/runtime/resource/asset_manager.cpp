#include "runtime/resource/asset_manager.h"
#include "runtime/function/global/global_context.h"


namespace Merak {
	extern const std::string resPath = "resources";

    void AssetManager::checkAssetPath() {
        {
			std::filesystem::path resDir{ resPath };
			if (!std::filesystem::exists(resDir)) {
				std::cout << "error: resources/ missing\n";
				std::cout << "expected: " << std::filesystem::absolute(resDir) << '\n';
				exit(0);
			}
			std::filesystem::path configPath{ resPath + "/game.config" };
			if (!std::filesystem::exists(configPath)) {
				std::cout << "error: resources/game.config missing";
				exit(0);
			}
		}
    }

    void AssetManager::ReadJsonFile(const std::string& path, rapidjson::Document& out_document)  {
        FILE* file_pointer = nullptr;
#ifdef _WIN32
        fopen_s(&file_pointer, path.c_str(), "rb");
#else
        file_pointer = fopen(path.c_str(), "rb");
#endif
        char buffer[65536];
        rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
        out_document.ParseStream(stream);
        std::fclose(file_pointer);

        if (out_document.HasParseError()) {
            std::cout << "error parsing json at [" << path << "]" << std::endl;
            exit(0);
        }
    }

    bool AssetManager::checkJsonKey(rapidjson::Document& doc, const char* mesName, const char* type) {
        if (type == "int")
            return doc.HasMember(mesName) && doc[mesName].IsInt();
        return doc.HasMember(mesName) && doc[mesName].IsString();
    }

	/*void Merak::AssetManager::loadIntroData() {
		if (gameConfig.HasMember("intro_image")) {
			for (auto& name : gameConfig["intro_image"].GetArray()) {
				if (!std::filesystem::exists(resPath + "/images/" + name.GetString() + ".png")) {
					std::cout << "error: missing image " + std::string(name.GetString());
					exit(0);
				}
				introStage.images.emplace_back(IMG_LoadTexture(
					g_context.renderSystem.renderer.get(), (resPath + "/images/" + name.GetString() + ".png").c_str()
				));
			}
		}

		if (gameConfig.HasMember("intro_text")) {
			if (!gameConfig.HasMember("font")) {
				std::cout << "error: text render failed. No font configured";
				exit(0);
			}
		}*/

		/*if (gameConfig.HasMember("font")) {
			auto fontPath = resPath + "/fonts/" + gameConfig["font"].GetString() + ".ttf";
			if (!std::filesystem::exists(fontPath)) {
				std::cout << "error: font " << gameConfig["font"].GetString() << " missing";
				exit(0);
			}
			TTF_Init();
			font = TTF_OpenFont(fontPath.c_str(), 16);
		}*/

		//if (gameConfig.HasMember("intro_text")) {

		//	for (auto& text : gameConfig["intro_text"].GetArray()) {
		//		// renderSystem.introData.texts.emplace_back(text.GetString());
		//		auto surface = TTF_RenderText_Solid(font, text.GetString(), textColor);
		//		assert(surface);
		//		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		//		assetManager->introStage.texts.emplace_back(texture);
		//		SDL_FreeSurface(surface);
		//	}
		//}

	// }

	//TTF_Font* AssetManager::getFont(const std::string& name, int size) {
	//	if (fonts.find(name) == fonts.end()) fonts.insert({ name, std::unordered_map<int, Font_p>() });
	//	else {
	//		auto iter = fonts.at(name).find(size);
	//		if (iter != fonts.at(name).end()) return iter->second.get();
	//	}
	//	auto fontPath = resPath + "/fonts/" + name + ".ttf";
	//	if (!std::filesystem::exists(fontPath)) {
	//		std::cout << "error: font " << name << " missing";
	//		exit(0);
	//	}
	//	fonts.at(name).emplace(size, Font_p(TTF_OpenFont(fontPath.c_str(), size), &TTF_CloseFont));
	//	return fonts.at(name).at(size).get();
	//}

	//Mix_Chunk* AssetManager::getAudio(const std::string& name) {
	//	if (!audioInitialized) {
	//		AudioHelper::Mix_OpenAudio498(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	//		AudioHelper::Mix_AllocateChannels498(50);
	//		audioInitialized = true;
	//	}
	//	auto iter = audios.find(name);
	//	if (iter != audios.end()) return iter->second.get();
	//	auto path = resPath + "/audio/" + name;
	//	if (std::filesystem::exists(path + ".wav")) {
	//		path += ".wav";
	//	} else if (std::filesystem::exists(path + ".ogg")) {
	//		path += ".ogg";
	//	} else {
	//		std::cout << "error: failed to play audio clip " << name;
	//		exit(0);
	//	}
	//	audios.insert( {name, Chunk_p(AudioHelper::Mix_LoadWAV498(path.c_str()), 
	//		[](Mix_Chunk* chunk){
	//			// do nothing here!
	//			// the autograde is not allocating the wav...
	//			// Mix_FreeChunk(chunk);
	//		})} );
	//	return audios.at(name).get();
	//}

	//SDL_Texture* AssetManager::getImage(const std::string& name) {
	//	auto iter = textures.find(name);
	//	if (iter != textures.end()) return iter->second.get();
	//	auto path = resPath + "/images/" + name + ".png";
	//	if (!std::filesystem::exists(path)) {
	//		std::cout << "error: missing image " << name;
	//		exit(0);
	//	}
	//	textures.emplace(name, Texture_p(
	//		IMG_LoadTexture(g_context.renderSystem.renderer.get(), path.c_str()),
	//		SDL_DestroyTexture
	//	));
	//	return textures.at(name).get();
	//}

	void AssetManager::loadGameConfig() {
		checkAssetPath();
		ReadJsonFile(resPath + "/game.config", gameConfig);
		/*if (checkJsonKey(gameConfig, "game_start_message"))
			messages.game_start_message = gameConfig["game_start_message"].GetString();
		if (checkJsonKey(gameConfig, "game_over_bad_message"))
			messages.game_over_bad_message = gameConfig["game_over_bad_message"].GetString();
		if (checkJsonKey(gameConfig, "game_over_good_message"))
			messages.game_over_good_message = gameConfig["game_over_good_message"].GetString();*/
		if (!checkJsonKey(gameConfig, "initial_scene")) {
			std::cout << "error: initial_scene unspecified";
			exit(0);
		}

		if (AssetManager::checkJsonKey(gameConfig, "game_title"))
			gameTitle = gameConfig["game_title"].GetString();
	}

	void AssetManager::startUp() {
		loadGameConfig();
		// TTF_Init();
		// loadAudio();
	}

	void AssetManager::cleanUp() {
		
	}

	AssetManager::~AssetManager() {
		cleanUp();
	}


  //  void AssetManager::loadAudio() {
		//AudioHelper::Mix_OpenAudio498(22050, MIX_DEFAULT_FORMAT, 2, 4096);
		//if (gameConfig.HasMember("intro_bgm")) {
		//	auto bgmName = gameConfig["intro_bgm"].GetString();
		//	auto bgmPath = resPath + "/audio/" + bgmName;
		//	std::string audioPath = "";
		//	if (std::filesystem::exists(bgmPath + ".wav")) {
		//		audioPath = bgmPath + ".wav";
		//	} else if (std::filesystem::exists(bgmPath + ".ogg")) {
		//		audioPath = bgmPath + ".ogg";
		//	} else {
		//		std::cout << "error: failed to play audio clip " << bgmName;
		//		exit(0);
		//	}
		//	introStage.hasAudio = true;
		//	// assert (AudioHelper::Mix_OpenAudio498(44100, MIX_DEFAULT_FORMAT, 2, 2048) == 0);
		//	// introStage.audio = std::make_unique<AudioRenderable>(AudioHelper::Mix_LoadWAV498(audioPath.c_str()));
		//	introStage.audio = AudioHelper::Mix_LoadWAV498(audioPath.c_str());
		//	// assert (renderSystem.introData.sound);
		//	// AudioHelper::Mix_AllocateChannels498(8);
		//}
		//if (gameConfig.HasMember("gameplay_audio")) {
		//	auto bgmName = gameConfig["gameplay_audio"].GetString();
		//	auto bgmPath = resPath + "/audio/" + bgmName;
		//	std::string audioPath = "";
		//	if (std::filesystem::exists(bgmPath + ".wav")) {
		//		audioPath = bgmPath + ".wav";
		//	} else if (std::filesystem::exists(bgmPath + ".ogg")) {
		//		audioPath = bgmPath + ".ogg";
		//	} else {
		//		std::cout << "error: failed to play audio clip " << bgmName;
		//		exit(0);
		//	}
		//	// renderSystem.hasBgm = true;
		//	// renderSystem.bgm = AudioHelper::Mix_LoadWAV498(audioPath.c_str());
		//}

		//if (AssetManager::checkJsonKey(gameConfig, "score_sfx")) {
		//	auto bgmPath = resPath + "/audio/" + gameConfig["score_sfx"].GetString();
		//	std::string audioPath = "";
		//	if (std::filesystem::exists(bgmPath + ".wav")) {
		//		audioPath = bgmPath + ".wav";
		//	} else if (std::filesystem::exists(bgmPath + ".ogg")) {
		//		audioPath = bgmPath + ".ogg";
		//	}
		//}

		//AudioHelper::Mix_AllocateChannels498(50);
  //  }
};