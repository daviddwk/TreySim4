#include "cutscene.hpp"

// TODO use templating with cutscene class type??
void CutscenePlayer::start(CutsceneType cutscene) {
    switch (cutscene) {
    case CutsceneType::testOne:
        activeCutscene = std::make_unique<TestOne>();
        break;
    }
}

bool CutscenePlayer::update() {
    if (activeCutscene) {
        bool endCutscene = !(*activeCutscene)->update();
        if (endCutscene) {
            activeCutscene = std::nullopt;
            return false;
        }
        return true;
    }
    return false;
}
