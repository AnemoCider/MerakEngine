#include "runtime/engine.h"
//#include "course/Helper.h"
#include <memory>

int main(int argc, char* argv[]) {
	auto engine = std::make_unique<Merak::MerakEngine>();
	engine->gameStart();
	while (engine->shouldContinue()) {
		engine->tickOneFrame();
	}
	engine->gameEnd();
}