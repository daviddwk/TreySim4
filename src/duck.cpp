#include "duck.hpp"

Duck::Duck()
    : _bodyId(Eend::Entities::ModelBatch::insert("resources/duck/duck.obj")),
      _headId(Eend::Entities::BillboardBatch::insert({"resources/duck/duckHead.png"})),
      _position(glm::vec3(0.0f)), _rotX(0.0f), _rotY(0.0f) {
    Eend::Entities::BillboardBatch::getRef(_headId).setScale(3.5f, 3.5f);
}

Duck::~Duck() {
    Eend::Entities::ModelBatch::erase(_bodyId);
    Eend::Entities::BillboardBatch::erase(_headId);
}

void Duck::setPosition(glm::vec3 position) {
    _position = position;
    Eend::Model& bodyRef = Eend::Entities::ModelBatch::getRef(_bodyId);
    Eend::Sprite& headRef = Eend::Entities::BillboardBatch::getRef(_headId);
    bodyRef.setPosition(glm::vec3(position.x, position.y + 0.08f, position.z + 0.5f));
    headRef.setPosition(glm::vec3(position.x, position.y + 3.00f, position.z));
}

void Duck::setRotation(float x, float y) {
    Eend::Model& bodyRef = Eend::Entities::ModelBatch::getRef(_bodyId);
    bodyRef.setRotation(x, y);
};
