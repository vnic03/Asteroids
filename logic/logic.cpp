#include "logic.h"

int runGame(sf::RenderWindow &window, const sf::Font& font, GameState& state) {
    bool paused = false;

    sf::Clock clock; // Game loop clock
    sf::Clock beat; // Beat clock

    // Timer for sound speed to make the beat faster over time
    sf::Clock soundSpeedTimer;

    // Seed for random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    int score = 0;
    int level = 1;
    float initialBI = 1.f; // Initial beat interval
    float beatInterval = initialBI;

    // Spaceship, asteroids, and aliens
    SpaceShip spaceship;
    auto asteroids = Asteroid::initAsteroids(10, level);
    std::vector<std::unique_ptr<Alien>> aliens;

    // Manages alien spawn timing, count, and interval
    sf::Clock alienSpawnTimer;
    int aliensSpawned = 0;
    float nextAlienSpawnTime;

    // Random time for first alien spawn between 8 and 12 seconds
    nextAlienSpawnTime = (std::rand() % 5) + 8;

    while (window.isOpen()) {
        // Time since last frame
        sf::Time elapsed = clock.restart();
        float delta = elapsed.asSeconds();

        // Stop alien sounds after 1.8 seconds
        if (alienBTimer.getElapsedTime().asSeconds() > 1.8) {
            sAlienB.stop();
            sAlienB.setLoop(false);
        }
        if (alienSTimer.getElapsedTime().asSeconds() > 1.8) {
            sAlienS.stop();
            sAlienS.setLoop(false);
        }

        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Space) {
                    if (!paused) {
                        spaceship.keyClicked = false;
                    }
                } else if (event.key.code == sf::Keyboard::P) {
                    paused = !paused;
                }
            }
        }
        if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == sf::Keyboard::Space) {
                spaceship.keyClicked = false;
            }
        }
        if (!paused) {
            if (asteroids.empty() && aliens.empty()) {
                level++;
                beatInterval = initialBI;
                soundSpeedTimer.restart();
                if (level % 2 == 0) {
                    spaceship.lives++;
                }
                spaceship.invincible = true;
                spaceship.invincibleTimer.restart();
                asteroids = Asteroid::initAsteroids(10, level);
                aliensSpawned = 0;
                alienSpawnTimer.restart();
                nextAlienSpawnTime = (std::rand() % 5) + 8;
            }
            if (aliensSpawned < 2 &&
                alienSpawnTimer.getElapsedTime().asSeconds() > nextAlienSpawnTime)
            {
                AlienSize size = (aliensSpawned % 2 == 0) ?
                                 AlienSize::BIG : AlienSize::SMALL;
                addAlien(aliens, size, delta);

                aliensSpawned++;
                alienSpawnTimer.restart();
                if (aliensSpawned < 2) {
                    nextAlienSpawnTime = (std::rand() % 5) + 8;
                }
            }

            if (!spaceship.explosion) {
                controllerInput(spaceship, paused, delta);

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
                    spaceship.rotate(-ROTATION * delta);
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
                    spaceship.rotate(ROTATION * delta);
                }
                spaceship.toggleEngine(
                        sf::Keyboard::isKeyPressed(sf::Keyboard::W)
                        || sf::Keyboard::isKeyPressed(sf::Keyboard::Up));

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) &&
                    !spaceship.keyClicked) {
                    spaceship.shoot();
                    spaceship.keyClicked = true;
                }
            }

            spaceship.update(delta);
            window.clear(sf::Color::Black);

            spaceship.draw(window);

            updateAsteroids(asteroids, spaceship, delta, score, level);
            updateAliens(aliens, spaceship, delta, score);

            for (auto &asteroid: asteroids) {
                window.draw(asteroid->shape);
            }
            for (auto& alien : aliens) {
                alien->draw(window);
            }

            drawScore(window, score);
            drawLives(window, spaceship);

            if (spaceship.lives <= 0) {
                state = GameState::GAME_OVER;
                return score; // Return final score
            }
        } else {
            // PAUSE - Display "Pause" text
            sf::Text pauseText("Pause", font, 40);
            pauseText.setFillColor(sf::Color::White);
            pauseText.setPosition(
                    SIZE_X / 2.f - pauseText.getLocalBounds().width / 2,
                    SIZE_Y / 2.f);
            window.draw(pauseText);
        }

        // Every 5 seconds the beat gets faster by 10%
        if (soundSpeedTimer.getElapsedTime().asSeconds() >= 5) {
            beatInterval *= 0.9f;
            soundSpeedTimer.restart();
        }

        if (beat.getElapsedTime().asSeconds() > beatInterval) {
            playBeat(sBeat1, sBeat2);
            beat.restart();
        }

        window.display();
    }
    return score;
}

