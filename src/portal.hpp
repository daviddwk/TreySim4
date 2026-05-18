#pragma once

#include <Eendgine/types.hpp>
#include <filesystem>

namespace Eend = Eendgine;

class Portal {
    public:
        Portal(Eend::Point2D position, Eend::Vector2D toCorner, std::filesystem::path terrainPath)
            : collision(
                  position - Eend::Vector2D(0.0f, toCorner.y),
                  position + Eend::Vector2D(toCorner.x, 0.0f)),
              terrainPath(terrainPath) {}

        Eend::Rectangle collision;
        std::filesystem::path terrainPath;
};
