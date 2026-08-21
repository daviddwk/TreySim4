#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/timer.hpp>

#include "cutscene.hpp"
#include "duck.hpp"
#include "park.hpp"
#include "textBox.hpp"
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
    Eend::Point duckPosition = Park::get().positionAtTile(Tile(15.0f, 15.0f));
    Duck::get().setPosition(duckPosition);
    Trey::get().setPosition(Park::get().positionAtTile(Tile(13.0f, 15.0f)));
    Eend::Cameras::getScene().setPosition(Eend::Point(45.0f, -75.0f, 15.0f));
    Eend::Cameras::getScene().setTarget(duckPosition);

    TextBoxQueue::get().clear();
    TextBoxQueue::get()
        .queue("duck", Font::daniel, "Let me learn you the way of kick dog.", 0.0f, true);
    TextBoxQueue::get().queue(
        "duck",
        Font::daniel,
        "Use the arrow keys to move and space to kick dog.",
        0.0f,
        true);
    TextBoxQueue::get().queue("duck", Font::daniel, "Good luck!", 0.0f, true);
}

Intro::~Intro() {}

bool Intro::update(float dt) {

    Eend::Point cameraPosition = Eend::Cameras::getScene().getPosition();
    Eend::Cameras::getScene().setPosition(cameraPosition + Eend::Point(dt));
    if (m_timer.update(TextBoxQueue::get().isEmpty())) return false;
    return true;
}
