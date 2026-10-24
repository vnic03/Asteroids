#pragma once

#include <SFML/Graphics.hpp>

#include "../MovableEntity.h"
#include "../GameConstants.h"

enum class AlienSize {
    BIG,
    SMALL
};

class Alien : public MovableEntity {
public:
    AlienSize alienSize;
    float collisionRadius = 0.f;

    explicit Alien(
        AlienSize size,
        int level = 1,
        std::optional<sf::Vector2f> position = std::nullopt
    );

    void draw(sf::RenderWindow& window) override;

    void update(float delta, bool sound) override;

    void shoot(
        bool sound,
        std::optional<sf::Vector2f> playerPos = std::nullopt
    ) override;

    void explode(bool sound) override;

    bool isDestroyed() const {
        return lives <= 0;
    }

    void takeDamage(bool sound) {
        if (isDestroyed()) return;

        --lives;

        if (lives <= 0) explode(sound);
    }

private:
    void initShape() override;
    void updateVisualPositions();

    SimulationClock shootTimer;
    SimulationClock age;
    SimulationClock chargeTimer;

    bool charging = false;
    sf::Vector2f aimDirection{0.f, 1.f};
    int volley = 0;

    float directionElapsed = 0.f;
    float shootInterval;

    sf::ConvexShape cockpit;
    sf::RectangleShape line;

    float r;
    float r_y;
    float cockpitHeight;
};
