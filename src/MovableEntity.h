#ifndef ASTEROIDS_MOVABLEENTITY_H
#define ASTEROIDS_MOVABLEENTITY_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <optional>
#include "GameConstants.h"

struct Projectile {
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed;
    float radius;
    sf::Color color;

    SimulationClock lifetime;

    Projectile(
            sf::Vector2f pos,
            sf::Vector2f dir,
            sf::Color color,
            float spd = 500.f,
            float rad = 2.f
        )
            :
                position(pos),
                direction(dir),
                speed(spd),
                radius(rad),
                color(color)
    {
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        direction = length > 0.0001f ? dir / length : sf::Vector2f(0.f, -1.f);
    }
};

class MovableEntity {
public:
    sf::ConvexShape shape;
    sf::Vector2f velocity;
    std::vector<Projectile> projectiles;
    bool explosion = false;
    int lives = 0;

    MovableEntity() : velocity(sf::Vector2f(0.f, 0.f)) { }

    virtual ~MovableEntity() = default;

    virtual void draw(sf::RenderWindow& window) = 0;

    virtual void update(float delta, bool sound) = 0;

    virtual void shoot(bool sound, std::optional<sf::Vector2f> targetPos = std::nullopt) = 0;

    virtual void explode(bool sound) = 0;

protected:
    virtual void initShape() = 0;

    // keeps all entities in the screen
    template<typename T>
    inline void wrapAroundScreen(T& t, sf::Vector2f pos) {
        if (pos.x < 0) pos.x += SIZE_X;
        else if (pos.x > SIZE_X) pos.x = 0;

        if (pos.y < 0) pos.y += SIZE_Y;
        else if (pos.y > SIZE_Y) pos.y = 0;

        t.setPosition(pos);
    }
};

#endif //ASTEROIDS_MOVABLEENTITY_H
