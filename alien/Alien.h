#ifndef ASTEROIDS_ALIEN_H
#define ASTEROIDS_ALIEN_H

#include <SFML/Graphics.hpp>
#include "../MovableEntity.h"
#include "../GameConstants.h"

// Enum class to track the size of the alien
enum class AlienSize {
    BIG,
    SMALL
};

// Represents an alien spaceship (Saucer enemy)
class Alien : public MovableEntity {
public:
    AlienSize alienSize;

    explicit Alien(AlienSize size);

    void draw(sf::RenderWindow &window) override;

    void update(float delta, bool sound) override;

    void shoot(bool sound, std::optional<sf::Vector2f> playerPos = std::nullopt) override;

    void explode(bool sound) override;

    // Returns true if the alien is destroyed (has no lives left)
    bool isDestroyed() const { return lives <= 0; }

    // Reduces the alien's lives by 1 and explodes if it has no lives left
    void takeDamage(bool sound) { if (--lives <= 0) explode(sound); }

private:
    void initShape() override;

    sf::Clock shootTimer; // Timer to track the time between shots
    float shootInterval; // Time between shots

    sf::ConvexShape cockpit; // Cockpit shape (trapezoid)
    sf::RectangleShape line; // Line in the middle of the alien(hexagon)

    float r; // Radius
    float r_y; // Radius y-axis
    float cockpitHeight;
};

#endif //ASTEROIDS_ALIEN_H
