#include "logic.h"


std::pair<int, int> runGame(sf::RenderWindow &window, const sf::Font& font,
                            GameState& state, int &score, int &score2)
{
    bool sound = true; // tracks if sound is playing
    bool paused = false; // tracks if the game is running or paused

    sf::Clock clock; // Game loop clock
    sf::Clock beat; // Beat clock

    // Timer for sound speed to make the beat faster over time
    sf::Clock soundSpeedTimer;

    // Seed for random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    int level = 1;
    float initialBI = 1.f; // Initial beat interval
    float beatInterval = initialBI;

    // 0: Single-PLayer, 1 & 2: COOP
    SpaceShip spaceships[] = { SpaceShip(0), SpaceShip(1), SpaceShip(2) };

    bool coop = state == GameState::COOP_RUNNING;

    // Asteroids, and Aliens
    const int ASTEROID_AMOUNT = coop ? 12 : 10;
    auto asteroids = Asteroid::initAsteroids(ASTEROID_AMOUNT, level);
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
        if (sound) {
            if (alienBTimer.getElapsedTime().asSeconds() > 1.8) {
                sAlienB.stop();
                sAlienB.setLoop(false);
            }
            if (alienSTimer.getElapsedTime().asSeconds() > 1.8) {
                sAlienS.stop();
                sAlienS.setLoop(false);
            }
        }

        sf::Event event{ };
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == SHOOT) {
                    if (!paused) {
                        if (!coop) spaceships[0].keyClicked = false;
                        else spaceships[1].keyClicked = false;
                    }
                } else if (event.key.code == COOP_SHOOT) {
                    if (coop && !paused) {
                        spaceships[2].keyClicked = false;
                    }

                } else if (event.key.code == PAUSE) {
                    paused = !paused;

                } else if (event.key.code == SOUND) {
                    sound = !sound;
                }
            }
        }
        if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == SHOOT) {
                if (!coop) spaceships[0].keyClicked = false;
                else spaceships[1].keyClicked = false;

            } else if (event.key.code == COOP_SHOOT) {
                if (coop) spaceships[2].keyClicked = false;
            }
        }
        if (!paused) {
            if (asteroids.empty() && aliens.empty()) {
                level++;
                beatInterval = initialBI;
                soundSpeedTimer.restart();
                if ((level % 2 == 0 && level < 10) || (level % 4 == 2 && level >= 10))
                {
                    if (!coop) {
                        spaceships[0].lives++;
                    } else {
                        spaceships[1].lives++;
                        spaceships[2].lives++;
                    }
                }
                if (!coop) {
                    spaceships[0].invincible = true;
                    spaceships[0].invincibleTimer.restart();
                } else {
                    spaceships[1].invincible = true;
                    spaceships[1].invincibleTimer.restart();
                    spaceships[2].invincible = true;
                    spaceships[2].invincibleTimer.restart();
                }
                asteroids = Asteroid::initAsteroids(ASTEROID_AMOUNT, level);
                aliensSpawned = 0;
                alienSpawnTimer.restart();
                nextAlienSpawnTime = coop ? (std::rand() % 5) + 6 : (std::rand() % 5) + 8;
            }
            if (aliensSpawned < 2 &&
                alienSpawnTimer.getElapsedTime().asSeconds() > nextAlienSpawnTime)
            {
                AlienSize size = (aliensSpawned % 2 == 0) ? AlienSize::BIG : AlienSize::SMALL;
                addAlien(aliens, size, delta, sound);

                aliensSpawned++;
                alienSpawnTimer.restart();

                if (aliensSpawned < 2) {
                    nextAlienSpawnTime = coop ? (std::rand() % 5) + 6 : (std::rand() % 5) + 8;
                }
            }

            // Controls
            if (!coop) {
                if (!spaceships[0].explosion) {
                    controllerInput(spaceships, paused, delta, coop, sound);
                    handlePlayerInput(spaceships[0], delta, sound, LEFT, RIGHT, UP, SHOOT);
                }
                spaceships[0].update(delta, sound);

            } else {
                if (!spaceships[1].explosion) {
                    controllerInput(spaceships, paused, delta, coop, sound);
                    handlePlayerInput(spaceships[1], delta, sound,LEFT, RIGHT, UP, SHOOT);
                }
                if (!spaceships[2].explosion) {
                    controllerInput(spaceships, paused, delta, coop, sound);
                    handlePlayerInput(spaceships[2], delta, sound, COOP_LEFT, COOP_RIGHT, COOP_UP, COOP_SHOOT);
                }
                spaceships[1].update(delta, sound);
                spaceships[2].update(delta, sound);
            }

            window.clear(sf::Color::Black);

            if (!coop) spaceships[0].draw(window);
            else {
                spaceships[1].draw(window);
                spaceships[2].draw(window);
            }

            if (!coop) {
                updateAsteroids(asteroids, spaceships[0], delta, score, level, sound);
                updateAliens(aliens, spaceships[0], delta, score, sound);
            } else {
                updateAsteroids(asteroids, spaceships[1], delta, score, level, sound);
                updateAliens(aliens, spaceships[1], delta, score, sound);
                updateAsteroids(asteroids, spaceships[2], delta, score2, level, sound);
                updateAliens(aliens, spaceships[2], delta, score2, sound);
            }

            for (auto &asteroid: asteroids) {
                window.draw(asteroid->shape);
            }
            for (auto& alien : aliens) {
                alien->draw(window);
            }

            if (!coop) {
                drawScore(window, spaceships[0].id, score);
            } else {
                drawScore(window, spaceships[1].id, score);
                drawScore(window, spaceships[2].id, score2);
            }

            if (!coop) drawLives(window, spaceships[0]);
            else drawLivesCoop(window, spaceships);

            if (!coop) {
                if (spaceships[0].lives <= 0) {
                    state = GameState::GAME_OVER;
                    return std::make_pair(score, -1);
                }
            } else {
                if (spaceships[1].lives <= 0 && spaceships[2].lives <= 0) {
                    state = GameState::GAME_OVER;
                    return std::make_pair(score, score2);
                }
            }

        } else {
            // PAUSE - Display "Pause" text
            sf::Text pauseText("Pause", font, 40);
            pauseText.setFillColor(WHITE);
            pauseText.setPosition(
                    SIZE_X / 2.f - pauseText.getLocalBounds().width / 2, SIZE_Y / 2.f
                    );
            window.draw(pauseText);
        }

        // Every 5 seconds the beat gets faster by 10%
        if (soundSpeedTimer.getElapsedTime().asSeconds() >= 5) {
            beatInterval *= 0.9f;
            soundSpeedTimer.restart();
        }

        if (beat.getElapsedTime().asSeconds() > beatInterval) {
            playBeat(sBeat1, sBeat2, sound);
            beat.restart();
        }

        window.display();
    }

    return coop ? std::make_pair(score, score2) : std::make_pair(score, -1);
}

