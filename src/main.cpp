#include <Eendgine/camera.hpp>
#include <Eendgine/collisionGeometry.hpp>
#include <Eendgine/entityBatch.hpp>
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

    glEnable(GL_DEPTH_TEST);

    Eend::ShaderProgram newShader("shaders/shader3D.vert", "shaders/shader3D.frag");
    Eend::Shaders shaders(Eend::ShaderProgram("shaders/shader.vert", "shaders/shader.frag"),
        Eend::ShaderProgram("shaders/shader3D.vert", "shaders/shader3D.frag"),
        Eend::ShaderProgram("shaders/shaderInpol.vert", "shaders/shaderInpol.frag"),
        Eend::ShaderProgram("shaders/shaderScreen.vert", "shaders/shaderScreen.frag"));

    Eend::Camera2D hudCamera(screenWidth, screenHeight);
    Eend::Camera3D sceneCamera((float)screenWidth / (float)screenHeight,
        glm::vec3(20.0f, 15.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f));

    std::vector<Eend::CollisionModel *> myColModels;

    Terrain testTerrain;
    testTerrain.generate_terrain("resources/new.png");

    Eend::Entities::ModelBatch::insert("resources/new.obj");

    while (!Eend::InputManager::shouldClose) {
        Eend::FrameLimiter::startInterval();
        Eend::Screen::bind();

        Eend::Entities::draw(shaders, hudCamera, sceneCamera);
        Eend::Screen::render(shaders.getShader(Eend::Shader::screen));

        Eend::InputManager::processInput();

        Eend::Window::swapBuffers();

        Eend::Info::print();

        Eend::FrameLimiter::stopInterval();
    }
    Eend::Screen::close();
    Eend::Info::close();
    Eend::FrameLimiter::close();
    return 0;
}
