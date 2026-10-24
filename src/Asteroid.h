#pragma once

#include <SFML/Graphics.hpp>

#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>

#include "GameConstants.h"
#include "logic/GameRules.h"

enum class AsteroidSize {
    BIG,
    MEDIUM,
    SMALL
};

class Asteroid {
public:
    sf::ConvexShape shape;
    sf::Vector2f velocity;

    AsteroidSize size;
    int hp;

    float collisionRadius = 0.f;

    explicit Asteroid(AsteroidSize size)
        : size(size) {
        switch (size) {
            case AsteroidSize::BIG:
                hp = 4;
                break;

            case AsteroidSize::MEDIUM:
                hp = 2;
                break;

            case AsteroidSize::SMALL:
                hp = 1;
                break;
        }
    }

    static std::vector<std::unique_ptr<Asteroid>> initAsteroids(
        int amount,
        int level
    ) {
        std::vector<std::unique_ptr<Asteroid>> asteroids;

        amount = GameRules::asteroidAmount(amount, level);

        if (amount > 0) {
            asteroids.reserve(static_cast<std::size_t>(amount));
        }

        for (int i = 0; i < amount; ++i) {
            const auto size =
                static_cast<AsteroidSize>(std::rand() % 3);

            auto asteroid = std::make_unique<Asteroid>(size);
            asteroid->initShape(level);

            asteroids.push_back(std::move(asteroid));
        }

        return asteroids;
    }

    void initShape(int level) {
        const int points = 12 + std::rand() % 6;
        shape.setPointCount(points);

        float base = 0.f;
        float speed = 0.f;

        switch (size) {
            case AsteroidSize::BIG:
                base = 30.f;
                speed = GameRules::BigAsteroidSpeed;
                break;

            case AsteroidSize::MEDIUM:
                base = 20.f;
                speed = GameRules::MediumAsteroidSpeed;
                break;

            case AsteroidSize::SMALL:
                base = 10.f;
                speed = GameRules::SmallAsteroidSpeed;
                break;
        }

        speed *= GameRules::asteroidSpeedMultiplier(level);

        for (int i = 0; i < points; ++i) {
            const float angle =
                (i * 360.f / points) * (M_PI / 180.f);

            const float variation =
                0.8f + static_cast<float>(std::rand() % 40) / 100.f;

            const float radius = base * variation;

            shape.setPoint(i, {
                radius * std::cos(angle),
                radius * std::sin(angle)
            });
        }

        shape.setPosition(
            std::rand() % SIZE_X,
            std::rand() % SIZE_Y
        );

        const float speedX =
            (std::rand() % 100 - 50) / 50.f * speed;

        const float speedY =
            (std::rand() % 100 - 50) / 50.f * speed;

        velocity = {speedX, speedY};

        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(1.5f);
        shape.setOutlineColor(sf::Color::White);

        collisionRadius = shape.getGlobalBounds().width / 2.f;
    }
};