void handlePlayerInput(SpaceShip& spaceship, float delta, bool sound,
                       sf::Keyboard::Key left, sf::Keyboard::Key right,
                       sf::Keyboard::Key thrust, sf::Keyboard::Key fire)
{
    if (sf::Keyboard::isKeyPressed(left)) {
        spaceship.rotate(-ROTATION * delta);
    }
    if (sf::Keyboard::isKeyPressed(right)) {
        spaceship.rotate(ROTATION * delta);
    }
    spaceship.toggleEngine(sf::Keyboard::isKeyPressed(thrust));
    if (sf::Keyboard::isKeyPressed(fire) && !spaceship.keyClicked) {
        spaceship.shoot(sound);
        spaceship.keyClicked = true;
    }
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
                     SpaceShip& spaceship, float delta, int &score, int &level, bool sound)
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
                            score += 20;
                            if (sound) sSmallEx.play();
                            break;
                        case AsteroidSize::MEDIUM:
                            score += 50;
                            if (sound) sMediumEx.play();
                            break;
                        case AsteroidSize::SMALL:
                            score += 100;
                            if (sound) sBigEx.play();
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

void updateAliens(std::vector<std::unique_ptr<Alien>>& aliens,
                  SpaceShip& spaceship, float delta, int &score, bool sound)
{
    for (auto& alien : aliens) {
        alien->projectiles.erase(
                std::remove_if(
                        alien->projectiles.begin(), alien->projectiles.end(),
                        [&spaceship] (const Projectile& p) {
                            return checkCollision(p.position,
                                                  p.radius,
                                                  spaceship.shape.getPosition(),
                                                  spaceship.radius );
                        }
                ),
                alien->projectiles.end()
        );
        alien->update(delta, sound);

        if (!spaceship.explosion && !spaceship.invincible &&
            checkCollision(spaceship.shape, spaceship.radius,
                           alien->shape, alien->shape.getGlobalBounds().width / 2))
        {
            spaceship.explode(sound);
            alien->explode(sound);
        }

        auto& projectiles = spaceship.projectiles;

        for (auto p_it = projectiles.begin(); p_it != projectiles.end();) {
            bool hit = false;

            if (!spaceship.explosion && checkCollision(p_it->position, p_it->radius,
                               alien->shape.getPosition(),
                               alien->shape.getGlobalBounds().width / 2))
            {
                hit = true;
                alien->takeDamage(sound);
                if (alien->isDestroyed()) {
                    score += (alien->alienSize == AlienSize::BIG) ? 200 : 1000;
                }
                p_it = projectiles.erase(p_it);
            } else {
                ++p_it;
            }
        }
        for (auto& p : alien->projectiles) {
            if (!spaceship.explosion && !spaceship.invincible &&
                checkCollision(p.position, p.radius, spaceship.shape.getPosition(), spaceship.radius))
            {
                spaceship.explode(sound);
                break;
            }
        }
    }
    aliens.erase(std::remove_if(
            aliens.begin(),
            aliens.end(),
            [] (const std::unique_ptr<Alien>& alien) {
                return alien->isDestroyed();
            }
    ), aliens.end());

    for (auto& alien : aliens) {
        alien->shoot(sound, spaceship.shape.getPosition());
    }
}

