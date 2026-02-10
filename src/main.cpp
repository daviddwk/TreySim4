#include <Eendgine/audio.hpp>
#include <Eendgine/camera.hpp>
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/inputManager.hpp>
#include <Eendgine/particles.hpp>
#include <Eendgine/screen.hpp>
#include <Eendgine/shader.hpp>
#include <Eendgine/shaders.hpp>
#include <Eendgine/textureCache.hpp>
#include <Eendgine/types.hpp>
#include <Eendgine/window.hpp>

#include <cmath>
#include <stb/stb_image.h>

#include <filesystem>

#include "duck.hpp"
#include "park.hpp"
#include "text.hpp"
#include "textBox.hpp"

namespace Eend = Eendgine;

static void pausedUpdate();
static void onDeath(Text& deathText);
static void unpausedUpdate(float dt, Eend::Camera3D& sceneCamera);
static void pauseLatch(bool& paused, bool& dead, Text& deathText);
static void onRespawn(Text& deathText);
static void onPause();
static void onUnpause();

const unsigned int screenHeight = 1080;
const unsigned int screenWidth = 1920;

int main() {

    Eend::Window::construct(screenWidth, screenHeight, "Quack");
    Eend::Screen::construct(screenWidth, screenHeight);
    Eend::Audio::construct();
    Eend::InputManager::construct();
    Eend::FrameLimiter::construct(60.0f, 20.0f);
    Eend::Entities::construct();
    Eend::Particles::construct();

    Eend::Shaders shaders(
        Eend::ShaderProgram("shaders/panel.vert", "shaders/panel.frag"),
        Eend::ShaderProgram("shaders/board.vert", "shaders/board.frag"),
        Eend::ShaderProgram("shaders/statue.vert", "shaders/statue.frag"),
        Eend::ShaderProgram("shaders/doll.vert", "shaders/doll.frag"),
        Eend::ShaderProgram("shaders/screen.vert", "shaders/screen.frag"));

    Eend::Camera2D hudCamera(screenWidth, screenHeight);
    Eend::Camera3D sceneCamera(
        (float)screenWidth / (float)screenHeight,
        Eend::Point(-20.0f, 5.0f, 0.0f),
        Eend::Point(3.0f, 0.0f, 3.0f));

    TextBoxQueue::construct();
    Duck::construct();
    Park::construct("terrain/grassy", Eend::Scale(3.0f, 3.0f, 20.0f));

    bool paused = false;
    bool dead = false;
    // Weird hack to get terrain destructor to run before Entity Batches are deleted
    // I need to create a level system that handles terrain
    {
        Duck& duck = Duck::get();

        duck.setPosition(Park::get().getSpawn());

        // float testAnimScale = 0.0f;
        // Eend::DollId testDollId = Eend::Entities::dolls().insert("testCube");

        Text testText(Font::daniel, "", Eend::Point(20.0f), 50.0f, INFINITY);
        Text deathText(Font::daniel, "", Eend::Point(500.0f, 300.0f, 0.0f), 200.0f, INFINITY);

        bool testColliding = false;

        Eend::PanelId exitId = Eend::Entities::panels().insert("exit");
        Eend::Panel* exitRef = Eend::Entities::panels().getRef(exitId);
        exitRef->setScale(Eend::Scale2D(50.0f, 50.0f));
        exitRef->setPosition(Eend::Point((float)screenWidth - 80.0f, 30.0f, 0.0f));

        TextBoxQueue::get().queue("duck", Font::daniel, "Help meeeee!", 3.0f, true);
        TextBoxQueue::get().queue("dog", Font::daniel, "It's over for you bucko.", 3.0f, false);
        TextBoxQueue::get().queue(
            "duck",
            Font::daniel,
            "What the duck did you just call me? You little quack! "
            "Aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahhhhhhhhhhhh",
            5.0f,
            true);

        Eend::Audio::get().playTrack(
            "resources/music/829534__josefpres__piano-loops-192-octave-long-loop-120-bpm.wav",
            50.0f);

        while (!Eend::InputManager::get().getShouldClose()) {
            float dt = Eend::FrameLimiter::get().deltaTime;
            Eend::FrameLimiter::get().startInterval();
            Eend::Screen::get().bind();
            Eend::InputManager::get().processInput();
            shaders.setPixelSize(5);

            Eend::Panel::MouseStatus exitMouseStatus =
                Eend::Entities::panels().getRef(exitId)->isClicked();
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
            // clang-format off
        testText.setText(
            std::format(
                "FPS:{:.4f} DT:{:.4f}\n"
                "X:{:.4f} Y:{:.4f} Z:{:.4f}\n"
                "duck:{} mouse:{} \n"
                "mouseX:{} dx:{} mouseY:{} dy:{}\n"
                "left:{} right:{} mid:{}\n"
                "dogs slain:{}",
                1.0f / dt, dt,
                duck.getPosition().x, duck.getPosition().y, duck.getPosition().z,
                testColliding, exitMouseString,
                Eend::InputManager::get().getMouseX(), Eend::InputManager::get().getDeltaMouseX(),
                Eend::InputManager::get().getMouseY(), Eend::InputManager::get().getDeltaMouseY(),
                Eend::InputManager::get().getLeftClick(),
                Eend::InputManager::get().getRightClick(),
                Eend::InputManager::get().getMiddleClick(),
                Park::get().numDogsKilled()));
            // clang-format on

            pauseLatch(paused, dead, deathText);
            if (paused) {
                pausedUpdate();
            } else {
                unpausedUpdate(dt, sceneCamera);
            }

            Eend::Entities::draw(shaders, hudCamera, sceneCamera);
            Eend::Screen::get().render(shaders.getShader(Eend::Shader::screen));

            Eend::Window::get().swapBuffers();
            Eend::FrameLimiter::get().stopInterval();
        }
        Eend::Entities::panels().erase(exitId);
    } // Terrain
    TextBoxQueue::destruct();
    Duck::destruct();
    Park::destruct();

    Eend::Particles::destruct();
    Eend::Entities::destruct();
    Eend::Audio::destruct();
    Eend::Screen::destruct();
    Eend::Window::destruct();
    Eend::FrameLimiter::destruct();
    return 0;
}

