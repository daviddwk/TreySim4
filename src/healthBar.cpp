#include "healthBar.hpp"

const unsigned int HEALTH_MAX = 100;
const float BAR_SCALE_MAX = 500.0f;

HealthBar::HealthBar()
    : _background(
          Eend::Entities::PanelBatch::insert(std::filesystem::path("healthbar/background"))),
      _bar(Eend::Entities::PanelBatch::insert(std::filesystem::path("healthbar/bar"))),
      _health(100) {
    Eend::Panel* backgroundRef = Eend::Entities::PanelBatch::getRef(_background);
    Eend::Panel* barRef = Eend::Entities::PanelBatch::getRef(_bar);

    backgroundRef->setScale(Eend::Scale2D(BAR_SCALE_MAX + 30.0f, 85.0f));
    backgroundRef->setPosition(Eend::Point(1285.0f, 20.0f, -10.0f));

    barRef->setTexture("high");
    barRef->setScale(Eend::Scale2D(BAR_SCALE_MAX, 75.0f));
    barRef->setPosition(Eend::Point(1300.0f, 25.0f, 100.0f));
}

HealthBar::~HealthBar() {
    Eend::Entities::PanelBatch::erase(_background);
    Eend::Entities::PanelBatch::erase(_bar);
}

bool HealthBar::damage(unsigned int damage) {
    bool ret = false;
    if (damage >= _health) {
        _health = 0;
        ret = true;
    } else {
        _health -= damage;
        ret = false;
    }
    update();
    return ret;
}

void HealthBar::heal(unsigned int heal) {
    if (heal + _health > HEALTH_MAX) {
        _health = HEALTH_MAX;
    } else {
        _health += heal;
    }
    update();
}

void HealthBar::update() {
    Eend::Panel* barRef = Eend::Entities::PanelBatch::getRef(_bar);
    barRef->setScale(Eend::Scale2D(BAR_SCALE_MAX * ((float)_health / (float)HEALTH_MAX), 75.0f));
    if (_health > 70.0) {
        barRef->setTexture("high");
    } else if (_health > 30.0f) {
        barRef->setTexture("mid");
    } else {
        barRef->setTexture("low");
    }
}
