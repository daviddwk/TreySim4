#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/timer.hpp>

#include "cutscene.hpp"
#include "duck.hpp"
#include "park.hpp"
#include "trey.hpp"

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
        case CutsceneType::intro:
            activeCutscene = std::make_unique<Intro>();
            break;
        }
        nextCutscene = std::nullopt;
        return true;
    }
    return false;
}

Intro::Intro() : m_timer(3.0f) {
    std::print("position set\n");
    Duck::get().setPosition(Park::get().positionAtTile(Tile(15.0f, 15.0f)));
    Trey::get().setPosition(Duck::get().getPosition());
}

Intro::~Intro() {}

bool Intro::update(float dt) {

    Eend::Point cameraPosition = Eend::Cameras::getScene().getPosition();
    Eend::Cameras::getScene().setPosition(cameraPosition + Eend::Point(dt));
    if (m_timer.update(dt)) return false;
    return true;
}
