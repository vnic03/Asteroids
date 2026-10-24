#include "logic.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

#include "Collisions.h"
#include "GameRenderer.h"
#include "GameRules.h"
#include "Movement.h"
#include "WaveController.h"

namespace {

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

    if (sound) {
        playSound(audio);
    }
}

void stopExpiredAlienSounds() {
    if (alienBTimer.getElapsedTime().asSeconds() >
            GameRules::AlienSoundSeconds &&
        sAlienB.getStatus() != sf::Sound::Stopped) {
        sAlienB.stop();
    }

    if (alienSTimer.getElapsedTime().asSeconds() >
            GameRules::AlienSoundSeconds &&
        sAlienS.getStatus() != sf::Sound::Stopped) {
        sAlienS.stop();
    }
}

void updatePlayers(
    SpaceShip* spaceships,
    int firstPlayer,
    int endPlayer,
    float delta,
    bool sound
) {
    for (int i = firstPlayer; i < endPlayer; ++i) {
        auto& ship = spaceships[i];

        if (!ship.explosion && ship.lives > 0) {
            if (i == 2) {
                handlePlayerInput(
                    ship, delta,
                    COOP_LEFT, COOP_RIGHT, COOP_UP
                );
            } else {
                handlePlayerInput(
                    ship, delta,
                    LEFT, RIGHT, UP
                );
            }
        }

        ship.update(delta, sound);
    }
}

void updateAlienShooting(
    std::vector<std::unique_ptr<Alien>>& aliens,
    const SpaceShip* spaceships,
    int firstPlayer,
    int endPlayer,
    bool sound
) {
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
}

void updateThrustSound(
    const SpaceShip* spaceships,
    int firstPlayer,
    int endPlayer,
    bool sound
) {
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
    } else if (sThrust.getStatus() != sf::Sound::Stopped) {
        sThrust.stop();
    }
}

} // namespace

std::pair<int, int> runGame(
    sf::RenderWindow& window,
    const sf::Font& font,
    GameState& state,
    int& score,
    int& score2
) {
    bool paused = false;
    bool sound = soundEnabled;

    sf::Clock frameClock;

    SimulationClock beat;
    SimulationClock soundSpeedTimer;

    float beatInterval = GameRules::InitialBeatInterval;

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    SpaceShip spaceships[] = {
        SpaceShip(0),
        SpaceShip(1),
        SpaceShip(2)
    };

    const bool coop = state == GameState::COOP_RUNNING;
    const int firstPlayer = coop ? 1 : 0;
    const int endPlayer = coop ? 3 : 1;

    const int asteroidAmount = GameRules::baseAsteroidAmount(coop);

    auto asteroids = Asteroid::initAsteroids(asteroidAmount, 1);
    std::vector<std::unique_ptr<Alien>> aliens;

    // Construct after the initial asteroids to preserve the order
    // in which random values are consumed.
    WaveController waves(coop);
    GameRenderer renderer(font);

    auto result = [&]() {
        stopGameSounds();
        return std::make_pair(score, coop ? score2 : -1);
    };

    while (window.isOpen()) {
        const float delta = std::min(
            frameClock.restart().asSeconds(),
            GameRules::MaxFrameDelta
        );

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

            if (event.type == sf::Event::JoystickButtonPressed &&
                event.joystickButton.button == 0) {
                if (event.joystickButton.joystickId == 0) {
                    spaceships[coop ? 1 : 0].shoot(sound);
                } else if (
                    coop &&
                    event.joystickButton.joystickId == 1
                ) {
                    spaceships[2].shoot(sound);
                }
            }
        }

        if (!paused && !wasPaused) {
            SimulationClock::advance(delta);

            stopExpiredAlienSounds();

            if (waves.shouldStartNextWave(
                    asteroids.empty() && aliens.empty()
                )) {
                const int nextLevel = waves.currentLevel() + 1;

                beatInterval = GameRules::InitialBeatInterval;
                soundSpeedTimer.restart();
                beat.restart();

                for (int i = firstPlayer; i < endPlayer; ++i) {
                    if (GameRules::grantsExtraLife(nextLevel)) {
                        ++spaceships[i].lives;
                    }

                    spaceships[i].invincible = true;
                    spaceships[i].invincibleTimer.restart();
                }

                asteroids = Asteroid::initAsteroids(
                    asteroidAmount,
                    nextLevel
                );

                waves.beginNextWave();
            }

            if (const auto alienSize = waves.pendingAlienSpawn()) {
                addAlien(aliens, *alienSize, sound);
                waves.confirmAlienSpawn();
            }

            updatePlayers(
                spaceships,
                firstPlayer,
                endPlayer,
                delta,
                sound
            );

            // World movement happens once, including in coop.
            moveAsteroids(asteroids, delta);

            for (auto& alien : aliens) {
                alien->update(delta, sound);
            }

            for (int i = firstPlayer; i < endPlayer; ++i) {
                int& playerScore = i == 2 ? score2 : score;

                resolveAsteroidCollisions(
                    asteroids,
                    spaceships[i],
                    playerScore,
                    waves.currentLevel(),
                    sound
                );

                resolveAlienCollisions(
                    aliens,
                    spaceships[i],
                    playerScore,
                    sound
                );
            }

            std::erase_if(aliens, [](const auto& alien) {
                return alien->isDestroyed();
            });

            updateAlienShooting(
                aliens,
                spaceships,
                firstPlayer,
                endPlayer,
                sound
            );

            updateThrustSound(
                spaceships,
                firstPlayer,
                endPlayer,
                sound
            );

            if (soundSpeedTimer.getElapsedTime().asSeconds() >=
                GameRules::BeatAccelerationInterval) {
                beatInterval = std::max(
                    GameRules::MinimumBeatInterval,
                    beatInterval * GameRules::BeatAccelerationFactor
                );

                soundSpeedTimer.restart();
            }

            if (beat.getElapsedTime().asSeconds() >= beatInterval) {
                playBeat(sBeat1, sBeat2, sound);
                beat.restart();
            }

            const bool gameOver =
                coop
                    ? spaceships[1].lives <= 0 &&
                      spaceships[2].lives <= 0
                    : spaceships[0].lives <= 0;

            if (gameOver) {
                state = GameState::GAME_OVER;
                return result();
            }
        }

        renderer.draw(
            window,
            asteroids,
            aliens,
            spaceships,
            coop,
            score,
            score2,
            paused
        );
    }

    return result();
}
