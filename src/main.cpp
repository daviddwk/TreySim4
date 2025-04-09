#include <Eendgine/camera.hpp>
#include <Eendgine/collisionGeometry.hpp>
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/inputManager.hpp>
#include <Eendgine/screen.hpp>
#include <Eendgine/shader.hpp>
#include <Eendgine/shaders.hpp>
#include <Eendgine/text.hpp>
#include <Eendgine/textureCache.hpp>
#include <Eendgine/types.hpp>
#include <Eendgine/window.hpp>

#include <cmath>
#include <stb/stb_image.h>

#include <filesystem>
#include <print>

#include "Eendgine/panel.hpp"
#include "duck.hpp"
#include "terrain.hpp"

namespace Eend = Eendgine;

const unsigned int screenHeight = 1080;
const unsigned int screenWidth = 1920;

int main() {

    Eend::Window::init(screenWidth, screenHeight, "Quack");
    Eend::Screen::init(screenWidth, screenHeight);
    Eend::InputManager::init();
    Eend::FrameLimiter::init(60.0f, 20.0f);

    glEnable(GL_DEPTH_TEST);

    Eend::ShaderProgram newShader("shaders/shader3D.vert", "shaders/shader3D.frag");
    Eend::Shaders shaders(Eend::ShaderProgram("shaders/shader.vert", "shaders/shader.frag"),
        Eend::ShaderProgram("shaders/shader3D.vert", "shaders/shader3D.frag"),
        Eend::ShaderProgram("shaders/shaderInpol.vert", "shaders/shaderInpol.frag"),
        Eend::ShaderProgram("shaders/shaderScreen.vert", "shaders/shaderScreen.frag"));

    Eend::Camera2D hudCamera(screenWidth, screenHeight);
    Eend::Camera3D sceneCamera((float)screenWidth / (float)screenHeight,
        Eend::Point(-20.0f, 5.0f, 0.0f), Eend::Point(3.0f, 0.0f, 3.0f));

    Terrain testTerrain("terrain/grassy", Eend::Scale(3.0f, 20.0f, 3.0f));

    Duck duck = Duck();

    duck.setPosition(testTerrain.positionAtTile(20.0f, 20.0f, 0.0f));
    Eend::Point duckPosition = duck.getPosition();
    sceneCamera.setPosition(
        Eend::Point(duckPosition.x, duckPosition.y + 15.0f, duckPosition.z + 25.0f));
    sceneCamera.setTarget(duckPosition);
    float duckRotation = 0.0f;

    float testAnimScale = 0.0f;
    Eend::DollId testDollId = Eend::Entities::DollBatch::insert("testCube");

    for (unsigned int i = 0; i < 50; ++i)
        Eend::DollId testDollId = Eend::Entities::DollBatch::insert("testCube");
    // Eend::StatueId testStatue = Eend::Entities::StatueBatch::insert("duck/statues/body");

    Eend::Text testText("daniel", "", Eend::Point(20.0f), 50.0f, INFINITY);

    Eend::CollisionRectangle testRectangle = {
        .upperLeft = Eend::Point(0.0f), .lowerRight = Eend::Point(5.0f)};
    bool testColliding = false;

    Eend::PanelId exitId = Eend::Entities::PanelBatch::insert("exit");
    Eend::Panel* exitRef = Eend::Entities::PanelBatch::getRef(exitId);
    exitRef->setScale(Eend::Scale2D(50.0f, 50.0f));
    exitRef->setPosition(Eend::Point((float)screenWidth - 80.0f, 30.0f, 0.0f));

    while (!Eend::InputManager::getShouldClose()) {
        float dt = Eend::FrameLimiter::deltaTime;
        Eend::FrameLimiter::startInterval();
        Eend::Screen::bind();
        shaders.setPixelSize(5);

        Eend::Panel::MouseStatus exitMouseStatus =
            Eend::Entities::PanelBatch::getRef(exitId)->isClicked(Eend::InputManager::getMouseX(),
                Eend::InputManager::getMouseY(), Eend::InputManager::getLeftClick());
        std::string exitMouseString = "";
        if (exitMouseStatus == Eend::Panel::MouseStatus::click) {
            exitMouseString = "click";
        }
        if (exitMouseStatus == Eend::Panel::MouseStatus::hover) {
            exitMouseString = "hover";
        }
        if (exitMouseStatus == Eend::Panel::MouseStatus::none) {
            exitMouseString = "none";
        }

        if (exitMouseStatus == Eend::Panel::MouseStatus::click) {
            Eend::InputManager::setShouldClose(true);
        }

        testText.setText(std::format("FPS:{:.4f} DT:{:.4f}\n"
                                     "X:{:.4f} Y:{:.4f} Z:{:.4f}\n"
                                     "duck:{} mouse:{} \n"
                                     "mouseX:{} dx:{} mouseY:{} dy:{}\n"
                                     "left:{} right:{} mid:{}\n",
            1.0f / dt, dt, duck.getPosition().x, duck.getPosition().y, duck.getPosition().z,
            testColliding, exitMouseString, Eend::InputManager::getMouseX(),
            Eend::InputManager::getDeltaMouseX(), Eend::InputManager::getMouseY(),
            Eend::InputManager::getDeltaMouseY(), Eend::InputManager::getLeftClick(),
            Eend::InputManager::getRightClick(), Eend::InputManager::getMiddleClick()));
        Eend::Entities::draw(shaders, hudCamera, sceneCamera);
        Eend::Screen::render(shaders.getShader(Eend::Shader::screen));

        duckPosition = duck.getPosition();
        Eend::Point oldDuckPosition = duckPosition;

        Eend::InputManager::processInput();

        float duckRotationOffset = 0.0f;
        unsigned int numPressed = 0;
        if (Eend::InputManager::getUpPress()) {
            Eend::Entities::DollBatch::getRef(testDollId)->setAnimation("one");
            duckPosition.z -= 25.0f * dt;
            // stupid hack because my trig is mid
            if (Eend::InputManager::getRightPress()) {
                duckRotationOffset = -180.0f;
            } else {
                duckRotationOffset += 180.0f;
            }
            numPressed++;
        }
        if (Eend::InputManager::getDownPress()) {
            Eend::Entities::DollBatch::getRef(testDollId)->setAnimation("two");
            duckPosition.z += 25.0f * dt;
            duckRotationOffset += 0.0f;
            numPressed++;
        }
        if (Eend::InputManager::getLeftPress()) {
            duckPosition.x -= 25.0f * dt;
            duckRotationOffset += 90.0f;
            numPressed++;
        }
        if (Eend::InputManager::getRightPress()) {
            duckPosition.x += 25.0f * dt;
            duckRotationOffset -= 90.0f;
            numPressed++;
        }
        // COORDINATE SYSTMES ARE TOTALLY WACKED UP RN

        if (!testTerrain.colliding(Eend::Point2D(duckPosition.x, duckPosition.z))) {
            testColliding = false;
        } else if (!testTerrain.colliding(Eend::Point2D(oldDuckPosition.x, duckPosition.z))) {
            duckPosition.x = oldDuckPosition.x;
            testColliding = true;
        } else if (!testTerrain.colliding(Eend::Point2D(duckPosition.x, oldDuckPosition.z))) {
            duckPosition.z = oldDuckPosition.z;
            testColliding = true;
        } else {
            duckPosition.x = oldDuckPosition.x;
            duckPosition.z = oldDuckPosition.z;
            testColliding = true;
        }

        if (numPressed) {
            duckRotation = (duckRotationOffset / (float)numPressed);
        } else {
            duckRotation += 100.0f * dt;
        }

        duckPosition.y = testTerrain.heightAtPoint(Eend::Point2D(duckPosition.x, duckPosition.z));
        testTerrain.update();
        duck.setPosition(duckPosition);
        duck.setRotation(duckRotation, 0.0f);
        sceneCamera.setPosition(
            Eend::Point(duckPosition.x, duckPosition.y + 15.0f, duckPosition.z + 25.0f));
        sceneCamera.setTarget(duckPosition);

        Eend::Entities::DollBatch::getRef(testDollId)->setAnim(testAnimScale);

        Eend::Window::swapBuffers();
        Eend::FrameLimiter::stopInterval();
    }
    Eend::Screen::close();
    Eend::Window::close();
    Eend::FrameLimiter::close();
    return 0;
}
