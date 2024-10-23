#include <Eendgine/entityBatches.hpp>

namespace Eend = Eendgine;

class Duck {
        // please make a position, rotation, whatever types already and decouple dependencies or
        // whatever
    public:
        Duck();
        ~Duck();

        void setPosition(glm::vec3 position);
        void setRotation(float x, float y);

        glm::vec3 getPosition() { return _position; };

    private:
        glm::vec3 _position;
        float _rotX;
        float _rotY;
        Eend::ModelId _bodyId;
        Eend::BillboardId _headId;
};
