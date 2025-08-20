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
        // DAMAGE (probably don't do every frame)
        // void takeDamage();
        // void dealDamage();

        // UPDATE
        // move
        // collision and correct
        //
        // spawn
        Terrain* m_terrain;
        std::vector<Dog> m_dogs;
};