void addAlien(std::vector<std::unique_ptr<Alien>>& aliens, AlienSize size, float delta, bool sound)
{
    auto alien = std::make_unique<Alien>(size);
    alien->update(delta, sound);
    aliens.push_back(std::move(alien));

    if (size == AlienSize::BIG) {
        sAlienB.setLoop(true);
        if (sound) sAlienB.play();
        alienBTimer.restart();
    }
    if (size == AlienSize::SMALL) {
        sAlienS.setLoop(true);
        if (sound) sAlienS.play();
        alienSTimer.restart();
    }
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

void drawScore(sf::RenderWindow &window, int id, int score) {
    std::string str = std::to_string(score);
    str = std::string(5 - str.length(), '0') + str;
    sf::Vector2f pos;

    if (id == 0 || id == 1) {
        pos = sf::Vector2f(SIZE_X - (100 + NUMBER_SCALE * 4), 20);
    } else {
        pos = sf::Vector2f(SIZE_X - (100 + NUMBER_SCALE * 4), 60);
    }

    for (char digit : str) {
        int n = digit - '0';
        drawNumber(window, n, pos, id);
        pos.x += NUMBER_SCALE * 1.2f;
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

// Handles the input from a single game controller
void handleControllerInput(SpaceShip& s, unsigned int id, float delta,
                           float joystickDeadzone, unsigned int move,
                           unsigned int shoot, bool& shot, bool paused, bool sound)
{
    float x = sf::Joystick::getAxisPosition(id, sf::Joystick::X);

    if (x < -joystickDeadzone && !paused) s.rotate(-ROTATION * delta);
    else if (x > joystickDeadzone) s.rotate(ROTATION * delta);

    s.toggleEngineC(sf::Joystick::isButtonPressed(id, move));

    bool shootButtonPressed = sf::Joystick::isButtonPressed(id, shoot);
    if (shootButtonPressed && !shot) {
        s.shoot(sound);
        shot = true;
    }
    if (!shootButtonPressed)  shot = false;
}

void controllerInput(SpaceShip* spaceships, bool& paused, float delta, bool& coop, bool sound)
{
    // These are PS4 specific rn
    const unsigned int CONTROLLER_ID = 0;
    const unsigned int SECOND_CONTROLLER_ID = 1;

    const float JOYSTICK_DEADZONE = coop ? 65 : 15;
    const unsigned int R2_BUTTON = 7;
    const unsigned int X_BUTTON = 0;

    // shoot button pressed
    static bool shot1 = false;
    static bool shot2 = false;

    if (sf::Joystick::isConnected(CONTROLLER_ID)) {
        handleControllerInput(coop ? spaceships[1] : spaceships[0],
                              CONTROLLER_ID, delta, JOYSTICK_DEADZONE, R2_BUTTON, X_BUTTON, shot1,
                              paused, sound);
    }
    if (sf::Joystick::isConnected(SECOND_CONTROLLER_ID) && coop) {
        handleControllerInput(spaceships[2], SECOND_CONTROLLER_ID, delta,
                              JOYSTICK_DEADZONE, R2_BUTTON, X_BUTTON, shot2, paused, sound);
    }
}