bool checkCollision(const sf::Shape& object1, float radius1,
                    const sf::Shape& object2, float radius2)
{
    sf::Vector2f position1 = object1.getPosition();
    sf::Vector2f position2 = object2.getPosition();

    float dx = position1.x - position2.x;
    float dy = position1.y - position2.y;

    float distance = std::sqrt(dx * dx + dy * dy);

    return distance < (radius1 + radius2);
}

bool checkCollision(const sf::Vector2f& point1, float radius1,
                    const sf::Vector2f& point2, float radius2) {
    float dx = point1.x - point2.x;
    float dy = point1.y - point2.y;

    float distance = std::sqrt(dx * dx + dy * dy);
    return distance < (radius1 + radius2);
}

void updateAsteroids(std::vector<std::unique_ptr<Asteroid>>& asteroids,
                     SpaceShip& spaceship, float delta, int &score, int &level)
{
    for (auto& asteroid : asteroids) {
        asteroid->shape.move(asteroid->velocity * delta);

        sf::Vector2f asteroidPos = asteroid->shape.getPosition();

        if (asteroidPos.x < 0) asteroidPos.x += SIZE_X;
        else if (asteroidPos.x > SIZE_X) asteroidPos.x -= SIZE_X;

        if (asteroidPos.y < 0) asteroidPos.y += SIZE_Y;
        else if (asteroidPos.y > SIZE_Y) asteroidPos.y -= SIZE_Y;

        asteroid->shape.setPosition(asteroidPos);

        float asteroidRadius = asteroid->shape.getGlobalBounds().width / 2;

        if (!spaceship.explosion && !spaceship.invincible &&
            checkCollision(spaceship.shape,spaceship.radius,
                           asteroid->shape, asteroidRadius))
        {
            spaceship.explode();
        }
    }

    auto& projectiles = spaceship.projectiles;

    for (auto p_it = projectiles.begin(); p_it != projectiles.end(); ) {
        bool hit = false;
        sf::Vector2f projectileEnd = p_it->position + p_it->direction * 10.f;

        for (auto a_it = asteroids.begin(); a_it != asteroids.end();) {
            float aRadius = (*a_it)->shape.getGlobalBounds().width / 2;

            if (checkCollision(p_it->position, p_it->radius,
                               (*a_it)->shape.getPosition(), aRadius) ||
                checkCollision(projectileEnd, p_it->radius,
                               (*a_it)->shape.getPosition(), aRadius))
            {
                --(*a_it)->hp;

                if ((*a_it)->hp <= 0) {
                    AsteroidSize size = (*a_it)->size;

                    switch (size) {
                        case AsteroidSize::BIG:
                            score += 20, sSmallEx.play();
                            break;
                        case AsteroidSize::MEDIUM:
                            score += 50, sMediumEx.play();
                            break;
                        case AsteroidSize::SMALL:
                            score += 100, sBigEx.play();
                            break;
                    }
                    sf::Vector2f oldPosition = (*a_it)->shape.getPosition();

                    a_it = asteroids.erase(a_it);

                    if (size == AsteroidSize::BIG) {
                        for (int i = 0; i < 2; i++) {
                            auto newAsteroid = std::make_unique<Asteroid>(
                                    AsteroidSize::MEDIUM);
                            newAsteroid->initShape(level);
                            newAsteroid->shape.setPosition(oldPosition +
                                                    sf::Vector2f(i * 10, 0));
                            asteroids.push_back(std::move(newAsteroid));
                        }
                    } else if (size == AsteroidSize::MEDIUM) {
                        for (int i = 0; i < 2; i++) {
                            auto newAsteroid = std::make_unique<Asteroid>(
                                    AsteroidSize::SMALL);
                            newAsteroid->initShape(level);
                            newAsteroid->shape.setPosition(oldPosition +
                                                        sf::Vector2f(i * 10, 0));
                            asteroids.push_back(std::move(newAsteroid));
                        }
                    }
                    hit = true;
                    break;
                }
                hit = true;
                break;
            } else {
                ++a_it;
            }
        }
        if (hit) {
            p_it = projectiles.erase(p_it);
        } else {
            ++p_it;
        }
    }
}

