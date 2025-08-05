#include <Eendgine/camera.hpp>
#include <Eendgine/collisionGeometry.hpp>
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/inputManager.hpp>
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

#include "Eendgine/panel.hpp"
#include "dog.hpp"
#include "duck.hpp"
#include "dustCloud.hpp"
#include "healthBar.hpp"
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
    Eend::InputManager::construct();
    Eend::FrameLimiter::construct(60.0f, 20.0f);
    Eend::Entities::construct();

    Eend::Shaders shaders(
        Eend::ShaderProgram("shaders/panel.vert", "shaders/panel.frag"),
        Eend::ShaderProgram("shaders/board.vert", "shaders/board.frag"),
        Eend::ShaderProgram("shaders/statue.vert", "shaders/statue.frag"),
        Eend::ShaderProgram("shaders/doll.vert", "shaders/doll.frag"),
        Eend::ShaderProgram("shaders/screen.vert", "shaders/screen.frag"));

    Eend::Camera2D hudCamera(screenWidth, screenHeight);
    Eend::Camera3D sceneCamera(
        (float)screenWidth / (float)screenHeight, Eend::Point(-20.0f, 5.0f, 0.0f),
        Eend::Point(3.0f, 0.0f, 3.0f));

    TextBoxQueue::construct();

    Terrain testTerrain("terrain/grassy", Eend::Scale(3.0f, 3.0f, 20.0f));

    Duck duck = Duck();
    PuppyMill puppyMill(&testTerrain);

    duck.setPosition(testTerrain.positionAtTile(20.0f, 20.0f, 0.0f));

    float testAnimScale = 0.0f;
    Eend::DollId testDollId = Eend::Entities::dolls().insert("testCube");

    Text testText(Font::DANIEL, "", Eend::Point(20.0f), 50.0f, INFINITY);

    bool testColliding = false;

    Eend::PanelId exitId = Eend::Entities::panels().insert("exit");
    Eend::Panel* exitRef = Eend::Entities::panels().getRef(exitId);
    exitRef->setScale(Eend::Scale2D(50.0f, 50.0f));
    exitRef->setPosition(Eend::Point((float)screenWidth - 80.0f, 30.0f, 0.0f));

    TextBoxQueue::get().queue("duck", Font::DANIEL, "Help meeeee!", 3.0f, true);
    TextBoxQueue::get().queue("dog", Font::DANIEL, "It's over for you bucko.", 3.0f, false);
    TextBoxQueue::get().queue(
        "duck", Font::DANIEL,
        "What the duck did you just call me? You little quack! "
        "Aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahhhhhhhhhhhh",
        5.0f, true);

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
        }
        if (exitMouseStatus == Eend::Panel::MouseStatus::HOVER) {
            exitMouseString = "hover";
        }
        if (exitMouseStatus == Eend::Panel::MouseStatus::NONE) {
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
                "left:{} right:{} mid:{}\n",
                1.0f / dt, dt,
                duck.getPosition().x, duck.getPosition().y, duck.getPosition().z,
                testColliding, exitMouseString,
                Eend::InputManager::get().getMouseX(), Eend::InputManager::get().getDeltaMouseX(),
                Eend::InputManager::get().getMouseY(),Eend::InputManager::get().getDeltaMouseY(),
                Eend::InputManager::get().getLeftClick(),
                Eend::InputManager::get().getRightClick(),
                Eend::InputManager::get().getMiddleClick()));
        // clang-format on
        Eend::Entities::draw(shaders, hudCamera, sceneCamera);
        Eend::Screen::get().render(shaders.getShader(Eend::Shader::SCREEN));

        Eend::InputManager::get().processInput();

        duck.update(dt, &testTerrain);
        testTerrain.update();

        puppyMill.damage(&duck);

        puppyMill.update(dt, &duck);
        TextBoxQueue::get().update();

        Eend::Point duckPosition = duck.getPosition();
        sceneCamera.setPosition(
            Eend::Point(duckPosition.x, duckPosition.y - 25.0f, duckPosition.z + 15.0f));
        sceneCamera.setTarget(duckPosition);

        Eend::Entities::dolls().getRef(testDollId)->setAnim(testAnimScale);

        Eend::Window::get().swapBuffers();
        Eend::FrameLimiter::get().stopInterval();
    }
    TextBoxQueue::destruct();
    Eend::Entities::destruct();
    Eend::Screen::destruct();
    Eend::Window::destruct();
    Eend::FrameLimiter::destruct();
    return 0;
}
