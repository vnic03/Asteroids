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
#include "Progression.h"
#include "WaveController.h"

namespace {

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

void updatePlayers(SpaceShip* spaceships, int firstPlayer, int endPlayer,
                   float delta, bool sound) {
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
}

void updateAlienShooting(std::vector<std::unique_ptr<Alien>>& aliens,
                         const SpaceShip* spaceships, int firstPlayer,
                         int endPlayer, bool sound) {
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
                difference.x * difference.x + difference.y * difference.y;

            if (distance < bestDistance) {
                bestDistance = distance;
                target = ship.shape.getPosition();
            }
        }

        // Also notify the alien when no target is available.
        alien->shoot(sound, target);
    }
}

void updateThrustSound(const SpaceShip* spaceships, int firstPlayer,
                       int endPlayer, bool sound) {
    bool thrusting = false;

    for (int i = firstPlayer; i < endPlayer; ++i) {
        const auto& ship = spaceships[i];

        thrusting |= !ship.explosion && ship.lives > 0 &&
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

std::pair<int, int> runGame(sf::RenderWindow& window, const sf::Font& font,
                            GameState& state, int& score, int& score2) {
    bool paused = false;
    bool sound = soundEnabled;

    sf::Clock frameClock;

    SimulationClock beat;
    SimulationClock soundSpeedTimer;

    float beatInterval = GameRules::InitialBeatInterval;

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    SpaceShip spaceships[] = {SpaceShip(0), SpaceShip(1), SpaceShip(2)};

    const bool coop = state == GameState::COOP_RUNNING;
    const int firstPlayer = coop ? 1 : 0;
    const int endPlayer = coop ? 3 : 1;

    std::vector<std::unique_ptr<Asteroid>> asteroids;
    std::vector<std::unique_ptr<Alien>> aliens;

    WaveController waves(coop);
    Progression progression;
    GameRenderer renderer(font);

    frameClock.restart();

    auto result = [&]() {
        stopGameSounds();
        return std::make_pair(score, coop ? score2 : -1);
    };

    while (window.isOpen()) {
        const float frameDelta = frameClock.restart().asSeconds();

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
                } else if (coop && event.joystickButton.joystickId == 1) {
                    spaceships[2].shoot(sound);
                }
            }
        }

        float remaining =
            (!paused && !wasPaused && window.hasFocus()) ? frameDelta : 0.f;

        while (remaining > 0.f) {
            const float delta = std::min(remaining, GameRules::MaxFrameDelta);

            remaining -= delta;
            SimulationClock::advance(delta);

            stopExpiredAlienSounds();

            const int previousWave = waves.currentLevel();

            waves.update(delta, spaceships, firstPlayer, endPlayer, asteroids,
                         aliens, sound);

            if (waves.currentLevel() != previousWave) {
                beatInterval = GameRules::InitialBeatInterval;

                soundSpeedTimer.restart();
                beat.restart();
            }

            updatePlayers(spaceships, firstPlayer, endPlayer, delta, sound);

            // Update the world once per simulation step.
            moveAsteroids(asteroids, delta);

            for (auto& alien : aliens) {
                alien->update(delta, sound);
            }

            for (int i = firstPlayer; i < endPlayer; ++i) {
                int& playerScore = i == 2 ? score2 : score;

                resolveAsteroidCollisions(asteroids, spaceships[i], playerScore,
                                          waves.currentLevel(), sound);

                resolveAlienCollisions(aliens, spaceships[i], playerScore,
                                       sound);
            }

            std::erase_if(
                aliens, [](const auto& alien) { return alien->isDestroyed(); });

            const int bonus =
                waves.finishFrame(asteroids.empty() && aliens.empty());

            for (int i = firstPlayer; i < endPlayer; ++i) {
                int& playerScore = i == 2 ? score2 : score;

                if (bonus > 0 && spaceships[i].lives > 0) {
                    Progression::addScore(playerScore, bonus);
                }

                progression.update(spaceships[i], playerScore);
            }

            updateAlienShooting(aliens, spaceships, firstPlayer, endPlayer,
                                sound);

            updateThrustSound(spaceships, firstPlayer, endPlayer, sound);

            if (!waves.resting() &&
                soundSpeedTimer.getElapsedTime().asSeconds() >=
                    GameRules::BeatAccelerationInterval) {
                beatInterval =
                    std::max(GameRules::MinimumBeatInterval,
                             beatInterval * GameRules::BeatAccelerationFactor);

                soundSpeedTimer.restart();
            }

            if (!waves.resting() &&
                beat.getElapsedTime().asSeconds() >= beatInterval) {
                playBeat(sBeat1, sBeat2, sound);
                beat.restart();
            }

            const bool gameOver =
                coop ? spaceships[1].lives <= 0 && spaceships[2].lives <= 0
                     : spaceships[0].lives <= 0;

            if (gameOver) {
                state = GameState::GAME_OVER;
                return result();
            }
        }

        renderer.draw(window, asteroids, aliens, spaceships, coop, score,
                      score2, paused, waves, progression);
    }

    return result();
}
