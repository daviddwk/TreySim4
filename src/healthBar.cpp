#include "healthBar.hpp"

const unsigned int HEALTH_MAX = 100;
const float BAR_SCALE_MAX = 500.0f;

HealthBar::HealthBar()
    : m_background(Eend::Entities::panels().insert(std::filesystem::path("healthbar/background"))),
      m_bar(Eend::Entities::panels().insert(std::filesystem::path("healthbar/bar"))),
      m_health(100) {
    Eend::Panel* backgroundRef = Eend::Entities::panels().getRef(m_background);
    Eend::Panel* barRef = Eend::Entities::panels().getRef(m_bar);

    backgroundRef->setScale(Eend::Scale2D(BAR_SCALE_MAX + 30.0f, 85.0f));
    backgroundRef->setPosition(Eend::Point(1285.0f, 20.0f, -10.0f));

    barRef->setTexture("high");
    barRef->setScale(Eend::Scale2D(BAR_SCALE_MAX, 75.0f));
    barRef->setPosition(Eend::Point(1300.0f, 25.0f, 100.0f));
}

HealthBar::~HealthBar() {
    Eend::Entities::panels().erase(m_background);
    Eend::Entities::panels().erase(m_bar);
}

bool HealthBar::damage(unsigned int damage) {
    bool ret = false;
    if (damage >= m_health) {
        m_health = 0;
        ret = true;
    } else {
        m_health -= damage;
        ret = false;
    }
    update();
    return ret;
}

void HealthBar::heal(unsigned int heal) {
    if (heal + m_health > HEALTH_MAX) {
        m_health = HEALTH_MAX;
    } else {
        m_health += heal;
    }
    update();
}

void HealthBar::update() {
    Eend::Panel* barRef = Eend::Entities::panels().getRef(m_bar);
    barRef->setScale(Eend::Scale2D(BAR_SCALE_MAX * ((float)m_health / (float)HEALTH_MAX), 75.0f));
    if (m_health > 70.0) {
        barRef->setTexture("high");
    } else if (m_health > 30.0f) {
        barRef->setTexture("mid");
    } else {
        barRef->setTexture("low");
    }
}
