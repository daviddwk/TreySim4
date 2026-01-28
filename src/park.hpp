#include "duck.hpp"
#include "terrain.hpp"

#include <Eendgine/types.hpp>

#include <filesystem>

class Park {
    public:
        static void construct(std::filesystem::path pngHeightMap, Eend::Scale scale, Duck* duck);
        static void destruct();
        static Park& get();

    public:
        Park(std::filesystem::path pngHeightMap, Eend::Scale scale, Duck* duck);

        inline static Park* m_instance = nullptr;

        void setTerrain(std::filesystem::path pngHeightMap, Eend::Scale scale);
        // just give pointer to terrain
        Terrain& getTerrain();

    private:
        Terrain m_terrain;
        Duck* m_duck;
};
