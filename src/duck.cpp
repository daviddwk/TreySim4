#include "duck.hpp"

Duck::Duck()
    : _bodyId(Eend::Entities::StatueBatch::insert("resources/duck/duck.obj")),
      _headId(Eend::Entities::BoardBatch::insert({"resources/duck/duckHead.png"})),
      _position(Eend::Point(0.0f)), _rotX(0.0f), _rotY(0.0f) {
    Eend::Entities::BoardBatch::getRef(_headId).setScale(3.5f, 3.5f);
}

Duck::~Duck() {
    Eend::Entities::StatueBatch::erase(_bodyId);
    Eend::Entities::BoardBatch::erase(_headId);
}

void Duck::setPosition(Eend::Point position) {
    _position = position;
    Eend::Statue& bodyRef = Eend::Entities::StatueBatch::getRef(_bodyId);
    Eend::Board& headRef = Eend::Entities::BoardBatch::getRef(_headId);
    bodyRef.setPosition(Eend::Point(position.x, position.y + 0.08f, position.z + 0.5f));
    headRef.setPosition(Eend::Point(position.x, position.y + 3.00f, position.z));
}

void Duck::setRotation(float x, float y) {
    Eend::Statue& bodyRef = Eend::Entities::StatueBatch::getRef(_bodyId);
    bodyRef.setRotation(x, y);
};
