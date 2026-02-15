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
#include "hud.hpp"
#include "park.hpp"
#include "text.hpp"
#include "textBox.hpp"

namespace Eend = Eendgine;

static void onStart();
static void pausedUpdate();
static void onDeath();
static void unpausedUpdate(float dt, Eend::Camera3D& sceneCamera);
static void pauseLatch(bool& paused, bool& dead);
static void onRespawn();
static void onPause();
static void onUnpause();
static void onEnd();

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
    Eend::Shaders::construct(
        Eend::ShaderProgram("shaders/panel.vert", "shaders/panel.frag"),
        Eend::ShaderProgram("shaders/board.vert", "shaders/board.frag"),
        Eend::ShaderProgram("shaders/statue.vert", "shaders/statue.frag"),
        Eend::ShaderProgram("shaders/doll.vert", "shaders/doll.frag"),
        Eend::ShaderProgram("shaders/screen.vert", "shaders/screen.frag"));

    // TODO make comeras their own thing
    Eend::Camera2D hudCamera(screenWidth, screenHeight);
    Eend::Camera3D sceneCamera(
        (float)screenWidth / (float)screenHeight,
        Eend::Point(-20.0f, 5.0f, 0.0f),
        Eend::Point(3.0f, 0.0f, 3.0f));

    Duck::construct();
    Park::construct("terrain/grassy", Eend::Scale(3.0f, 3.0f, 20.0f));
    Hud::construct();
    TextBoxQueue::construct();

    onStart();

    while (!Eend::InputManager::get().getShouldClose()) {
        static bool paused = false;
        static bool dead = false;
        static float dt = 0.0;

        Eend::FrameLimiter::get().startInterval();
        Eend::Screen::get().bind();
        Eend::InputManager::get().processInput();
        Eend::Shaders::get().getShader(Eend::Shader::screen).setInt("pixelSize", 5);

        dt = Eend::FrameLimiter::get().deltaTime;

        Hud::get().update(dt);

        pauseLatch(paused, dead);
        if (paused) {
            pausedUpdate();
        } else {
            unpausedUpdate(dt, sceneCamera);
        }

        Eend::Entities::draw(hudCamera, sceneCamera);
        Eend::Screen::get().render();
        Eend::Window::get().swapBuffers();
        Eend::FrameLimiter::get().stopInterval();
    }

    onEnd();

    TextBoxQueue::destruct();
    Hud::destruct();
    Duck::destruct();
    Park::destruct();

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

    Duck::get().setPosition(Park::get().getSpawn());

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
}

static void pauseLatch(bool& paused, bool& dead) {

    static bool escapeReleased = false;

    if (!dead && (Duck::get().health.getHealth() == 0)) {
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
    Duck::get().setAlive(false);
}

static void onRespawn() {
    Hud::get().setDeathText(false);
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

static void onEnd() {}
