#pragma once

#include <memory>
#include <vector>

#include "../Asteroid.h"
#include "../spaceship/Spaceship.h"

void moveAsteroids(std::vector<std::unique_ptr<Asteroid>>& asteroids,
                   float delta);

void handlePlayerInput(SpaceShip& spaceship, float delta,
                       sf::Keyboard::Key left, sf::Keyboard::Key right,
                       sf::Keyboard::Key thrust);
