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

#include <stb/stb_image.h>

#include <filesystem>

#include "cutscene.hpp"
#include "duck.hpp"
#include "hud.hpp"
#include "menu.hpp"
#include "park.hpp"
#include "text.hpp"
#include "textBox.hpp"
#include "trey.hpp"

namespace Eend = Eendgine;

static void startFrame();
static void endFrame();
static void pauseLatch(bool& paused, bool& dead);
static void onStart();
static void pausedUpdate();
static void onDeath();
static void unpausedUpdate();
static void cutsceneUpdate();
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
        Eend::ShaderProgram(Eend::SHADER_PATH_PANEL_VERT, Eend::SHADER_PATH_PANEL_FRAG),
        Eend::ShaderProgram(Eend::SHADER_PATH_BOARD_VERT, Eend::SHADER_PATH_BOARD_FRAG),
        Eend::ShaderProgram(Eend::SHADER_PATH_STATUE_VERT, Eend::SHADER_PATH_STATUE_FRAG),
        Eend::ShaderProgram(Eend::SHADER_PATH_DOLL_VERT, Eend::SHADER_PATH_DOLL_FRAG),
        Eend::ShaderProgram(Eend::SHADER_PATH_SCREEN_VERT, Eend::SHADER_PATH_SCREEN_FRAG));
    Eend::Cameras::construct(
        Eend::Camera2D(screenWidth, screenHeight),
        // starting position for menu
        Eend::Camera3D(
            static_cast<float>(screenWidth) / static_cast<float>(screenHeight),
            Eend::Point(-20.0f, 5.0f, 0.0f),
            Eend::Point(3.0f, 0.0f, 3.0f)));

    bool menu = true;

    Menu::construct();
    while (menu && !Eend::InputManager::get().getShouldClose()) { // exit menu
        bool start = false;

        startFrame();

        Menu::get().update(start, menu);

        endFrame();

        if (start) {
            Menu::destruct();
            // could wrap in a loading screen if it was slow enough
            Trey::construct();
            Duck::construct();
            Park::construct("grassy");
            Hud::construct();
            TextBoxQueue::construct();

            onStart();
            while (!Eend::InputManager::get().getShouldClose()) {
                static bool paused = false;
                static bool dead = false;

                startFrame();

                Hud::get().update();

                if (!CutscenePlayer::update()) {
                    pauseLatch(paused, dead);
                    if (paused) {
                        pausedUpdate();
                    } else {
                        unpausedUpdate();
                    }
                } else {
                    cutsceneUpdate();
                }

                endFrame();
            }
            Eend::InputManager::get().setShouldClose(false);

            onEnd();

            TextBoxQueue::destruct();
            Hud::destruct();
            Trey::destruct();
            Duck::destruct();
            Park::destruct();

            Menu::construct();
        }
    }
    Menu::destruct();

    Eend::Shaders::destruct();
    Eend::Particles::destruct();
    Eend::Entities::destruct();
    Eend::Audio::destruct();
    Eend::Screen::destruct();
    Eend::Window::destruct();
    Eend::FrameLimiter::destruct();
    return 0;
}

static void startFrame() {
    Eend::FrameLimiter::get().startInterval();
    Eend::Screen::get().bind();
    Eend::InputManager::get().processInput();
    Eend::Shaders::get()
        .getShader(Eend::Shader::screen)
        .setInt(Eend::SHADER_SCREEN_INT_PIXEL_SIZE, 5);
}

static void endFrame() {
    Eend::Entities::draw(Eend::Cameras::getHud(), Eend::Cameras::getScene());
    Eend::Screen::get().render();
    Eend::Window::get().swapBuffers();
    Eend::FrameLimiter::get().stopInterval();
}

static void pauseLatch(bool& paused, bool& dead) {

    static bool escapeReleased = false;

    if (!dead && (Trey::get().health.getHealth() == 0)) {
        dead = true;
        onDeath();
    }

    if (paused || dead) {
        bool escapePressed = Eend::InputManager::get().isKeyPressed(SDL_SCANCODE_ESCAPE);
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
        bool escapePressed = Eend::InputManager::get().isKeyPressed(SDL_SCANCODE_ESCAPE);
        if (!escapePressed) escapeReleased = true;
        if (escapeReleased && escapePressed) {
            paused = true;
            escapeReleased = false;
            onPause();
        }
    }
}

static void onStart() {

    Trey::get().setPosition(Park::get().getSpawn());
    Duck::get().setPosition(Park::get().positionAtTile(Tile(15.0f, 15.0f)));

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
    Park::get().playgroundEnable("tree1"); // DEBUG
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
    Park::get().playgroundToggle("tree1"); // DEBUG
}

static void onPause() {}

static void pausedUpdate() {}

static void unpausedUpdate() {
    float dt = Eend::FrameLimiter::get().deltaTime;

    Trey::get().update();
    Duck::get().update();

    float duckDistance = glm::distance(Trey::get().getPosition2D(), Duck::get().getPosition2D());
    float interactDistance = 5.0f;
    if ((duckDistance < interactDistance) && Eend::InputManager::get().onKeyUp(SDL_SCANCODE_E)) {
        // TextBoxQueue::get().clear();
        // TextBoxQueue::get().queue("duck", Font::daniel, "Hey man.", 3.0f, true);

        CutscenePlayer::start(CutsceneType::testOne);
    }

    if (Eend::InputManager::get().onKeyDown(SDL_SCANCODE_W)) Park::get().nextWave(); // DEBUG

    Eend::Point treyPosition = Trey::get().getPosition();
    float terrainHeight = Park::get().elevationAtPoint(treyPosition);

    // const Eend::Point cameraOffset = Eend::Point(0.0f, -25.0f, 12.5f);
    const Eend::Point cameraOffset = Eend::Point(0.0f, -1.0f, 50.0f); // DEBUG
    // const float cameraLag = (10.0f * dt);
    const float cameraLag = (10000.0f * dt); // DEBUG

    Eend::Point lastCameraPosition = Eend::Cameras::getScene().getPosition();
    Eend::Point approachCameraPosition =
        Eend::Point(treyPosition.x, treyPosition.y, terrainHeight) + cameraOffset;
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

static void cutsceneUpdate() {
    TextBoxQueue::get().update();
    Eend::Particles::get().update();
    // Park::get().update();
}

static void onEnd() {}
