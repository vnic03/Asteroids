#include "Movement.h"

#include <algorithm>
#include <cmath>

void moveAsteroids(
    std::vector<std::unique_ptr<Asteroid>>& asteroids,
    float delta
) {
    for (auto& asteroid : asteroids) {
        auto position =
            asteroid->shape.getPosition() + asteroid->velocity * delta;

        position.x = std::fmod(position.x, static_cast<float>(SIZE_X));
        position.y = std::fmod(position.y, static_cast<float>(SIZE_Y));

        if (position.x < 0.f) position.x += SIZE_X;
        if (position.y < 0.f) position.y += SIZE_Y;

        asteroid->shape.setPosition(position);
    }
}

void handlePlayerInput(
    SpaceShip& spaceship,
    float delta,
    sf::Keyboard::Key left,
    sf::Keyboard::Key right,
    sf::Keyboard::Key thrust
) {
    const unsigned controllerId = spaceship.id == 2 ? 1u : 0u;
    const bool connected = sf::Joystick::isConnected(controllerId);

    float turn = 0.f;

    if (sf::Keyboard::isKeyPressed(left)) {
        turn -= 1.f;
    }

    if (sf::Keyboard::isKeyPressed(right)) {
        turn += 1.f;
    }

    bool controllerThrust = false;

    if (connected) {
        const float axis = sf::Joystick::getAxisPosition(
            controllerId,
            sf::Joystick::X
        );

        if (axis < -15.f) turn -= 1.f;
        if (axis > 15.f) turn += 1.f;

        controllerThrust =
            sf::Joystick::isButtonPressed(controllerId, 7);
    }

    turn = std::clamp(turn, -1.f, 1.f);

    spaceship.rotate(turn * ROTATION * delta);
    spaceship.toggleEngine(sf::Keyboard::isKeyPressed(thrust));
    spaceship.toggleEngineC(controllerThrust);
}
