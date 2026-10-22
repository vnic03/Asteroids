#include "logic.h"
#include <algorithm>
#include <ctime>
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

void moveAsteroids(
    std::vector<std::unique_ptr<Asteroid>>& asteroids,
    float delta
) {
    for (auto& asteroid : asteroids) {
        auto pos = asteroid->shape.getPosition() + asteroid->velocity * delta;

        pos.x = std::fmod(pos.x, static_cast<float>(SIZE_X));
        pos.y = std::fmod(pos.y, static_cast<float>(SIZE_Y));

        if (pos.x < 0.f) pos.x += SIZE_X;
        if (pos.y < 0.f) pos.y += SIZE_Y;

        asteroid->shape.setPosition(pos);
    }
}

}

std::pair<int, int> runGame(
    sf::RenderWindow& window,
    const sf::Font& font,
    GameState& state,
    int& score,
    int& score2
) {
    bool paused = false;
    bool sound = soundEnabled;

    // Only this clock measures real time.
    sf::Clock frameClock;

    SimulationClock beat;
    SimulationClock soundSpeedTimer;
    SimulationClock alienSpawnTimer;

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    int level = 1;
    float beatInterval = 1.f;

    SpaceShip spaceships[] = {
        SpaceShip(0),
        SpaceShip(1),
        SpaceShip(2)
    };

    const bool coop = state == GameState::COOP_RUNNING;
    const int firstPlayer = coop ? 1 : 0;
    const int endPlayer = coop ? 3 : 1;

    const int asteroidAmount = coop ? 12 : 10;
    auto asteroids = Asteroid::initAsteroids(asteroidAmount, level);

    std::vector<std::unique_ptr<Alien>> aliens;
    int aliensSpawned = 0;

    float nextAlienSpawnTime =
        static_cast<float>((std::rand() % 5) + 8);

    auto result = [&]() {
        stopGameSounds();
        return std::make_pair(score, coop ? score2 : -1);
    };

    while (window.isOpen()) {
        // Prevent a stall from producing one enormous movement step.
        const float delta =
            std::min(frameClock.restart().asSeconds(), 0.05f);

        const bool wasPaused = paused;

        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed &&
                 event.key.code == sf::Keyboard::Escape)) {
                window.close();
                return result();
            }

            if (event.type == sf::Event::LostFocus) {
                paused = true;
                stopGameSounds();
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == PAUSE) {
                    paused = !paused;

                    if (paused) {
                        stopGameSounds();
                    }
                } else if (event.key.code == SOUND) {
                    sound = !sound;
                    soundEnabled = sound;

                    if (!sound) {
                        stopGameSounds();
                    }
                }
            }
            if (paused || wasPaused || !window.hasFocus()) {
                continue;
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == SHOOT) {
                    spaceships[coop ? 1 : 0].shoot(sound);
                } else if (coop && event.key.code == COOP_SHOOT) {
                    spaceships[2].shoot(sound);
                }
            }

            if (event.type == sf::Event::JoystickButtonPressed) {
                // Existing PS4 mapping: X is button 0.
                if (event.joystickButton.button == 0) {
                    if (event.joystickButton.joystickId == 0) {
                        spaceships[coop ? 1 : 0].shoot(sound);
                    } else if (coop && event.joystickButton.joystickId == 1) {
                        spaceships[2].shoot(sound);
                    }
                }
            }
        }

        // Skip simulation on the frame that resumes a paused game.
        if (!paused && !wasPaused) {
            SimulationClock::advance(delta);

            if (alienBTimer.getElapsedTime().asSeconds() > 1.8f) {
                sAlienB.stop();
            }

            if (alienSTimer.getElapsedTime().asSeconds() > 1.8f) {
                sAlienS.stop();
            }

            if (asteroids.empty() && aliens.empty()) {
                ++level;
                beatInterval = 1.f;
                soundSpeedTimer.restart();
                beat.restart();

                const bool extraLife =
                    (level % 2 == 0 && level < 10) ||
                    (level % 4 == 2 && level >= 10);

                for (int i = firstPlayer; i < endPlayer; ++i) {
                    if (extraLife) {
                        ++spaceships[i].lives;
                    }

                    spaceships[i].invincible = true;
                    spaceships[i].invincibleTimer.restart();
                }

                asteroids =
                    Asteroid::initAsteroids(asteroidAmount, level);

                aliensSpawned = 0;
                alienSpawnTimer.restart();

                nextAlienSpawnTime = static_cast<float>(
                    (std::rand() % 5) + (coop ? 6 : 8)
                );
            }

            if (aliensSpawned < 2 &&
                alienSpawnTimer.getElapsedTime().asSeconds() >
                    nextAlienSpawnTime) {
                addAlien(
                    aliens,
                    aliensSpawned == 0 ? AlienSize::BIG : AlienSize::SMALL,
                    sound
                );

                ++aliensSpawned;
                alienSpawnTimer.restart();

                nextAlienSpawnTime = static_cast<float>(
                    (std::rand() % 5) + (coop ? 6 : 8)
                );
            }

            for (int i = firstPlayer; i < endPlayer; ++i) {
                auto& ship = spaceships[i];

                if (!ship.explosion && ship.lives > 0) {
                    if (i == 2) {
                        handlePlayerInput(ship, delta, COOP_LEFT, COOP_RIGHT, COOP_UP);
                    } else {
                        handlePlayerInput(ship, delta, LEFT, RIGHT, UP);
                    }
                }

                ship.update(delta, sound);
            }

            // The world moves exactly once, irrespective of player count.
            moveAsteroids(asteroids, delta);

            for (auto& alien : aliens) {
                alien->update(delta, sound);
            }

            for (int i = firstPlayer; i < endPlayer; ++i) {
                int& playerScore = i == 2 ? score2 : score;

                resolveAsteroidCollisions(
                    asteroids, spaceships[i], playerScore, level, sound
                );

                resolveAlienCollisions(
                    aliens, spaceships[i], playerScore, sound
                );
            }

            std::erase_if(aliens, [](const auto& alien) {
                return alien->isDestroyed();
            });

            for (auto& alien : aliens) {
                std::optional<sf::Vector2f> target;
                float bestDistance = std::numeric_limits<float>::max();

                for (int i = firstPlayer; i < endPlayer; ++i) {
                    const auto& ship = spaceships[i];

                    if (ship.explosion || ship.lives <= 0) {
                        continue;
                    }

                    const auto difference =
                        ship.shape.getPosition() - alien->shape.getPosition();

                    const float distance =
                        difference.x * difference.x +
                        difference.y * difference.y;

                    if (distance < bestDistance) {
                        bestDistance = distance;
                        target = ship.shape.getPosition();
                    }
                }

                if (target) {
                    alien->shoot(sound, target);
                }
            }

            bool thrusting = false;

            for (int i = firstPlayer; i < endPlayer; ++i) {
                const auto& ship = spaceships[i];

                thrusting |=
                    !ship.explosion &&
                    ship.lives > 0 &&
                    (ship.engineOn || ship.engineOnC);
            }

            if (sound && thrusting) {
                if (sThrust.getStatus() != sf::Sound::Playing) {
                    playSound(sThrust);
                }
            } else {
                sThrust.stop();
            }

            if (soundSpeedTimer.getElapsedTime().asSeconds() >= 5.f) {
                beatInterval = std::max(0.1f, beatInterval * 0.9f);
                soundSpeedTimer.restart();
            }

            if (beat.getElapsedTime().asSeconds() >= beatInterval) {
                playBeat(sBeat1, sBeat2, sound);
                beat.restart();
            }

            const bool gameOver =
                coop
                    ? spaceships[1].lives <= 0 && spaceships[2].lives <= 0
                    : spaceships[0].lives <= 0;

            if (gameOver) {
                state = GameState::GAME_OVER;
                return result();
            }
        }

        // Always redraw, including while paused.
        window.clear(sf::Color::Black);

        for (const auto& asteroid : asteroids) {
            window.draw(asteroid->shape);
        }

        for (auto& alien : aliens) {
            alien->draw(window);
        }

        for (int i = firstPlayer; i < endPlayer; ++i) {
            spaceships[i].draw(window);

            drawScore(
                window,
                spaceships[i].id,
                i == 2 ? score2 : score
            );
        }

        if (coop) {
            drawLivesCoop(window, spaceships);
        } else {
            drawLives(window, spaceships[0]);
        }

        if (paused) {
            sf::Text text("Pause", font, 40);
            text.setFillColor(WHITE);
            text.setPosition(
                SIZE_X / 2.f - text.getLocalBounds().width / 2.f,
                SIZE_Y / 2.f
            );
            window.draw(text);
        }

        window.display();
    }

    return result();
}

