#include "Alien.h"
#include "../logic/GameRules.h"
#include <algorithm>

Alien::Alien(AlienSize size) : alienSize(size) {
    float startX = 0.f, startY = 0.f;

    if (size == AlienSize::BIG) {
        shootInterval = GameRules::BigAlienShootInterval;
        lives = GameRules::BigAlienLives;

        const int side = std::rand() % 4;

        switch (side) {
            case 0:
                startX = std::rand() % SIZE_X;
                startY = 0.f;
                break;

            case 1:
                startX = SIZE_X;
                startY = std::rand() % SIZE_Y;
                break;

            case 2:
                startX = std::rand() % SIZE_X;
                startY = SIZE_Y;
                break;

            case 3:
                startX = 0.f;
                startY = std::rand() % SIZE_Y;
                break;
        }

        shape.setPosition(startX, startY);

        const sf::Vector2f center(SIZE_X / 2.f, SIZE_Y / 2.f);
        sf::Vector2f direction = center - shape.getPosition();

        const float length = std::sqrt(
            direction.x * direction.x +
            direction.y * direction.y
        );

        direction /= length;
        velocity = direction * GameRules::BigAlienSpeed;
    } else {
        velocity = {GameRules::SmallAlienSpeed, 0.f};
        shootInterval = GameRules::SmallAlienShootInterval;
        lives = GameRules::SmallAlienLives;
    }

    initShape();

    collisionRadius = shape.getGlobalBounds().width / 2.f;

    updateVisualPositions();
}

void Alien::draw(sf::RenderWindow &window) {
    window.draw(shape);
    window.draw(line);
    window.draw(cockpit);

    drawProjectiles(window);
}

void Alien::update(float delta, bool) {
    if (isDestroyed()) return;

    if (alienSize == AlienSize::SMALL) {
        directionElapsed += delta;

        if (directionElapsed >=
            GameRules::SmallAlienDirectionInterval) {
            directionElapsed = 0.f;

            const float angle = static_cast<float>(
                (std::rand() % 360) * M_PI / 180.0
            );

            velocity = {
                std::cos(angle) * GameRules::SmallAlienSpeed,
                std::sin(angle) * GameRules::SmallAlienSpeed
            };
        }
    }

    wrapAroundScreen(
        shape,
        shape.getPosition() + velocity * delta
    );

    updateVisualPositions();

    for (auto& projectile : projectiles) {
        projectile.position +=
            projectile.direction * projectile.speed * delta;
    }

    std::erase_if(projectiles, [](const Projectile& projectile) {
        const auto position = projectile.position;
        const float radius = projectile.radius;

        return
            projectile.lifetime.getElapsedTime().asSeconds() >
                GameRules::AlienProjectileLifetime ||
            position.x < -radius ||
            position.x > SIZE_X + radius ||
            position.y < -radius ||
            position.y > SIZE_Y + radius;
    });
}

void Alien::shoot(bool sound, std::optional<sf::Vector2f> playerPos)
{
    if (isDestroyed() ||
        shootTimer.getElapsedTime().asSeconds() < shootInterval) {
        return;
    }

    const bool big = alienSize == AlienSize::BIG;

    if (!big && !playerPos) return;

    const auto startPos = shape.getPosition();

    const int shootCount =
        big ? GameRules::BigAlienShotCount
            : GameRules::SmallAlienShotCount;

    const float radius =
        big ? GameRules::BigAlienProjectileRadius
            : GameRules::SmallAlienProjectileRadius;

    const float speed =
        big ? GameRules::BigAlienProjectileSpeed
            : GameRules::SmallAlienProjectileSpeed;

    for (int i = 0; i < shootCount; ++i) {
        sf::Vector2f direction;

        if (big) {
            const float angle = static_cast<float>(
                (std::rand() % 360) * M_PI / 180.0
            );

            direction = {
                std::cos(angle),
                std::sin(angle)
            };
        } else {
            direction = *playerPos - startPos;

            if (direction.x * direction.x +
                direction.y * direction.y < 0.0001f) {
                continue;
            }
        }

        projectiles.emplace_back(
            startPos,
            direction,
            WHITE,
            speed,
            radius
        );
    }

    shootTimer.restart();

    if (sound) playSound(sFire);
}

void Alien::explode(bool sound) {
    if (explosion) return;
    explosion = true;
    lives = 0;
    projectiles.clear();
    if (sound) playSound(sEx);
}

void Alien::updateVisualPositions() {
    line.setPosition(shape.getPosition());

    const float cockpitOffsetY =
        -r_y - cockpitHeight / 2.f + 1.9f;

    cockpit.setPosition(
        shape.getPosition().x,
        shape.getPosition().y + cockpitOffsetY
    );
}

void Alien::initShape() {
    const int NUM_POINTS = 6;
    r = alienSize == AlienSize::BIG ? 24.f : 12.f;
    r_y = r * 0.5f;
    shape.setPointCount(NUM_POINTS);
    float angle = 2 * M_PI / NUM_POINTS;

    for (int i = 0; i < NUM_POINTS; ++i) {
        float x = cosf(angle * i) * r;
        float y = sinf(angle * i) * r_y;
        shape.setPoint(i, sf::Vector2f(x, y));
    }

    float lineLength = 2 * r;
    float thickness = 3.f;
    line.setSize(sf::Vector2f(lineLength, thickness));
    line.setOrigin(lineLength / 2, thickness / 2);

    shape.setOutlineColor(WHITE);
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineThickness(1.5f);

    cockpit.setPointCount(4);

    float cockpitWidthTop = r * 0.6f;
    float cockpitWidthBottom = r * 0.8f;
    cockpitHeight = r_y * 0.6f;

    cockpit.setPoint(0, sf::Vector2f(-cockpitWidthTop / 2, -cockpitHeight));
    cockpit.setPoint(1, sf::Vector2f(cockpitWidthTop / 2, -cockpitHeight));
    cockpit.setPoint(2, sf::Vector2f(cockpitWidthBottom / 2, 0));
    cockpit.setPoint(3, sf::Vector2f(-cockpitWidthBottom / 2, 0));

    cockpit.setFillColor(sf::Color::Transparent);
    cockpit.setOutlineColor(WHITE);
    cockpit.setOutlineThickness(1.5f);
}
