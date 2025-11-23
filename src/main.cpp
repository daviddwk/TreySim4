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
#include <print>

#include "duck.hpp"
#include "puppyMill.hpp"
#include "terrain.hpp"
#include "text.hpp"
#include "textBox.hpp"

namespace Eend = Eendgine;

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

    bool escapeReleased = false;
    bool paused = false;
    bool dead = false;

    Terrain testTerrain("terrain/grassy", Eend::Scale(3.0f, 3.0f, 20.0f));

    Duck duck = Duck();
    PuppyMill puppyMill(&testTerrain);

    duck.setPosition(testTerrain.positionAtTile(20.0f, 20.0f, 0.0f));

    float testAnimScale = 0.0f;
    Eend::DollId testDollId = Eend::Entities::dolls().insert("testCube");

    Text testText(Font::DANIEL, "", Eend::Point(20.0f), 50.0f, INFINITY);
    Text deathText(Font::DANIEL, "", Eend::Point(500.0f, 300.0f, 0.0f), 200.0f, INFINITY);

    bool testColliding = false;

    Eend::PanelId exitId = Eend::Entities::panels().insert("exit");
    Eend::Panel* exitRef = Eend::Entities::panels().getRef(exitId);
    exitRef->setScale(Eend::Scale2D(50.0f, 50.0f));
    exitRef->setPosition(Eend::Point((float)screenWidth - 80.0f, 30.0f, 0.0f));

    TextBoxQueue::get().queue("duck", Font::DANIEL, "Help meeeee!", 3.0f, true);
    TextBoxQueue::get().queue("dog", Font::DANIEL, "It's over for you bucko.", 3.0f, false);
    TextBoxQueue::get().queue(
        "duck",
        Font::DANIEL,
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
        shaders.setPixelSize(5);

        Eend::Panel::MouseStatus exitMouseStatus =
            Eend::Entities::panels().getRef(exitId)->isClicked();
        std::string exitMouseString = "";
        if (exitMouseStatus == Eend::Panel::MouseStatus::CLICK) {
            exitMouseString = "click";
        } else if (exitMouseStatus == Eend::Panel::MouseStatus::HOVER) {
            exitMouseString = "hover";
        } else if (exitMouseStatus == Eend::Panel::MouseStatus::NONE) {
            exitMouseString = "none";
        }

        if (exitMouseStatus == Eend::Panel::MouseStatus::CLICK) {
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
                puppyMill.getNumKilled()));
        // clang-format on

        Eend::InputManager::get().processInput();

        // run once on death
        if (!dead && (duck.health.getHealth() == 0)) {
            dead = true;
            deathText.setText("YOU DIED!\nPRESS ESC\n");
        }

        // pause latch and menu setup / teardown
        if (paused || dead) {
            bool escapePressed = Eend::InputManager::get().getEscapePress();
            if (!escapePressed) escapeReleased = true;
            if (escapeReleased && escapePressed) {
                escapeReleased = false;
                if (paused) {
                    paused = false;
                }
                // destroy menu
                // every frame death stuff
                if (dead) {
                    dead = false;
                    deathText.setText("");
                    puppyMill = PuppyMill(&testTerrain);
                    Eendgine::Entities::shrink();
                    duck.health.heal(100);
                    duck.setPosition(testTerrain.positionAtTile(20.0f, 20.0f, 0.0f));
                }
            }
        } else {
            bool escapePressed = Eend::InputManager::get().getEscapePress();
            if (!escapePressed) escapeReleased = true;
            if (escapeReleased && escapePressed) {
                paused = true;
                escapeReleased = false;
                // create menu
                // testing shrink
            }
        }

        if (paused || dead) {
            // handle menu
        } else {

            testTerrain.update();
            duck.update(dt, &testTerrain);
            puppyMill.update(dt, &duck);

            Eend::Point duckPosition = duck.getPosition();
            sceneCamera.setPosition(
                Eend::Point(duckPosition.x, duckPosition.y - 25.0f, duckPosition.z + 15.0f));
            sceneCamera.setTarget(duckPosition);

            TextBoxQueue::get().update();
            Eend::Particles::get().update(dt);
            Eend::Entities::dolls().getRef(testDollId)->setAnim(testAnimScale);
        }

        Eend::Entities::draw(shaders, hudCamera, sceneCamera);
        Eend::Screen::get().render(shaders.getShader(Eend::Shader::SCREEN));

        Eend::Window::get().swapBuffers();
        Eend::FrameLimiter::get().stopInterval();
    }
    TextBoxQueue::destruct();
    Eend::Particles::destruct();
    Eend::Entities::destruct();
    Eend::Audio::destruct();
    Eend::Screen::destruct();
    Eend::Window::destruct();
    Eend::FrameLimiter::destruct();
    return 0;
}
