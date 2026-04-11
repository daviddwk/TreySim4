#include <Eendgine/audio.hpp>
#include <Eendgine/camera.hpp>
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/inputManager.hpp>
#include <Eendgine/particles.hpp>
#include <Eendgine/screen.hpp>
#include <Eendgine/shader.hpp>
#include <Eendgine/shaders.hpp>
#include <Eendgine/texture.hpp>
#include <Eendgine/types.hpp>
#include <Eendgine/window.hpp>

#include <cmath>
#include <stb/stb_image.h>

#include <filesystem>

#include "hud.hpp"
#include "park.hpp"
#include "text.hpp"
#include "textBox.hpp"
#include "trey.hpp"

namespace Eend = Eendgine;

static void onStart();
static void pausedUpdate();
static void onDeath();
static void unpausedUpdate();
static void pauseLatch(bool& paused, bool& dead);
static void onRespawn();
static void onPause();
static void onUnpause();
static void onEnd();

const unsigned int screenHeight = 960;
const unsigned int screenWidth = 1280;

int main() {

    Eend::Window::construct(screenWidth, screenHeight, "Quack");
    Eend::Screen::construct(screenWidth, screenHeight);
    Eend::Audio::construct();
    Eend::InputManager::construct();
    Eend::FrameLimiter::construct(60.0f, 20.0f);
    Eend::Entities::construct();
    Eend::Particles::construct();
    Eend::Shaders::construct(
        Eend::ShaderProgram("shaders/panel.vert", "shaders/panel.frag"),
        Eend::ShaderProgram("shaders/board.vert", "shaders/board.frag"),
        Eend::ShaderProgram("shaders/statue.vert", "shaders/statue.frag"),
        Eend::ShaderProgram("shaders/doll.vert", "shaders/doll.frag"),
        Eend::ShaderProgram("shaders/screen.vert", "shaders/screen.frag"));
    Eend::Cameras::construct(
        Eend::Camera2D(screenWidth, screenHeight),
        // starting position for menu
        Eend::Camera3D(
            static_cast<float>(screenWidth) / static_cast<float>(screenHeight),
            Eend::Point(-20.0f, 5.0f, 0.0f),
            Eend::Point(3.0f, 0.0f, 3.0f)));

    bool menu = true;

    // menu init
    Eend::PanelId startButton = Eend::Entities::panels().insert("menu/startButton");
    Eend::Panel* startRef = Eend::Entities::panels().getRef(startButton);
    startRef->setScale(Eend::Scale2D(250.0f, 100.0f));
    startRef->setPosition(Eend::Point(140.0f, 500.0f, 0.0f));
    startRef->setTexture("none");
    Eend::PanelId exitButton = Eend::Entities::panels().insert("menu/exitButton");
    Eend::Panel* exitRef = Eend::Entities::panels().getRef(exitButton);
    exitRef->setScale(Eend::Scale2D(250.0f, 100.0f));
    exitRef->setPosition(Eend::Point(890.0f, 500.0f, 0.0f));
    exitRef->setTexture("none");
    //  menu init
    while (menu && !Eend::InputManager::get().getShouldClose()) { // exit menu
        bool start = false;
        Eend::FrameLimiter::get().startInterval();
        Eend::Screen::get().bind();
        Eend::InputManager::get().processInput();
        Eend::Shaders::get().getShader(Eend::Shader::screen).setInt("pixelSize", 5);

        // menu update
        Eend::Panel* startRef = Eend::Entities::panels().getRef(startButton);
        auto startButtonStatus = startRef->isClicked();
        if (startButtonStatus == Eend::Panel::MouseStatus::click) {
            std::print("start\n");
            start = true;
        } else if (startButtonStatus == Eend::Panel::MouseStatus::hover) {
            startRef->setTexture("hover");
        } else {
            startRef->setTexture("none");
        }
        Eend::Panel* exitRef = Eend::Entities::panels().getRef(exitButton);
        auto exitButtonStatus = exitRef->isClicked();
        if (exitButtonStatus == Eend::Panel::MouseStatus::click) {
            std::print("stop\n");
            menu = false;
        } else if (exitButtonStatus == Eend::Panel::MouseStatus::hover) {
            exitRef->setTexture("hover");
        } else {
            exitRef->setTexture("none");
        }
        // menu update

        Eend::Entities::draw(Eend::Cameras::getHud(), Eend::Cameras::getScene());
        Eend::Screen::get().render();
        Eend::Window::get().swapBuffers();
        Eend::FrameLimiter::get().stopInterval();

        if (start) {
            // menu destroy
            Eend::Entities::panels().erase(startButton);
            Eend::Entities::panels().erase(exitButton);
            // menu destroy
            // could wrap in a loading screen if it was slow enough
            Trey::construct();
            // TODO build the scale into the format silly
            Park::construct("terrain/grassy");
            Hud::construct();
            TextBoxQueue::construct();

            onStart();
            while (!Eend::InputManager::get().getShouldClose()) {
                static bool paused = false;
                static bool dead = false;

                Eend::FrameLimiter::get().startInterval();
                Eend::Screen::get().bind();
                Eend::InputManager::get().processInput();
                Eend::Shaders::get().getShader(Eend::Shader::screen).setInt("pixelSize", 5);

                Hud::get().update();

                pauseLatch(paused, dead);
                if (paused) {
                    pausedUpdate();
                } else {
                    unpausedUpdate();
                }

                Eend::Entities::draw(Eend::Cameras::getHud(), Eend::Cameras::getScene());
                Eend::Screen::get().render();
                Eend::Window::get().swapBuffers();
                Eend::FrameLimiter::get().stopInterval();
            }
            Eend::InputManager::get().setShouldClose(false);

            onEnd();

            TextBoxQueue::destruct();
            Hud::destruct();
            Trey::destruct();
            Park::destruct();
            // menu init
            startButton = Eend::Entities::panels().insert("menu/startButton");
            startRef = Eend::Entities::panels().getRef(startButton);
            startRef->setScale(Eend::Scale2D(250.0f, 100.0f));
            startRef->setPosition(Eend::Point(140.0f, 500.0f, 0.0f));
            startRef->setTexture("none");
            exitButton = Eend::Entities::panels().insert("menu/exitButton");
            exitRef = Eend::Entities::panels().getRef(exitButton);
            exitRef->setScale(Eend::Scale2D(250.0f, 100.0f));
            exitRef->setPosition(Eend::Point(890.0f, 500.0f, 0.0f));
            exitRef->setTexture("none");
            //  menu init
        }
    }
    // menu destroy
    Eend::Entities::panels().erase(startButton);
    Eend::Entities::panels().erase(exitButton);
    // menu destroy

    Eend::Shaders::destruct();
    Eend::Particles::destruct();
    Eend::Entities::destruct();
    Eend::Audio::destruct();
    Eend::Screen::destruct();
    Eend::Window::destruct();
    Eend::FrameLimiter::destruct();
    return 0;
}

