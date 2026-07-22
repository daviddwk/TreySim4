#include "items.hpp"
#include "park.hpp"

#include <Eendgine/fatalError.hpp>

constexpr float HEIGHT_OFFSET = 5.0f;
constexpr Eend::Scale2D SCALE = Eend::Scale2D(5.0f);

Item::Item(Item::Type type, Eend::Point position)
    : m_type(type), m_position(position),
      m_sprite(Eend::Entities::boards().insert(Item::spritePath(type))) {
    Eend::Board* spriteRef = Eend::Entities::boards().getRef(m_sprite);
    spriteRef->setScale(Eend::Scale2D(SCALE));
    spriteRef->setPosition(position);
}

Item::~Item() { Eend::Entities::boards().erase(m_sprite); }

Item::Item(const Item& other) noexcept
    : m_type(other.m_type), m_position(other.m_position),
      m_sprite(Eend::Entities::boards().clone(other.m_sprite)) {}

Item& Item::operator=(const Item& other) noexcept {

    if (&other == this) return *this;

    Eend::Entities::boards().erase(m_sprite);

    m_type = other.m_type;
    m_position = other.m_position;
    m_sprite = Eend::Entities::boards().clone(other.m_sprite);

    return *this;
}

Eend::Point Item::getPosition() { return m_position; }

Item::Type Item::getType() { return m_type; }

std::filesystem::path Item::spritePath(Item::Type type) {
    switch (type) {
    case Item::Type::doubleKick:
        return std::filesystem::path("resources/duck/statues/body");
        break;
    default:
        return std::filesystem::path("resources/duck/statues/body");
        break;
    }
}

Item::Type Item::typeFromString(std::string typeString) {
    if (typeString == "doubleKick") {
        return Item::Type::doubleKick;
    } else {
        Eend::fatalError("item type " + typeString + " does not exsist");
    }
}
