#include <queue>

struct bootCircleTraits {
    public:
        float bootRadius;
        float circleRadius;
        float speed;
        bootCircleTraits& operator+=(const bootCircleTraits& rhs) {
            bootRadius += rhs.bootRadius;
            circleRadius += rhs.circleRadius;
            speed += rhs.speed;
            return *this;
        }
        bootCircleTraits& operator-=(const bootCircleTraits& rhs) {
            bootRadius -= rhs.bootRadius;
            circleRadius -= rhs.circleRadius;
            speed -= rhs.speed;
            return *this;
        }
};

class BootCircleItem { // TODO also inherit from item?
    public:
        const bootCircleTraits traits;
};

class bootCircle {
    public:
        bootCircle() {
            traits.bootRadius = 1.0f; // default values go here
            traits.circleRadius = 1.0f;
            traits.speed = 1.0f;
        };

        bool enabled() { return !items.empty(); }

        void addItem(BootCircleItem item) {
            traits += item.traits;
            items.push(item);
        };
        void removeItem() {
            if (enabled()) {
                BootCircleItem item = items.front();
                traits -= item.traits;
                items.pop();
            }
        };

    private:
        bootCircleTraits traits;
        std::queue<BootCircleItem> items;
};