void updateAliens(std::vector<std::unique_ptr<Alien>>& aliens,
                  SpaceShip& spaceship, float delta, int &score)
{
    for (auto& alien : aliens) {
        alien->projectiles.erase(
                std::remove_if(
                        alien->projectiles.begin(),
                        alien->projectiles.end(),
                        [&spaceship](const Projectile& p) {
                            return checkCollision(p.position, p.radius,
                                                  spaceship.shape.getPosition(),
                                                  spaceship.radius);
                        }
                ),
                alien->projectiles.end()
        );
        alien->update(delta);

        if (!spaceship.explosion && !spaceship.invincible &&
            checkCollision(spaceship.shape, spaceship.radius,
                           alien->shape, alien->shape.getGlobalBounds().width / 2))
        {
            spaceship.explode();
            alien->explode();
        }

        auto& projectiles = spaceship.projectiles;

        for (auto p_it = projectiles.begin(); p_it != projectiles.end();) {
            bool hit = false;

            if (!spaceship.explosion && checkCollision(p_it->position, p_it->radius,
                               alien->shape.getPosition(),
                               alien->shape.getGlobalBounds().width / 2))
            {
                hit = true;
                alien->takeDamage();
                p_it = projectiles.erase(p_it);

                score += (alien->alienSize == AlienSize::BIG) ? 200 : 1000;
            } else {
                ++p_it;
            }
        }
        for (auto& p : alien->projectiles) {
            if (!spaceship.explosion && !spaceship.invincible &&
                checkCollision(p.position, p.radius,
                               spaceship.shape.getPosition(), spaceship.radius))
            {
                spaceship.explode();
                break;
            }
        }
    }
    aliens.erase(std::remove_if(aliens.begin(), aliens.end(),
                                [] (const std::unique_ptr<Alien>& alien) {
                                    return alien->isDestroyed();
    }),
                 aliens.end());

    sf::Vector2f shipPos = spaceship.shape.getPosition();
    for (auto& alien : aliens) {
        alien->shoot(shipPos);
    }
}

void addAlien(std::vector<std::unique_ptr<Alien>>& aliens, AlienSize size,
              float delta) {
    auto alien = std::make_unique<Alien>(size);
    alien->update(delta);
    aliens.push_back(std::move(alien));

    if (size == AlienSize::BIG) {
        sAlienB.setLoop(true);
        sAlienB.play();
        alienBTimer.restart();
    }
    if (size == AlienSize::SMALL) {
        sAlienS.setLoop(true);
        sAlienS.play();
        alienSTimer.restart();
    }
}

void drawScore(sf::RenderWindow &window, int score) {
    std::string s = std::to_string(score);
    s = std::string(5 - s.length(), '0') + s;

    sf::Vector2f pos(SIZE_X - (100 + SCALE * 4), 20);

    for (char digit : s) {
        int n = digit - '0';
        drawNumber(window, n, pos);
        pos.x += SCALE * 1.2f;
    }
}

void drawNumber(sf::RenderWindow &window, int n, sf::Vector2f pos) {
    const auto& vectors = NUMBER_LINES[n];

    for (size_t i = 0; i < vectors.size(); i += 2) {
        sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(pos.x + vectors[i].x * SCALE,
                                        pos.y +vectors[i].y * SCALE)),
                sf::Vertex(sf::Vector2f(pos.x + vectors[i + 1].x * SCALE,
                                        pos.y + vectors[i + 1].y * SCALE))
        };
        line[0].color = sf::Color::White;
        line[1].color = sf::Color::White;

        window.draw(line, 2, sf::Lines);
    }
}

void drawLives(sf::RenderWindow &window, const SpaceShip& spaceship) {
    sf::ConvexShape shape;

    shape.setPointCount(5);

    shape.setPoint(0, sf::Vector2f(-10, -12));
    shape.setPoint(1, sf::Vector2f(0, 10));
    shape.setPoint(2, sf::Vector2f(10, -12));
    shape.setPoint(3, sf::Vector2f(8, -10));
    shape.setPoint(4, sf::Vector2f(-8, -10));

    shape.setOutlineColor(sf::Color::White);
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineThickness(1.0f);
    shape.setRotation(180);

    for (int i = 0; i < spaceship.lives; ++i) {
        shape.setPosition(25 + i * 40, 28);
        window.draw(shape);
    }
}

// shoot button pressed ?
bool shot = false;

void controllerInput(SpaceShip& spaceship, bool& paused, float delta) {
    if (sf::Joystick::isConnected(0)) {
        // left stick
        float x = sf::Joystick::getAxisPosition(0, sf::Joystick::X);

        if (x < -15 && !paused) {
            spaceship.rotate(-ROTATION * delta);
        } else if (x > 15) {
            spaceship.rotate(ROTATION * delta);
        }

        // These are PS4 specific rn
        spaceship.toggleEngineC(sf::Joystick::isButtonPressed(0, 7)); // R2-Button

        bool xButtonPressed = sf::Joystick::isButtonPressed(0, 0); // X-Button
        if (xButtonPressed && !shot) {
            spaceship.shoot();
            shot = true;
        }
        if (!xButtonPressed) {
            shot = false;
        }
    }
}
