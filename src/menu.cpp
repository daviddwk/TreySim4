#include "menu.hpp"

void Menu::construct() {
    assert(_instance == nullptr);
    _instance = new Menu;
}

void Menu::destruct() {
    assert(_instance != nullptr);
    delete _instance;
    _instance = nullptr;
}

Menu& Menu::get() {
    assert(_instance != nullptr);
    return *_instance;
}

Menu::Menu()
    : m_startButton(Eend::Entities::panels().insert("resources/menu/startButton")),
      m_exitButton(Eend::Entities::panels().insert("resources/menu/exitButton")) {

    Eend::Panel* startRef = Eend::Entities::panels().getRef(m_startButton);
    startRef->setScale(Eend::Scale2D(250.0f, 100.0f));
    startRef->setPosition(Eend::Point(140.0f, 500.0f, 0.0f));
    startRef->setTexture("none");

    Eend::Panel* exitRef = Eend::Entities::panels().getRef(m_exitButton);
    exitRef->setScale(Eend::Scale2D(250.0f, 100.0f));
    exitRef->setPosition(Eend::Point(890.0f, 500.0f, 0.0f));
    exitRef->setTexture("none");
}

Menu::~Menu() {
    Eend::Entities::panels().erase(m_startButton);
    Eend::Entities::panels().erase(m_exitButton);
}

void Menu::update(bool& start, bool& menu) {
    Eend::Panel* startRef = Eend::Entities::panels().getRef(m_startButton);
    auto startButtonStatus = startRef->isClicked();
    if (startButtonStatus == Eend::Panel::MouseStatus::click) {
        start = true;
    } else if (startButtonStatus == Eend::Panel::MouseStatus::hover) {
        startRef->setTexture("hover");
    } else {
        startRef->setTexture("none");
    }
    Eend::Panel* exitRef = Eend::Entities::panels().getRef(m_exitButton);
    auto exitButtonStatus = exitRef->isClicked();
    if (exitButtonStatus == Eend::Panel::MouseStatus::click) {
        menu = false;
    } else if (exitButtonStatus == Eend::Panel::MouseStatus::hover) {
        exitRef->setTexture("hover");
    } else {
        exitRef->setTexture("none");
    }
}
