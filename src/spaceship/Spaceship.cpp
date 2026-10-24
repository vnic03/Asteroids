#include "Spaceship.h"
#include "../logic/GameRules.h"
#include <algorithm>

SpaceShip::SpaceShip(int id) : id(id) {
    lives = GameRules::StartingLives;
    initShape();
    initEngineShape();
    invincible = true;
    invincibleTimer.restart();
}

void SpaceShip::draw(sf::RenderWindow& window) {
    if (!explosion) {
        const bool visible =
            !invincible ||
            static_cast<int>(invincibleTimer.getElapsedTime().asSeconds() /
                             0.2f) %
                    2 ==
                0;

        if (visible) {
            window.draw(shape);

            if (engineOn || engineOnC) {
                window.draw(engine);
            }
        }

        drawProjectiles(window);

    } else {
        for (const auto& fragment : fragments) {
            sf::Vertex line[] = {
                sf::Vertex(fragment.position, WHITE),
                sf::Vertex(fragment.position + fragment.direction * 10.f,
                           fragment.color)};

            window.draw(line, 2, sf::Lines);
        }
    }
}

void SpaceShip::update(float delta, bool) {
    if (explosion) {
        for (auto& fragment : fragments) {
            fragment.position += fragment.direction * fragment.speed * delta;
        }

        if (respawnTime.getElapsedTime().asSeconds() >
            GameRules::RespawnSeconds) {
            respawn();
        }

        return;
    }

    if (invincible && invincibleTimer.getElapsedTime().asSeconds() >
                          GameRules::InvincibleSeconds) {
        invincible = false;
    }

    if (engineOn || engineOnC) {
        const float angle = shape.getRotation() - 90.f;
        const float radians = (angle + 180.f) * M_PI / 180.f;

        velocity += sf::Vector2f(std::cos(radians) * MOVE_SPEED * delta,
                                 std::sin(radians) * MOVE_SPEED * delta);
    } else {
        velocity *= std::pow(DRAG, delta);
    }

    wrapAroundScreen(shape, shape.getPosition() + velocity * delta);

    const float angle = shape.getRotation() * M_PI / 180.f;
    const float engineDistance = 12.f;

    const sf::Vector2f engineOffset(std::sin(angle) * engineDistance,
                                    -std::cos(angle) * engineDistance);

    engine.setPosition(shape.getPosition() + engineOffset);
    engine.setRotation(shape.getRotation());

    for (auto& projectile : projectiles) {
        projectile.position += projectile.direction * projectile.speed * delta;

        if (projectile.position.x < 0.f) {
            projectile.position.x += SIZE_X;
        } else if (projectile.position.x > SIZE_X) {
            projectile.position.x -= SIZE_X;
        }

        if (projectile.position.y < 0.f) {
            projectile.position.y += SIZE_Y;
        } else if (projectile.position.y > SIZE_Y) {
            projectile.position.y -= SIZE_Y;
        }
    }

    std::erase_if(projectiles, [](const Projectile& projectile) {
        return projectile.lifetime.getElapsedTime().asSeconds() >
               GameRules::PlayerProjectileLifetime;
    });
}

void SpaceShip::rotate(float angle) { shape.rotate(angle); }

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

void SpaceShip::initShape() {
    shape.setPointCount(5);

    shape.setPoint(0, sf::Vector2f(-5, -6));
    shape.setPoint(1, sf::Vector2f(0, 6));
    shape.setPoint(2, sf::Vector2f(5, -6));
    shape.setPoint(3, sf::Vector2f(4, -5));
    shape.setPoint(4, sf::Vector2f(-4, -5));

    shape.setPosition(SIZE_X / 2.f, SIZE_Y / 2.f);

    shape.setOutlineColor(handleColor());

    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineThickness(1.5f);
    shape.setRotation(180);
}

void SpaceShip::initEngineShape() {
    engine.setPointCount(3);

    engine.setPoint(
        0, sf::Vector2f(-6 + std::rand() % 6 - 3, 4 + std::rand() % 4 - 2));
    engine.setPoint(
        1, sf::Vector2f(6 + std::rand() % 6 - 3, 4 + std::rand() % 4 - 2));
    engine.setPoint(2, sf::Vector2f(0, -8 + std::rand() % 6 - 3));

    engine.setFillColor(handleColor());
}

void SpaceShip::explode(bool sound) {
    if (explosion || lives <= 0)
        return;
    projectiles.clear();
    engineOn = false;
    engineOnC = false;

    if (sound)
        playSound(sEx);

    explosion = true;
    respawnTime.restart();

    fragments.clear();
    sf::Vector2f center = shape.getPosition();

    for (int i = 0; i < 15; i++) {
        auto angle = static_cast<float>(std::rand() % 360) * M_PI / 180.f;
        sf::Vector2f direction(std::cos(angle), std::sin(angle));
        auto speed = static_cast<float>(std::rand() % 50 + 50);

        fragments.emplace_back(center, direction, speed, handleColor());
    }
    lives--;
}

void SpaceShip::respawn() {
    if (lives > 0) {
        explosion = false;
        // Reset spaceship position at the middle
        shape.setPosition(SIZE_X / 2.f, SIZE_Y / 2.f);
        velocity = sf::Vector2f(0, 0);

        invincible = true;
        invincibleTimer.restart();
    }
}

void SpaceShip::shoot(bool sound, std::optional<sf::Vector2f>) {
    if (explosion || lives <= 0)
        return;

    const auto startPos = shape.getPosition();
    const float angle =
        static_cast<float>((shape.getRotation() + 90.f) * M_PI / 180.0);

    const sf::Vector2f direction(std::cos(angle), std::sin(angle));

    projectiles.emplace_back(startPos, direction, handleColor());
    velocity -= direction * RECOIL;

    if (sound)
        playSound(sFire);
}
