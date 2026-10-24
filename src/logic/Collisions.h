#pragma once

#include <memory>
#include <vector>

#include "../Asteroid.h"
#include "../alien/Alien.h"
#include "../spaceship/Spaceship.h"


bool checkCollision(
    const sf::Vector2f& position1,
    float radius1,
    const sf::Vector2f& position2,
    float radius2
);

bool checkCollision(
    const sf::Shape& object1,
    float radius1,
    const sf::Shape& object2,
    float radius2
);

void resolveAsteroidCollisions(
    std::vector<std::unique_ptr<Asteroid>>& asteroids,
    SpaceShip& spaceship,
    int& score,
    int level,
    bool sound
);

void resolveAlienCollisions(
    std::vector<std::unique_ptr<Alien>>& aliens,
    SpaceShip& spaceship,
    int& score,
    bool sound
);
