#include "Alien.h"
#include <algorithm>

Alien::Alien(AlienSize size) : alienSize(size) {
    float startX; float startY;

    if (size == AlienSize::BIG) {
        shootInterval = 3.f; // 3 sec
        lives = 10;

        int side = std::rand() % 4;
        switch (side) {
            case 0: // up
                startX = std::rand() % SIZE_X;
                startY = 0;
                break;
            case 1: // right
                startX = SIZE_X;
                startY = std::rand() % SIZE_Y;
                break;
            case 2: // down
                startX = std::rand() % SIZE_X;
                startY = SIZE_Y;
                break;
            case 3: // left
                startX = 0;
                startY = std::rand() % SIZE_Y;
                break;
        }
        shape.setPosition(startX, startY);

        sf::Vector2f center(SIZE_X / 2.f, SIZE_Y / 2.f);
        sf::Vector2f startPos(startX, startY);
        sf::Vector2f direction = center - startPos;

        float length = std::sqrt(direction.x * direction.x +
                direction.y * direction.y);
        direction /= length;
        velocity = direction * 50.f;

    } else { // SMALL
        velocity = sf::Vector2f(100.f, 0.f);
        shootInterval = 1.5f;
        lives = 2;
    }
    initShape();
}

void Alien::draw(sf::RenderWindow &window) {
    line.setPosition(shape.getPosition());
    float cockpitOffsetY = (-r_y) - (cockpitHeight / 2) + 1.9f;
    cockpit.setPosition(shape.getPosition().x, shape.getPosition().y + cockpitOffsetY);

    window.draw(shape);
    window.draw(line);
    window.draw(cockpit);

    for (const auto& p : projectiles) {
        sf::CircleShape circle(p.radius);
        circle.setPosition(p.position - sf::Vector2f(p.radius, p.radius));
        circle.setFillColor(p.color);
        window.draw(circle);
    }
}

void Alien::update(float delta, bool) {
    if (isDestroyed()) return;

    if (alienSize == AlienSize::SMALL) {
        directionElapsed += delta;

        if (directionElapsed >= 0.5f) {
            directionElapsed = 0.f;

            const float angle =
                static_cast<float>((std::rand() % 360) * M_PI / 180.0);

            velocity = sf::Vector2f(
                    std::cos(angle) * 100.f,
                    std::sin(angle) * 100.f
            );
        }
    }

    wrapAroundScreen(shape, shape.getPosition() + velocity * delta);

    for (auto& p : projectiles) {
        p.position += p.direction * p.speed * delta;
    }

    std::erase_if(projectiles, [](const Projectile& projectile) {
        const auto pos = projectile.position;
        const float radius = projectile.radius;

        return projectile.lifetime.getElapsedTime().asSeconds() > 30.f ||
               pos.x < -radius ||
               pos.x > SIZE_X + radius ||
               pos.y < -radius ||
               pos.y > SIZE_Y + radius;
    });
}

void Alien::shoot(bool sound, std::optional<sf::Vector2f> playerPos) {
    if (isDestroyed() ||
        shootTimer.getElapsedTime().asSeconds() < shootInterval) {
        return;
    }

    if (alienSize == AlienSize::SMALL && !playerPos) return;

    const auto startPos = shape.getPosition();

    const int shootCount = alienSize == AlienSize::BIG ? 3 : 1;
    const float radius = alienSize == AlienSize::BIG ? 8.f : 5.f;
    const float speed = alienSize == AlienSize::BIG ? 80.f : 120.f;

    for (int i = 0; i < shootCount; ++i) {
        sf::Vector2f direction;

        if (alienSize == AlienSize::BIG) {
            const float angle =
                static_cast<float>((std::rand() % 360) * M_PI / 180.0);

            direction = sf::Vector2f(std::cos(angle), std::sin(angle));
        } else {
            direction = *playerPos - startPos;

            if (direction.x * direction.x +
                direction.y * direction.y < 0.0001f) {
                continue;
            }
        }

        // Projectile normalizes the direction.
        projectiles.emplace_back(startPos, direction, WHITE, speed, radius);
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
