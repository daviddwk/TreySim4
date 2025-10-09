#include "dog.hpp"
#include "duck.hpp"
#include "terrain.hpp"

class PuppyMill {
    public:
        PuppyMill(Terrain* terrain);
        ~PuppyMill();

        void update(float dt, Duck* duck);

    private:
        void damage(Duck* duck);
        void spawn();

        static constexpr int M_DMG_TICK_MS = 200;
        static constexpr float M_SPAWN_TIME_MS = 1000;

        Terrain* m_terrain;
        std::vector<Dog> m_dogs;
};
