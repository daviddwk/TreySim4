#include <Eendgine/frameLimiter.hpp>

#include "cutscene.hpp"

// TODO use templating with cutscene class type??
void CutscenePlayer::start(CutsceneType cutscene) { nextCutscene = cutscene; }

bool CutscenePlayer::update() {
    if (activeCutscene) {
        bool endCutscene = !(*activeCutscene)->update(Eend::FrameLimiter::get().deltaTime);
        if (endCutscene) {
            activeCutscene = std::nullopt;
            return false;
        }
        return true;
    } else if (nextCutscene) {
        switch (*nextCutscene) {
        case CutsceneType::testOne:
            activeCutscene = std::make_unique<TestOne>();
            break;
        }
        nextCutscene = std::nullopt;
        return true;
    }
    return false;
}