static void onStart() {

    Trey::get().setPosition(Park::get().getSpawn());

    TextBoxQueue::get().queue("duck", Font::daniel, "Help meeeee!", 3.0f, true);
    TextBoxQueue::get().queue("dog", Font::daniel, "It's over for you bucko.", 3.0f, false);
    TextBoxQueue::get().queue(
        "duck",
        Font::daniel,
        "What the duck did you just call me? You little quack! "
        "Aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahhhhhhhhhhhh",
        5.0f,
        true);

    // Eend::Audio::get().playTrack(
    //     "resources/music/829534__josefpres__piano-loops-192-octave-long-loop-120-bpm.wav",
    //     50.0f);
}

static void pauseLatch(bool& paused, bool& dead) {

    static bool escapeReleased = false;

    if (!dead && (Trey::get().health.getHealth() == 0)) {
        dead = true;
        onDeath();
    }

    if (paused || dead) {
        bool escapePressed = Eend::InputManager::get().getEscapePress();
        if (!escapePressed) escapeReleased = true;
        if (escapeReleased && escapePressed) {
            escapeReleased = false;
            if (paused) {
                paused = false;
                onUnpause();
            }
            if (dead) {
                dead = false;
                onRespawn();
            }
        }
    } else {
        bool escapePressed = Eend::InputManager::get().getEscapePress();
        if (!escapePressed) escapeReleased = true;
        if (escapeReleased && escapePressed) {
            paused = true;
            escapeReleased = false;
            onPause();
        }
    }
}
static void onDeath() {
    Hud::get().setDeathText(true);
    Trey::get().setAlive(false);
}

static void onRespawn() {
    Hud::get().setDeathText(false);
    Park::get().reset();
    Trey::get().health.heal(100);
    Trey::get().setPosition(Park::get().getSpawn());
    Trey::get().setAlive(true);
    Eendgine::Entities::shrink();
}

static void onUnpause() {
    // Park::get().setTerrain("terrain/grassy"); // DEBUG
}

static void onPause() {
    Park::get().nextWave(); // DEBUG
}

static void pausedUpdate() {}

static void unpausedUpdate() {
    float dt = Eend::FrameLimiter::get().deltaTime;

    Trey::get().update();

    Eend::Point treyPosition = Trey::get().getPosition();
    float terrainHeight = Park::get().heightAtPoint(Eend::Point2D(treyPosition.x, treyPosition.y));

    static Eend::Point lastCameraPosition =
        Eend::Point(treyPosition.x, treyPosition.y - 25.0f, terrainHeight + 12.5f);
    Eend::Point approachCameraPosition =
        Eend::Point(treyPosition.x, treyPosition.y - 25.0f, terrainHeight + 12.5f);
    float cameraLag = (10.0f * dt);
    lastCameraPosition =
        (lastCameraPosition + (approachCameraPosition * cameraLag)) / (cameraLag + 1.0f);
    Eend::Cameras::getScene().setPosition(lastCameraPosition);
    Eend::Cameras::getScene().setTarget(
        Eend::Point(treyPosition.x, treyPosition.y, terrainHeight + 3.0f));

    TextBoxQueue::get().update();
    Eend::Particles::get().update();
    // Eend::Entities::dolls().getRef(testDollId)->setAnim(testAnimScale);
    Park::get().update();
}

static void onEnd() {}
