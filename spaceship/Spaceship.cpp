#include "Spaceship.h"
#include "../GameConstants.h"

#include <cstdlib>
#include <cmath>

//
SpaceShip::SpaceShip() {
    lives = 3;
    initShape();
    initEngineShape();
    // invincible at start of the game
    invincible = true;
    invincibleTimer.restart();
}

void SpaceShip::draw(sf::RenderWindow &window) {
    if (!explosion) {
        if (!invincible ||((int)
            (invincibleTimer.getElapsedTime().asSeconds() / 0.2)) % 2 == 0)
        {
            window.draw(shape); // Draw spaceship
            // Draw engine if spaceship is moving
            if (engineOn || engineOnC) window.draw(engine);
        }
        // Draw projectiles as circles
        for (const auto& projectile : projectiles) {
            sf::CircleShape circle(projectile.radius);
            circle.setPosition(projectile.position - sf::Vector2f(
                    projectile.radius, projectile.radius));
            circle.setFillColor(sf::Color::White);
            window.draw(circle);
        }
    } else { // Draw explosion fragments as lines
        for (const auto& fragment : fragments) {
            sf::Vertex line[] = {
                    sf::Vertex(fragment.position, sf::Color::White),
                    sf::Vertex(fragment.position +
                        fragment.direction *
                        10.f, sf::Color::White)
            };
            window.draw(line, 2, sf::Lines);
        }
    }
}

void SpaceShip::update(float delta) {
    // Show Explosion fragments for 2 seconds, then respawn
    if (explosion) {
        for (auto& fragment : fragments) {
            fragment.position += fragment.direction * fragment.speed * delta;
        }
        if (respawnTime.getElapsedTime().asSeconds() > 2) {
            respawn();
        }
        return;
    }
    // invincible for 3 seconds
    if (invincible && invincibleTimer.getElapsedTime().asSeconds() > 3.f) {
        invincible = false;
    }

    if (engineOn || engineOnC) {
        sThrust.play(); // play thrust sound

        float angle = shape.getRotation() - 90;
        // convert angle to radian
        float radian = (angle + 180) * M_PI / 180.0f;

        velocity += sf::Vector2f(
                std::cos(radian) * MOVE_SPEED * delta,
                std::sin(radian) * MOVE_SPEED * delta);

    } else  velocity *= std::pow(DRAG, delta); // apply drag

    sf::Vector2f pos = shape.getPosition() + velocity * delta;

    // Wrap around the screen
    if (pos.x < 0) pos.x += SIZE_X;
    else if (pos.x > SIZE_X) pos.x = 0;

    if (pos.y < 0) pos.y += SIZE_Y;
    else if (pos.y > SIZE_Y) pos.y = 0;

    shape.setPosition(pos);

    float angle = shape.getRotation() * M_PI / 180.f;
    const float engineDistance = 12.f;

    // connects the engine to the spaceship.
    sf::Vector2f engineOffset(
            std::sin(angle) * engineDistance, -std::cos(angle) * engineDistance);

    engine.setPosition(pos + engineOffset);
    engine.setRotation(shape.getRotation());

    auto it = projectiles.begin();
    while (it != projectiles.end()) {
        it->position += it->direction * it->speed * delta;

        // Wrap around the screen
        if (it->position.x < 0) it->position.x += SIZE_X;
        else if (it->position.x > SIZE_X) it->position.x -= SIZE_X;

        if (it->position.y < 0) it->position.y += SIZE_Y;
        else if (it->position.y > SIZE_Y) it->position.y -= SIZE_Y;

        if (it->lifetime.getElapsedTime().asSeconds() > 1.2f) {
            it = projectiles.erase(it);
        } else {
            ++it;
        }
    }
}

void SpaceShip::rotate(float angle) {
    shape.rotate(angle);
}

void SpaceShip::toggleEngine(bool active) {
    engineOn = active;
    updateEngine();
}

void SpaceShip::toggleEngineC(bool active) {
    engineOnC = active;
    updateEngine();
}

void SpaceShip::updateEngine() {
    if (engineOn || engineOnC) {
        float fluctuation = std::rand() % 5 - 2;

        engine.setPoint(0, sf::Vector2f(-6 + fluctuation, 4));
        engine.setPoint(1, sf::Vector2f(6 + fluctuation, 4));
        engine.setPoint(2, sf::Vector2f(0, -8 + fluctuation));
    }
}

// initializes the shape of the spaceship.
void SpaceShip::initShape() {
    shape.setPointCount(5);

    shape.setPoint(0, sf::Vector2f(-5, -6));
    shape.setPoint(1, sf::Vector2f(0, 6));
    shape.setPoint(2, sf::Vector2f(5, -6));
    shape.setPoint(3, sf::Vector2f(4, -5));
    shape.setPoint(4, sf::Vector2f(-4, -5));

    shape.setPosition(SIZE_X / 2.f, SIZE_Y / 2.f);

    shape.setOutlineColor(sf::Color::White);
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineThickness(1.5f);
    shape.setRotation(180);
}

// initializes the shape of the engine.
void SpaceShip::initEngineShape() {
    engine.setPointCount(3);

    engine.setPoint(
            0, sf::Vector2f(-6 + std::rand() % 6 - 3, 4 + std::rand() % 4 - 2));
    engine.setPoint(
            1, sf::Vector2f(6 + std::rand() % 6 - 3, 4 + std::rand() % 4 - 2));
    engine.setPoint(
            2, sf::Vector2f(0, -8 + std::rand() % 6 - 3));

    engine.setFillColor(sf::Color::White);
}

void SpaceShip::explode() {
    sEx.play(); // Play explosion sound
    explosion = true;
    respawnTime.restart();

    fragments.clear();
    sf::Vector2f center = shape.getPosition();

    // Create 15 explosion fragments
    for (int i = 0; i < 15; i++) {
        auto angle = static_cast<float>(std::rand() % 360) * M_PI / 180.f;
        sf::Vector2f direction(std::cos(angle), std::sin(angle));
        auto speed = static_cast<float>(std::rand() % 50 + 50);

        fragments.emplace_back(center, direction, speed);
    }
    lives--; // Decrease lives by 1 after explosion (getting destroyed)
}

void SpaceShip::respawn() {
    explosion = false;
    // Reset spaceship position at the middle
    shape.setPosition(SIZE_X / 2.f, SIZE_Y / 2.f);
    velocity = sf::Vector2f(0, 0);

    invincible = true;
    invincibleTimer.restart();
}

void SpaceShip::shoot(std::optional<sf::Vector2f> pos) {
    // No need for parameter pos, because spaceship shoots in the direction it is facing
    sf::Vector2f startPos = shape.getPosition();

    auto angle = static_cast<float>((shape.getRotation() + 90) * M_PI / 180.0);
    sf::Vector2f direction(std::cos(angle), std::sin(angle));

    projectiles.emplace_back(startPos, direction);

    velocity += direction * (-RECOIL); // recoil

    sFire.play(); // play shooting sound
}
