#include "Collisions.h"

#include <limits>

namespace {

void addScore(int& score, int amount) {
    const int maximum = std::numeric_limits<int>::max();

    if (score > maximum - amount) {
        score = maximum;
    } else {
        score += amount;
    }
}

void awardAsteroidScore(
    AsteroidSize size,
    int& score,
    bool sound
) {
    switch (size) {
        case AsteroidSize::BIG:
            addScore(score, 20);
            if (sound) playSound(sBigEx);
            break;

        case AsteroidSize::MEDIUM:
            addScore(score, 50);
            if (sound) playSound(sMediumEx);
            break;

        case AsteroidSize::SMALL:
            addScore(score, 100);
            if (sound) playSound(sSmallEx);
            break;
    }
}

void spawnAsteroidFragments(
    std::vector<std::unique_ptr<Asteroid>>& asteroids,
    AsteroidSize destroyedSize,
    sf::Vector2f position,
    int level
) {
    if (destroyedSize == AsteroidSize::SMALL) {
        return;
    }

    const AsteroidSize fragmentSize =
        destroyedSize == AsteroidSize::BIG
            ? AsteroidSize::MEDIUM
            : AsteroidSize::SMALL;

    for (int i = 0; i < 2; ++i) {
        auto fragment = std::make_unique<Asteroid>(fragmentSize);
        fragment->initShape(level);
        fragment->shape.setPosition(
            position + sf::Vector2f(static_cast<float>(i * 10), 0.f)
        );

        asteroids.push_back(std::move(fragment));
    }
}

} // namespace

bool checkCollision(
    const sf::Vector2f& position1,
    float radius1,
    const sf::Vector2f& position2,
    float radius2
) {
    const float dx = position1.x - position2.x;
    const float dy = position1.y - position2.y;
    const float radiusSum = radius1 + radius2;

    return dx * dx + dy * dy < radiusSum * radiusSum;
}

bool checkCollision(
    const sf::Shape& object1,
    float radius1,
    const sf::Shape& object2,
    float radius2
) {
    return checkCollision(
        object1.getPosition(),
        radius1,
        object2.getPosition(),
        radius2
    );
}

void resolveAsteroidCollisions(
    std::vector<std::unique_ptr<Asteroid>>& asteroids,
    SpaceShip& spaceship,
    int& score,
    int level,
    bool sound
) {
    if (spaceship.explosion || spaceship.lives <= 0) {
        return;
    }

    if (!spaceship.invincible) {
        for (const auto& asteroid : asteroids) {
            if (checkCollision(
                    spaceship.shape,
                    spaceship.radius,
                    asteroid->shape,
                    asteroid->collisionRadius
                )) {
                spaceship.explode(sound);

                // explode() clears the player's projectiles.
                return;
            }
        }
    }

    auto& projectiles = spaceship.projectiles;

    for (auto projectile = projectiles.begin();
         projectile != projectiles.end();) {
        bool hit = false;

        const sf::Vector2f projectileEnd =
            projectile->position + projectile->direction * 10.f;

        for (auto asteroid = asteroids.begin();
             asteroid != asteroids.end();
             ++asteroid) {
            const auto position = (*asteroid)->shape.getPosition();
            const float radius = (*asteroid)->collisionRadius;

            const bool overlaps =
                checkCollision(
                    projectile->position,
                    projectile->radius,
                    position,
                    radius
                ) ||
                checkCollision(
                    projectileEnd,
                    projectile->radius,
                    position,
                    radius
                );

            if (!overlaps) {
                continue;
            }

            hit = true;
            --(*asteroid)->hp;

            if ((*asteroid)->hp <= 0) {
                const AsteroidSize size = (*asteroid)->size;

                awardAsteroidScore(size, score, sound);
                asteroids.erase(asteroid);

                // No asteroid iterator is used after this mutation.
                spawnAsteroidFragments(
                    asteroids,
                    size,
                    position,
                    level
                );
            }

            break;
        }

        if (hit) {
            projectile = projectiles.erase(projectile);
        } else {
            ++projectile;
        }
    }
}

void resolveAlienCollisions(
    std::vector<std::unique_ptr<Alien>>& aliens,
    SpaceShip& spaceship,
    int& score,
    bool sound
) {
    if (spaceship.explosion || spaceship.lives <= 0) {
        return;
    }

    for (auto& alien : aliens) {
        if (alien->isDestroyed()) {
            continue;
        }

        const float alienRadius = alien->collisionRadius;

        if (!spaceship.invincible &&
            checkCollision(
                spaceship.shape,
                spaceship.radius,
                alien->shape,
                alienRadius
            )) {
            spaceship.explode(sound);
            alien->explode(sound);
            return;
        }

        auto& playerShots = spaceship.projectiles;

        for (auto shot = playerShots.begin();
             shot != playerShots.end() && !alien->isDestroyed();) {
            if (!checkCollision(
                    shot->position,
                    shot->radius,
                    alien->shape.getPosition(),
                    alienRadius
                )) {
                ++shot;
                continue;
            }

            shot = playerShots.erase(shot);
            alien->takeDamage(sound);

            if (alien->isDestroyed()) {
                addScore(
                    score,
                    alien->alienSize == AlienSize::BIG ? 200 : 1000
                );
            }
        }

        if (alien->isDestroyed()) {
            continue;
        }

        auto& enemyShots = alien->projectiles;

        for (auto shot = enemyShots.begin();
             shot != enemyShots.end();) {
            if (!checkCollision(
                    shot->position,
                    shot->radius,
                    spaceship.shape.getPosition(),
                    spaceship.radius
                )) {
                ++shot;
                continue;
            }

            shot = enemyShots.erase(shot);

            if (!spaceship.invincible) {
                spaceship.explode(sound);
                return;
            }
        }
    }
}
