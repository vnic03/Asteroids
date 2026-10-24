#ifndef ASTEROIDS_ALIEN_H
#define ASTEROIDS_ALIEN_H

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

    explicit Alien(AlienSize size);

    void draw(sf::RenderWindow &window) override;

    void update(float delta, bool sound) override;

    void shoot(bool sound, std::optional<sf::Vector2f> playerPos =
            std::nullopt) override;

    void explode(bool sound) override;

    bool isDestroyed() const { return lives <= 0; }

    void takeDamage(bool sound) {
        if (isDestroyed()) return;

        --lives;

        if (lives <= 0) explode(sound);
    }

private:
    void initShape() override;

    void updateVisualPositions();

    SimulationClock shootTimer;
    float directionElapsed = 0.f;
    float shootInterval; // Time between shots

    sf::ConvexShape cockpit;
    sf::RectangleShape line;

    float r; // Radius
    float r_y; // Radius y-axis
    float cockpitHeight;
};

#endif //ASTEROIDS_ALIEN_H
