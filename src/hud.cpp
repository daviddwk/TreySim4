#include "hud.hpp"

#include "duck.hpp"
#include "park.hpp"

#include <Eendgine/entityBatches.hpp>
#include <Eendgine/inputManager.hpp>
#include <Eendgine/types.hpp>

#include <assert.h>

void Hud::construct() {
    assert(m_instance == nullptr);
    m_instance = new Hud;
}

void Hud::destruct() {
    assert(m_instance != nullptr);
    delete m_instance;
    m_instance = nullptr;
}

Hud& Hud::get() {
    assert(m_instance != nullptr);
    return *m_instance;
}

Hud::Hud()
    : m_deathText(Font::daniel, "", Eend::Point(500.0f, 300.0f, 0.0f), 200.0f, INFINITY),
      m_testText(Font::daniel, "", Eend::Point(20.0f), 50.0f, INFINITY),
      m_exitPanel(Eend::Entities::panels().insert("exit")) {

    Eend::Panel* exitRef = Eend::Entities::panels().getRef(m_exitPanel);
    exitRef->setScale(Eend::Scale2D(50.0f, 50.0f));
    exitRef->setPosition(Eend::Point(1920.0f - 80.0f, 30.0f, 0.0f));
}

Hud::~Hud() { Eend::Entities::panels().erase(m_exitPanel); }

void Hud::setDeathText(bool set) {
    if (set) {
        m_deathText.setText("YOU DIED!\nPRESS ESC\n");
    } else {
        m_deathText.setText("");
    }
}

void Hud::update(float dt) {
    Eend::Point duckPosition = Duck::get().getPosition();

    Eend::Panel::MouseStatus exitMouseStatus =
        Eend::Entities::panels().getRef(m_exitPanel)->isClicked();
    std::string exitMouseString = "";
    if (exitMouseStatus == Eend::Panel::MouseStatus::click) {
        exitMouseString = "click";
    } else if (exitMouseStatus == Eend::Panel::MouseStatus::hover) {
        exitMouseString = "hover";
    } else if (exitMouseStatus == Eend::Panel::MouseStatus::none) {
        exitMouseString = "none";
    }

    if (exitMouseStatus == Eend::Panel::MouseStatus::click) {
        Eend::InputManager::get().setShouldClose(true);
    }

    m_testText.setText(
        std::format(
            "FPS:{:.4f} DT:{:.4f}\n"
            "X:{:.4f} Y:{:.4f} Z:{:.4f}\n"
            "duck:{} mouse:{} \n"
            "mouseX:{} dx:{} mouseY:{} dy:{}\n"
            "left:{} right:{} mid:{}\n"
            "dogs slain:{}",
            1.0f / dt,
            dt,
            duckPosition.x,
            duckPosition.y,
            duckPosition.z,
            true,
            exitMouseString,
            Eend::InputManager::get().getMouseX(),
            Eend::InputManager::get().getDeltaMouseX(),
            Eend::InputManager::get().getMouseY(),
            Eend::InputManager::get().getDeltaMouseY(),
            Eend::InputManager::get().getLeftClick(),
            Eend::InputManager::get().getRightClick(),
            Eend::InputManager::get().getMiddleClick(),
            Park::get().numDogsKilled()));
}
