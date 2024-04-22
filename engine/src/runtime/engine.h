#pragma once

namespace Merak {

	class MerakEngine {
	private:

		void logicTick();
		void renderTick();

	public:

		struct Event {
			bool nextIntro = true;
			// bool playerInfoUpdated = false;
			void reset() {
				nextIntro = false;
			}
		} perFrameEvents;

		bool shouldContinue();
		void gameStart();

		void tickOneFrame();

		void beginOfFrame();
		void endOfFrame();

		void gameEnd();
	};

};