static void pauseLatch(bool& paused, bool& dead, Text& deathText) {

    static bool escapeReleased = false;

    if (!dead && (Duck::get().health.getHealth() == 0)) {
        dead = true;
        onDeath(deathText);
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
            // destroy menu
            // every frame death stuff
            if (dead) {
                dead = false;
                onRespawn(deathText);
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
static void onDeath(Text& deathText) {
    deathText.setText("YOU DIED!\nPRESS ESC\n");
    Duck::get().setAlive(false);
}

static void onRespawn(Text& deathText) {
    deathText.setText("");
    Park::get().reset();
    Duck::get().health.heal(100);
    Duck::get().setPosition(Park::get().getSpawn());
    Duck::get().setAlive(true);
    Eendgine::Entities::shrink();
}

static void onUnpause() { Park::get().setTerrain("terrain/test", Eend::Scale(3.0)); }

static void onPause() {}

static void pausedUpdate() {}

static void unpausedUpdate(float dt, Eend::Camera3D& sceneCamera) {
    Duck::get().update(dt);

    Eend::Point duckPosition = Duck::get().getPosition();
    float terrainHeight = Park::get().heightAtPoint(Eend::Point2D(duckPosition.x, duckPosition.y));

    static Eend::Point lastCameraPosition =
        Eend::Point(duckPosition.x, duckPosition.y - 25.0f, terrainHeight + 12.5f);
    Eend::Point approachCameraPosition =
        Eend::Point(duckPosition.x, duckPosition.y - 25.0f, terrainHeight + 12.5f);
    float cameraLag = (10.0f * dt);
    lastCameraPosition =
        (lastCameraPosition + (approachCameraPosition * cameraLag)) / (cameraLag + 1.0f);
    sceneCamera.setPosition(lastCameraPosition);
    sceneCamera.setTarget(Eend::Point(duckPosition.x, duckPosition.y, terrainHeight + 3.0f));

    TextBoxQueue::get().update();
    Eend::Particles::get().update(dt);
    // Eend::Entities::dolls().getRef(testDollId)->setAnim(testAnimScale);
    Park::get().update(dt);
}
