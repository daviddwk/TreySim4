#pragma once

#include "puppyMill.hpp"
#include "terrain.hpp"

#include <Eendgine/types.hpp>

#include <filesystem>

class Park {
    public:
        static void construct(std::filesystem::path pngHeightMap, Eend::Scale scale);
        static void destruct();
        static Park& get();

        // maybe have an update function so that this actually gets set at end of frame
        void update(float dt);
        void reset();

        unsigned int numDogsKilled();

        void setTerrain(std::filesystem::path pngHeightMap, Eend::Scale scale);
        // just give pointer to terrain
        Terrain& getTerrain();

    private:
        Park(std::filesystem::path pngHeightMap, Eend::Scale scale);
        ~Park() = default;

        Park(const Park& other) = delete;
        Park& operator=(const Park& other) = delete;

        Park(Park&& other) = delete;
        Park& operator=(Park&& other) = delete;

        inline static Park* m_instance = nullptr;

        Terrain m_terrain;
        // part of park and dog spawns should be a part of the park format TODO
        PuppyMill m_puppyMill;
};
