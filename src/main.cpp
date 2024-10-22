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

#include "terrain.hpp"

namespace Eend = Eendgine;

const unsigned int screenHeight = 750;
const unsigned int screenWidth = 1000;

int main() {

    Eend::Window::init(screenWidth, screenHeight, "Neigh");
    Eend::Screen::init(screenWidth, screenHeight);
    Eend::InputManager::init();
    Eend::Info::init();
    Eend::FrameLimiter::init(30.0f);

    Eend::Info::registerFloat("billHeight", 0);

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

    Eend::BillboardId bill = Eend::Entities::BillboardBatch::insert({"resources/duck2.png"});
    float billHeight = testTerrain.heightAtPoint(0.0f, 0.0f);
    glm::vec3 billPos = glm::vec3(0.0f, billHeight, 0.0f);
    glm::vec3 cameraPos = glm::vec3(billPos.x + 15.0f, billPos.y + 10.0f, billPos.x);
    Eend::Sprite& billRef = Eend::Entities::BillboardBatch::getRef(bill);
    billRef.setScale(2.0f, 2.0f);
    billRef.setPosition(billPos);
    sceneCamera.setPosition(cameraPos);
    sceneCamera.setTarget(billPos);

    while (!Eend::InputManager::shouldClose) {
        float dt = Eend::FrameLimiter::deltaTime;
        Eend::FrameLimiter::startInterval();
        Eend::Screen::bind();
        shaders.setPixelSize(5);

        Eend::Entities::draw(shaders, hudCamera, sceneCamera);
        Eend::Screen::render(shaders.getShader(Eend::Shader::screen));

        Eend::InputManager::processInput();

        if (Eend::InputManager::upPress) {
            billPos.x -= 0.03f * dt;
        }
        if (Eend::InputManager::downPress) {
            billPos.x += 0.03f * dt;
        }
        if (Eend::InputManager::leftPress) {
            billPos.z += 0.03f * dt;
        }
        if (Eend::InputManager::rightPress) {
            billPos.z -= 0.03f * dt;
        }
        billPos.y = testTerrain.heightAtPoint(billPos.x, billPos.z) + 1.0f;

        glm::vec3 cameraPos = glm::vec3(billPos.x + 15.0f, billPos.y + 10.0f, billPos.z);
        Eend::Sprite& billRef = Eend::Entities::BillboardBatch::getRef(bill);
        billRef.setScale(2.0f, 2.0f);
        billRef.setPosition(billPos);
        sceneCamera.setPosition(cameraPos);
        sceneCamera.setTarget(billPos);

        Eend::Window::swapBuffers();

        Eend::Info::print();

        Eend::FrameLimiter::stopInterval();
    }
    Eend::Screen::close();
    Eend::Info::close();
    Eend::FrameLimiter::close();
    return 0;
}
