#include <Eendgine/camera.hpp>
#include <Eendgine/collisionGeometry.hpp>
#include <Eendgine/entityBatches.hpp>
#include <Eendgine/frameLimiter.hpp>
#include <Eendgine/info.hpp>
#include <Eendgine/inputManager.hpp>
#include <Eendgine/model.hpp>
#include <Eendgine/screen.hpp>
#include <Eendgine/shader.hpp>
#include <Eendgine/shaders.hpp>
#include <Eendgine/sprite.hpp>
#include <Eendgine/text.hpp>
#include <Eendgine/textureCache.hpp>
#include <Eendgine/window.hpp>

#include <stb/stb_image.h>

#include <filesystem>
#include <numbers>
#include <vector>

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

    Eend::Info::registerFloat("billHeight", 0);
    Eend::Info::registerFloat("duck rotation", 0);

    glEnable(GL_DEPTH_TEST);

    Eend::ShaderProgram newShader("shaders/shader3D.vert", "shaders/shader3D.frag");
    Eend::Shaders shaders(Eend::ShaderProgram("shaders/shader.vert", "shaders/shader.frag"),
        Eend::ShaderProgram("shaders/shader3D.vert", "shaders/shader3D.frag"),
        Eend::ShaderProgram("shaders/shaderInpol.vert", "shaders/shaderInpol.frag"),
        Eend::ShaderProgram("shaders/shaderScreen.vert", "shaders/shaderScreen.frag"));

    Eend::Camera2D hudCamera(screenWidth, screenHeight);
    Eend::Camera3D sceneCamera((float)screenWidth / (float)screenHeight,
        glm::vec3(-20.0f, 5.0f, 0.0f), glm::vec3(3.0f, 0.0f, 3.0f));

    Eend::Info::registerInt("main", 0);

    Terrain testTerrain("resources/terrain/test", glm::vec3(7.0f, 10.0f, 7.0f));

    Duck duck = Duck();

    float duckHeight = testTerrain.heightAtPoint(0.0f, 0.0f);

    glm::vec3 duckPosition = duck.getPosition();
    sceneCamera.setPosition(
        glm::vec3(duckPosition.x + 25.0f, duckPosition.y + 15.0f, duckPosition.z));
    sceneCamera.setTarget(duckPosition);
    float duckRotation = 0.0f;

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
            duckPosition.x -= 0.03f * dt;
            duckRotationOffset += 90.0f;
            numPressed++;
        }
        if (Eend::InputManager::downPress) {
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

        duckPosition.y = testTerrain.heightAtPoint(duckPosition.x, duckPosition.z);

        duck.setPosition(duckPosition);
        duck.setRotation(duckRotation, 0.0f);
        sceneCamera.setPosition(
            glm::vec3(duckPosition.x + 25.0f, duckPosition.y + 15.0f, duckPosition.z));
        sceneCamera.setTarget(duckPosition);

        Eend::Window::swapBuffers();

        Eend::Info::print();

        Eend::FrameLimiter::stopInterval();
    }
    Eend::Screen::close();
    Eend::Info::close();
    Eend::FrameLimiter::close();
    return 0;
}