void handlePlayerInput(
    SpaceShip& spaceship,
    float delta,
    sf::Keyboard::Key left,
    sf::Keyboard::Key right,
    sf::Keyboard::Key thrust
) {
    const unsigned controllerId = spaceship.id == 2 ? 1u : 0u;
    const bool connected = sf::Joystick::isConnected(controllerId);

    float turn = 0.f;

    if (sf::Keyboard::isKeyPressed(left)) {
        turn -= 1.f;
    }

    if (sf::Keyboard::isKeyPressed(right)) {
        turn += 1.f;
    }

    bool controllerThrust = false;

    if (connected) {
        const float axis = sf::Joystick::getAxisPosition(
            controllerId,
            sf::Joystick::X
        );

        if (axis < -15.f) turn -= 1.f;
        if (axis > 15.f) turn += 1.f;

        controllerThrust =
            sf::Joystick::isButtonPressed(controllerId, 7);
    }

    turn = std::clamp(turn, -1.f, 1.f);
    spaceship.rotate(turn * ROTATION * delta);

    spaceship.toggleEngine(sf::Keyboard::isKeyPressed(thrust));
    spaceship.toggleEngineC(controllerThrust);
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

void resolveAsteroidCollisions(
        std::vector<std::unique_ptr<Asteroid>>& asteroids,
        SpaceShip& spaceship,
        int &score,
        int &level,
        bool sound
) {
    if (spaceship.explosion || spaceship.lives <= 0) return;

    for (auto& asteroid : asteroids) {
        float asteroidRadius = asteroid->shape.getGlobalBounds().width / 2;

        if (!spaceship.explosion && !spaceship.invincible &&
            checkCollision(spaceship.shape,spaceship.radius,
                           asteroid->shape, asteroidRadius))
        {
            spaceship.explode(sound);
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
                    sf::Vector2f oldPosition = (*a_it)->shape.getPosition();

                    a_it = asteroids.erase(a_it);

                    if (size == AsteroidSize::BIG) {
                        for (int i = 0; i < 2; i++) {
                            auto newAsteroid = std::make_unique<Asteroid>(AsteroidSize::MEDIUM);
                            newAsteroid->initShape(level);
                            newAsteroid->shape.setPosition(oldPosition + sf::Vector2f(i * 10, 0));
                            asteroids.push_back(std::move(newAsteroid));
                        }

                    } else if (size == AsteroidSize::MEDIUM) {
                        for (int i = 0; i < 2; i++) {
                            auto newAsteroid = std::make_unique<Asteroid>(AsteroidSize::SMALL);
                            newAsteroid->initShape(level);
                            newAsteroid->shape.setPosition(oldPosition + sf::Vector2f(i * 10, 0));
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

void resolveAlienCollisions(
    std::vector<std::unique_ptr<Alien>>& aliens,
    SpaceShip& spaceship,
    int& score,
    bool sound
) {
    if (spaceship.explosion || spaceship.lives <= 0) return;

    for (auto& alien : aliens) {
        if (alien->isDestroyed()) {
            continue;
        }

        const float alienRadius =
            alien->shape.getGlobalBounds().width / 2.f;

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

        auto& shots = spaceship.projectiles;

        for (auto it = shots.begin();
             it != shots.end() && !alien->isDestroyed();) {
            if (checkCollision(
                    it->position,
                    it->radius,
                    alien->shape.getPosition(),
                    alienRadius
                )) {
                it = shots.erase(it);
                alien->takeDamage(sound);

                if (alien->isDestroyed()) {
                    addScore(
                        score,
                        alien->alienSize == AlienSize::BIG ? 200 : 1000
                    );
                }
            } else {
                ++it;
            }
        }

        if (alien->isDestroyed()) {
            continue;
        }

        auto& enemyShots = alien->projectiles;

        for (auto it = enemyShots.begin(); it != enemyShots.end();) {
            if (!checkCollision(
                    it->position,
                    it->radius,
                    spaceship.shape.getPosition(),
                    spaceship.radius
                )) {
                ++it;
                continue;
            }

            it = enemyShots.erase(it);

            if (!spaceship.invincible) {
                spaceship.explode(sound);
                return;
            }
        }
    }
}

void addAlien(
    std::vector<std::unique_ptr<Alien>>& aliens,
    AlienSize size,
    bool sound
) {
    aliens.push_back(std::make_unique<Alien>(size));

    auto& audio = size == AlienSize::BIG ? sAlienB : sAlienS;
    auto& timer = size == AlienSize::BIG ? alienBTimer : alienSTimer;

    audio.setLoop(true);
    timer.restart();

    if (sound) playSound(audio);
}

// Draws a single digit of the score on the screen at the specified position
void drawNumber(sf::RenderWindow &window, int n, sf::Vector2f pos, int id)
{
    const auto& vectors = NUMBER_LINES[n];
    sf::VertexArray vertices(sf::Lines, vectors.size());

    for (size_t i = 0; i < vectors.size(); i += 2) {
        vertices[i].position = sf::Vector2f(
                pos.x + vectors[i].x * NUMBER_SCALE, pos.y + vectors[i].y * NUMBER_SCALE );
        vertices[i + 1].position = sf::Vector2f(
                pos.x + vectors[i + 1].x * NUMBER_SCALE, pos.y + vectors[i + 1].y * NUMBER_SCALE );
    }

    sf::Color color = (id == 1) ? PLAYER_1_COLOR : (id == 2) ? PLAYER_2_COLOR : WHITE;

    for (size_t i = 0; i < vertices.getVertexCount(); ++i) {
        vertices[i].color = color;
    }

    window.draw(vertices);
}

void drawScore(sf::RenderWindow& window, int id, int score) {
    std::string text = std::to_string(std::max(0, score));

    if (text.size() < 5) {
        text.insert(0, 5 - text.size(), '0');
    }

    const float spacing = NUMBER_SCALE * 1.2f;

    sf::Vector2f position(
        SIZE_X - 20.f - spacing * static_cast<float>(text.size()),
        id == 2 ? 60.f : 20.f
    );

    for (char digit : text) {
        drawNumber(window, digit - '0', position, id);
        position.x += spacing;
    }
}

void drawLives(sf::RenderWindow &window, const SpaceShip& spaceship, int y) {
    sf::ConvexShape shape;

    shape.setPointCount(5);

    shape.setPoint(0, sf::Vector2f(-10, -12));
    shape.setPoint(1, sf::Vector2f(0, 10));
    shape.setPoint(2, sf::Vector2f(10, -12));
    shape.setPoint(3, sf::Vector2f(8, -10));
    shape.setPoint(4, sf::Vector2f(-8, -10));

    shape.setOutlineColor(spaceship.handleColor());
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineThickness(1.0f);
    shape.setRotation(180);

    for (int i = 0; i < spaceship.lives; ++i) {
        shape.setPosition(25 + i * 40, y);
        window.draw(shape);
    }
}

void drawLivesCoop(sf::RenderWindow &window, const SpaceShip* spaceships) {
    // 28 and 68 are the y-offset
    drawLives(window, spaceships[1], 28);
    drawLives(window, spaceships[2], 68);
}
