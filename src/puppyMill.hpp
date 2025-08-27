#include "dog.hpp"
#include "duck.hpp"
#include "terrain.hpp"

class PuppyMill {
    public:
        PuppyMill(Terrain* terrain);
        ~PuppyMill();

        void damage(Duck* duck); // should take in player class
        void update(float dt, Duck* duck);

    private:
        static constexpr int M_DMG_TICK_MS = 200;
        Terrain* m_terrain;
        std::vector<Dog> m_dogs;
};
