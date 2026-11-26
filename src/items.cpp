#include "items.hpp"
#include "park.hpp"

#include <Eendgine/fatalError.hpp>

constexpr float HEIGHT_OFFSET = 5.0f;
constexpr Eend::Scale2D SCALE = Eend::Scale2D(5.0f);

// maybe could template but probably not really worth
Item::Item(Eend::Point position, std::filesystem::path texture, BootCircleTraits traits)
    : position(position), sprite(Eend::Entities::boards().insert(texture)), texture(texture),
      traits(traits) {
    construct();
}

Item::Item(Eend::Point position, std::filesystem::path texture, BootThrowTraits traits)
    : position(position), sprite(Eend::Entities::boards().insert(texture)), texture(texture),
      traits(traits) {

    construct();
}

void Item::construct() {
    Eend::Board* spriteRef = Eend::Entities::boards().getRef(sprite);
    spriteRef->setScale(Eend::Scale2D(SCALE));
    spriteRef->setPosition(position);
}

Item::~Item() { Eend::Entities::boards().erase(sprite); }

Item::Item(const Item& other) noexcept
    : position(other.position), sprite(Eend::Entities::boards().clone(other.sprite)),
      texture(other.texture), traits(other.traits) {}

Item& Item::operator=(const Item& other) noexcept {

    if (&other == this) return *this;

    Eend::Entities::boards().erase(sprite);

    position = other.position;
    sprite = Eend::Entities::boards().clone(other.sprite);
    texture = other.texture;
    traits = other.traits;

    return *this;
}

Eend::Point Item::getPosition() { return position; }

BootCircleTraits Item::getBootCircleTraits() { return traits.boot_circle; }
BootThrowTraits Item::getBootThrowTraits() { return traits.boot_throw; }
