#include <Eendgine/camera.hpp>
#include <Eendgine/collisionGeometry.hpp>
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/info.hpp>
#include <Eendgine/inputManager.hpp>
#include <Eendgine/screen.hpp>
#include <Eendgine/shader.hpp>
#include <Eendgine/shaders.hpp>
#include <Eendgine/sprite.hpp>
#include <Eendgine/text.hpp>
#include <Eendgine/textureCache.hpp>
#include <Eendgine/types.hpp>
#include <Eendgine/window.hpp>

#include <stb/stb_image.h>

#include <filesystem>

#include "duck.hpp"
#include "terrain.hpp"

namespace Eend = Eendgine;

const unsigned int screenHeight = 1080;
const unsigned int screenWidth = 1920;

int main() {
    Eend::Window::init(screenWidth, screenHeight, "Neigh");
    Eend::Screen::init(screenWidth, screenHeight);
    Eend::InputManager::init();
    Eend::Info::init();
    Eend::FrameLimiter::init(30.0f);

    Eend::Info::registerFloat("billHeight", INFO_OPTION_NONE);
    Eend::Info::registerFloat("duck rotation", INFO_OPTION_NONE);
    Eend::Info::registerInt("textNum", INFO_OPTION_NONE);
    Eend::Info::registerFloat("_animScale", INFO_OPTION_NONE);
    Eend::Info::registerFloat("readScale", INFO_OPTION_NONE);

    glEnable(GL_DEPTH_TEST);

    Eend::ShaderProgram newShader("shaders/shader3D.vert", "shaders/shader3D.frag");
    Eend::Shaders shaders(Eend::ShaderProgram("shaders/shader.vert", "shaders/shader.frag"),
        Eend::ShaderProgram("shaders/shader3D.vert", "shaders/shader3D.frag"),
        Eend::ShaderProgram("shaders/shaderInpol.vert", "shaders/shaderInpol.frag"),
        Eend::ShaderProgram("shaders/shaderScreen.vert", "shaders/shaderScreen.frag"));

    Eend::Camera2D hudCamera(screenWidth, screenHeight);
    Eend::Camera3D sceneCamera((float)screenWidth / (float)screenHeight,
        Eend::Point(-20.0f, 5.0f, 0.0f), Eend::Point(3.0f, 0.0f, 3.0f));

    Eend::Info::registerInt("main", INFO_OPTION_NONE);

    Terrain testTerrain("terrain/test", Eend::Scale(7.0f, 10.0f, 7.0f));
    unsigned int textNum = 1234;
    // Eend::Text testText("test", std::to_string(textNum), Eend::Point(0.0f), 100.0f);

    Duck duck = Duck();

    Eend::Point duckPosition = duck.getPosition();
    sceneCamera.setPosition(
        Eend::Point(duckPosition.x + 25.0f, duckPosition.y + 15.0f, duckPosition.z));
    sceneCamera.setTarget(duckPosition);
    float duckRotation = 0.0f;

    float testAnimScale = 0.0f;
    Eend::DollId testDollId = Eend::Entities::DollBatch::insert("testCube");

    Eend::StatueId testStatue = Eend::Entities::StatueBatch::insert("duck/statues/body");

    while (!Eend::InputManager::shouldClose) {
        float dt = Eend::FrameLimiter::deltaTime;
        Eend::FrameLimiter::startInterval();
        Eend::Screen::bind();
        shaders.setPixelSize(5);

        Eend::Entities::draw(shaders, hudCamera, sceneCamera);
        Eend::Screen::render(shaders.getShader(Eend::Shader::screen));

        duckPosition = duck.getPosition();

        Eend::InputManager::processInput();

        float duckRotationOffset = 0.0f;
        unsigned int numPressed = 0;
        if (Eend::InputManager::upPress) {
            Eend::Entities::DollBatch::getRef(testDollId).setAnimation("one");
            textNum += 1;
            // testText.setText(std::to_string(textNum));
            // testText.clearText();
            duckPosition.x -= 0.03f * dt;
            duckRotationOffset += 90.0f;
            numPressed++;
        }
        if (Eend::InputManager::downPress) {
            Eend::Entities::DollBatch::getRef(testDollId).setAnimation("two");
            duckPosition.x += 0.03f * dt;
            duckRotationOffset -= 90.0f;
            numPressed++;
        }
        if (Eend::InputManager::leftPress) {
            duckPosition.z += 0.03f * dt;
            duckRotationOffset += 0.0f;
            numPressed++;
        }
        if (Eend::InputManager::rightPress) {

            duckPosition.z -= 0.03f * dt;
            // stupid hack because my trig is mid
            if (duckRotationOffset < 0.0f) {
                duckRotationOffset = -270.0f;
            } else {
                duckRotationOffset += 180.0f;
            }
            numPressed++;
        }
        if (numPressed) {
            duckRotation = (duckRotationOffset / (float)numPressed);
        } else {
            duckRotation += 0.1f * dt;
        }
        Eend::Info::updateFloat("duck rotation", duckRotation);
        Eend::Info::updateInt("textNum", textNum);

        duckPosition.y = testTerrain.heightAtPoint(duckPosition.x, duckPosition.z);

        duck.setPosition(duckPosition);
        duck.setRotation(duckRotation, 0.0f);
        sceneCamera.setPosition(
            Eend::Point(duckPosition.x + 25.0f, duckPosition.y + 15.0f, duckPosition.z));
        sceneCamera.setTarget(duckPosition);

        testAnimScale += 0.001f * dt;
        Eend::Info::updateFloat("_animScale", testAnimScale);
        Eend::Info::updateFloat(
            "readScale", Eend::Entities::DollBatch::getRef(testDollId).getAnim());
        Eend::Entities::DollBatch::getRef(testDollId).setAnim(testAnimScale);

        Eend::Window::swapBuffers();

        Eend::Info::print();

        Eend::FrameLimiter::stopInterval();
    }
    Eend::Screen::close();
    Eend::Info::close();
    Eend::FrameLimiter::close();
    return 0;
}
