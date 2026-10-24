#include "Alien.h"
#include "../logic/GameRules.h"

#include <algorithm>
#include <cmath>

namespace {

sf::Vector2f normalized(sf::Vector2f vector) {
    const float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);

    return length > 0.0001f ? vector / length : sf::Vector2f(0.f, 1.f);
}

sf::Vector2f rotated(sf::Vector2f vector, float degrees) {
    const float angle = degrees * GameRules::Pi / 180.f;

    return {vector.x * std::cos(angle) - vector.y * std::sin(angle),
            vector.x * std::sin(angle) + vector.y * std::cos(angle)};
}

} // namespace

Alien::Alien(AlienSize size, int level, std::optional<sf::Vector2f> position)
    : alienSize(size) {
    const bool big = size == AlienSize::BIG;

    lives = big ? GameRules::BigAlienLives : GameRules::SmallAlienLives;

    shootInterval = GameRules::alienShootInterval(big, level);

    shape.setPosition(position.value_or(sf::Vector2f(48.f, 48.f)));

    const sf::Vector2f center(SIZE_X / 2.f, SIZE_Y / 2.f);

    velocity = normalized(center - shape.getPosition()) *
               (big ? GameRules::BigAlienSpeed : GameRules::SmallAlienSpeed);

    initShape();

    collisionRadius = shape.getGlobalBounds().width / 2.f;

    updateVisualPositions();
}

void Alien::draw(sf::RenderWindow& window) {
    shape.setOutlineColor(WHITE);
    cockpit.setOutlineColor(WHITE);
    line.setFillColor(WHITE);

    window.draw(shape);
    window.draw(line);
    window.draw(cockpit);

    drawProjectiles(window);
}

void Alien::update(float delta, bool) {
    if (isDestroyed())
        return;

    if (alienSize == AlienSize::SMALL) {
        directionElapsed += delta;

        if (directionElapsed >= GameRules::SmallAlienDirectionInterval) {
            directionElapsed -= GameRules::SmallAlienDirectionInterval;

            const float angle = (std::rand() % 360) * GameRules::Pi / 180.f;

            velocity = {std::cos(angle) * GameRules::SmallAlienSpeed,
                        std::sin(angle) * GameRules::SmallAlienSpeed};
        }
    }

    wrapAroundScreen(shape, shape.getPosition() + velocity * delta);

    updateVisualPositions();

    for (auto& projectile : projectiles) {
        projectile.position += projectile.direction * projectile.speed * delta;
    }

    std::erase_if(projectiles, [](const Projectile& projectile) {
        return projectile.lifetime.getElapsedTime().asSeconds() >
                   GameRules::AlienProjectileLifetime ||
               projectile.position.x < -projectile.radius ||
               projectile.position.x > SIZE_X + projectile.radius ||
               projectile.position.y < -projectile.radius ||
               projectile.position.y > SIZE_Y + projectile.radius;
    });
}

void Alien::shoot(bool sound, std::optional<sf::Vector2f> target) {
    if (isDestroyed())
        return;

    if (!target) {
        charging = false;
        shootTimer.restart();
        return;
    }

    if (age.getElapsedTime().asSeconds() < GameRules::AlienSpawnGrace) {
        return;
    }

    if (!charging) {
        if (shootTimer.getElapsedTime().asSeconds() <
            shootInterval - GameRules::AlienChargeSeconds) {
            return;
        }

        aimDirection = normalized(*target - shape.getPosition());

        if (alienSize == AlienSize::BIG) {
            aimDirection = rotated(aimDirection, (volley % 3 - 1) * 15.f);
        }

        charging = true;
        chargeTimer.restart();
        return;
    }

    if (chargeTimer.getElapsedTime().asSeconds() <
        GameRules::AlienChargeSeconds) {
        return;
    }

    const bool big = alienSize == AlienSize::BIG;
    const int count = big ? 3 : 1;

    for (int i = 0; i < count; ++i) {
        projectiles.emplace_back(
            shape.getPosition(),
            rotated(aimDirection, big ? (i - 1) * 25.f : 0.f), WHITE,
            big ? GameRules::BigAlienProjectileSpeed
                : GameRules::SmallAlienProjectileSpeed,
            big ? GameRules::BigAlienProjectileRadius
                : GameRules::SmallAlienProjectileRadius);
    }

    ++volley;
    charging = false;
    shootTimer.restart();

    if (sound)
        playSound(sFire);
}

void Alien::explode(bool sound) {
    if (explosion)
        return;

    explosion = true;
    lives = 0;
    projectiles.clear();

    if (sound) {
        playSound(sEx);
    }
}

void Alien::updateVisualPositions() {
    line.setPosition(shape.getPosition());

    const float cockpitOffsetY = -r_y - cockpitHeight / 2.f + 1.9f;

    cockpit.setPosition(shape.getPosition().x,
                        shape.getPosition().y + cockpitOffsetY);
}

void Alien::initShape() {
    constexpr int pointCount = 6;

    r = alienSize == AlienSize::BIG ? 24.f : 12.f;
    r_y = r * 0.5f;

    shape.setPointCount(pointCount);

    const float angleStep = 2.f * GameRules::Pi / pointCount;

    for (int i = 0; i < pointCount; ++i) {
        shape.setPoint(
            i, {std::cos(angleStep * i) * r, std::sin(angleStep * i) * r_y});
    }

    shape.setOutlineColor(WHITE);
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineThickness(1.5f);

    const float lineLength = 2.f * r;
    constexpr float thickness = 3.f;

    line.setSize({lineLength, thickness});
    line.setOrigin(lineLength / 2.f, thickness / 2.f);
    line.setFillColor(WHITE);

    cockpit.setPointCount(4);

    const float topWidth = r * 0.6f;
    const float bottomWidth = r * 0.8f;
    cockpitHeight = r_y * 0.6f;

    cockpit.setPoint(0, {-topWidth / 2.f, -cockpitHeight});
    cockpit.setPoint(1, {topWidth / 2.f, -cockpitHeight});
    cockpit.setPoint(2, {bottomWidth / 2.f, 0.f});
    cockpit.setPoint(3, {-bottomWidth / 2.f, 0.f});

    cockpit.setFillColor(sf::Color::Transparent);
    cockpit.setOutlineColor(WHITE);
    cockpit.setOutlineThickness(1.5f);
}
