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
    int spawnLevel = 1;

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

    int fragmentWeight() const {
        return size == AsteroidSize::BIG ? 4 :
               size == AsteroidSize::MEDIUM ? 2 : 1;
    }

    static std::vector<std::unique_ptr<Asteroid>> initAsteroids(
        int amount,
        int level
    ) {
        std::vector<std::unique_ptr<Asteroid>> asteroids;

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

        spawnLevel = level;

        const float base =
            size == AsteroidSize::BIG ? 30.f :
            size == AsteroidSize::MEDIUM ? 20.f : 10.f;

        const float speed = GameRules::asteroidSpeed(
            static_cast<int>(size),
            level
        );

        for (int i = 0; i < points; ++i) {
            const float angle = (i * 360.f / points) * (M_PI / 180.f);

            const float variation = 0.8f +
                static_cast<float>(std::rand() % 40) / 100.f;

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

        const float heading = (std::rand() % 360) * GameRules::Pi / 180.f;

        velocity = {
            std::cos(heading) * speed,
            std::sin(heading) * speed
        };

        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(1.5f);
        shape.setOutlineColor(sf::Color::White);

        collisionRadius = shape.getGlobalBounds().width / 2.f;
    }
};
