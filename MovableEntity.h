#ifndef ASTEROIDS_MOVABLEENTITY_H
#define ASTEROIDS_MOVABLEENTITY_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <SFML/System/Clock.hpp>
#include <optional>
#include "GameConstants.h"

// Represents a projectile with a position, direction, speed, and radius
struct Projectile {
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed;
    float radius;
    sf::Color color;

    // Tracks the duration a projectile remains active
    sf::Clock lifetime;

    Projectile(sf::Vector2f pos, sf::Vector2f dir, sf::Color color, float spd = 500.f, float rad = 2.f)
            : position(pos), direction(dir), color(color), speed(spd), radius(rad) {

        // Normalize Direction
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        direction = sf::Vector2f(dir.x / length, dir.y / length);
    }
};

// Base class for entities that can move and shoot in the game
class MovableEntity {
public:
    sf::ConvexShape shape;
    sf::Vector2f velocity;
    std::vector<Projectile> projectiles;
    bool explosion = false; // explosion flag
    int lives = 0; // lives of the entity

    MovableEntity() : velocity(sf::Vector2f(0.f, 0.f)) { }

    virtual ~MovableEntity() = default;

    // draws the entity
    virtual void draw(sf::RenderWindow& window) = 0;

    // updates position and velocity of the entity
    virtual void update(float delta) = 0;

    // shoots a projectile, optional target position for the Alien class
    virtual void shoot(std::optional<sf::Vector2f> targetPos = std::nullopt) = 0;

    // makes the entity explode
    virtual void explode() = 0;

protected:
    // initializes the shape of the entity
    virtual void initShape() = 0;

    // keeps all entities in the screen
    template<typename T>
    void wrapAroundScreen(T& t, sf::Vector2f pos) {
        if (pos.x < 0) pos.x += SIZE_X;
        else if (pos.x > SIZE_X) pos.x = 0;

        if (pos.y < 0) pos.y += SIZE_Y;
        else if (pos.y > SIZE_Y) pos.y = 0;

        t.setPosition(pos);
    }
};

#endif //ASTEROIDS_MOVABLEENTITY_H
