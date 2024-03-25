#ifndef ASTEROID_H
#define ASTEROID_H

#include <SFML/Graphics.hpp>
#include <cstdlib>
#include <cmath>

#include "GameConstants.h"

// Enum class to track the size of the asteroid
enum class AsteroidSize {
    BIG,
    MEDIUM,
    SMALL
};

// Class to represent an asteroid
class Asteroid {
public:
    sf::ConvexShape shape;
    sf::Vector2f velocity;
    AsteroidSize size;
    int hp;

    // Different Size -> Different HP
    // Each hp is one hit
    explicit Asteroid(AsteroidSize size) : size(size) {
        switch (size) {
            case AsteroidSize::BIG: hp = 4;
                break;
            case AsteroidSize::MEDIUM: hp = 2;
                break;
            case AsteroidSize::SMALL: hp = 1;
                break;
        }
    }

    // Initializes a vector of unique Asteroid objects.
    static std::vector<std::unique_ptr<Asteroid>> initAsteroids(
            int amount, int &level)
    {
        std::vector<std::unique_ptr<Asteroid>> asteroids;

        // Increase the amount of asteroids for higher levels
        // Starts at 4, Stops at 20
        if (level > 3 && amount <= 20) amount += 1;

        // Create the asteroids
        for (int i = 0; i < amount; i++) {
            // Randomly choose a size
            auto size = static_cast<AsteroidSize>(std::rand() % 3);
            auto asteroid = std::make_unique<Asteroid>(size);
            asteroid->initShape(level);
            asteroids.push_back(std::move(asteroid));
        }
        return asteroids;
    }

    void initShape(int &level) {
        // Randomize the number of points
        const int points = 12 + std::rand() % 6;
        shape.setPointCount(points);

        // Different Size -> Different Speed
        float base; // Size of the asteroid
        float speed;
        switch (size) {
            case AsteroidSize::BIG:
                base = 30.0f;
                speed = 50.f;
                break;
            case AsteroidSize::MEDIUM:
                base = 20.0f;
                speed = 120.f;
                break;
            case AsteroidSize::SMALL:
                base = 10.0f;
                speed = 200.f;
                break;
        }

        float speedLevel; // Speed multiplier based on level
        if (level <= 10) {
            // Up to level 10: Increase by 10% per level
            speedLevel = 1.f + (level - 1) * 0.1f;
        } else if (level <= 20) {
            // From Level 11 to 20: Increase starts at 1.9 and increases by 5% per level
            speedLevel = 1.9f + (level - 10) * 0.05f;
        } else {
            // From level 21: Increase is fixed at 2.4 plus 0.1 per level above 20
            speedLevel = 2.4f + (level - 20) * 0.01f;
        }
        speed *= speedLevel;

        // Create the asteroid shape
        for (int i = 0; i < points; i++) {
            float angle = (i * 360.0f / points) * (M_PI / 180.0f);
            float variation = 0.8f + static_cast<float>
                    (std::rand() % 40) / 100.0f;

            float radius = base * variation;

            sf::Vector2f point = sf::Vector2f(
                    radius * std::cos(angle), radius * std::sin(angle));

            shape.setPoint(i, point);
        }
        shape.setPosition(std::rand() % SIZE_X, std::rand() % SIZE_Y);

        // Randomize the velocity
        float speedX = (std::rand() % 100 - 50) / 50.0f * speed;
        float speedY = (std::rand() % 100 - 50) / 50.0f * speed;
        velocity = sf::Vector2f(speedX, speedY);

        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineThickness(1.5f);
        shape.setOutlineColor(sf::Color::White);
    }
};

#endif // ASTEROID_H