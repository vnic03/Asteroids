#include "WaveController.h"
#include "GameRules.h"

#include <algorithm>
#include <cmath>

WaveController::WaveController(bool coop) : coop(coop) { buildSchedule(); }

float WaveController::secondsRemaining() const {
    return std::max(0.f,
                    inRest ? restRemaining : GameRules::WaveSeconds - elapsed);
}

int WaveController::weight(Kind kind) {
    switch (kind) {
    case Kind::BIG_ROCK:
        return 4;
    case Kind::MEDIUM_ROCK:
        return 2;
    case Kind::SMALL_ROCK:
        return 1;
    default:
        return 0;
    }
}

void WaveController::buildSchedule() {
    events.clear();

    const auto type = GameRules::waveType(level);
    const int count = GameRules::asteroidCount(level);

    auto rock = [&](int index) {
        if (level == 1) {
            return index < 2 ? Kind::BIG_ROCK : Kind::MEDIUM_ROCK;
        }

        if (type == GameRules::WaveType::DEBRIS) {
            return Kind::SMALL_ROCK;
        }

        if (type == GameRules::WaveType::FIELD) {
            return index % 3 == 0 ? Kind::BIG_ROCK : Kind::MEDIUM_ROCK;
        }

        if (level <= 3) {
            return index % 2 == 0 ? Kind::BIG_ROCK : Kind::MEDIUM_ROCK;
        }

        return static_cast<Kind>(index % 3);
    };

    const float span = level == 1 ? 6.f : 16.f;

    for (int i = 0; i < count; ++i) {
        events.push_back({span * i / std::max(1, count - 1), rock(i), false});
    }

    if (coop) {
        const int extra = (count + 1) / 2;

        for (int i = 0; i < extra; ++i) {
            events.push_back({span * (i + 0.5f) / extra, rock(i), true});
        }
    }

    auto alien = [&](float at, Kind kind) {
        events.push_back({at, kind, false});
    };

    if (level == 2 || level == 3) {
        alien(12.f, Kind::BIG_ALIEN);
    } else if (level == 4) {
        alien(12.f, Kind::SMALL_ALIEN);
    } else if (level == 5) {
        alien(9.f, Kind::BIG_ALIEN);
        alien(18.f, Kind::SMALL_ALIEN);
    } else if (level > 5) {
        switch (type) {
        case GameRules::WaveType::RECOVERY:
            alien(14.f, Kind::BIG_ALIEN);
            break;

        case GameRules::WaveType::FIELD:
            alien(16.f, Kind::BIG_ALIEN);
            break;

        case GameRules::WaveType::DEBRIS:
            alien(16.f, Kind::SMALL_ALIEN);
            break;

        case GameRules::WaveType::HUNT:
            alien(6.f, Kind::BIG_ALIEN);
            alien(14.f, Kind::SMALL_ALIEN);
            alien(22.f, Kind::SMALL_ALIEN);
            break;

        default:
            alien(4.f, Kind::BIG_ALIEN);
            alien(10.f, Kind::SMALL_ALIEN);
            alien(17.f, Kind::BIG_ALIEN);
            alien(23.f, Kind::SMALL_ALIEN);
            break;
        }
    }

    std::stable_sort(
        events.begin(), events.end(),
        [](const Event& a, const Event& b) { return a.at < b.at; });
}

void WaveController::beginNextWave(float carry) {
    ++level;

    elapsed = carry;
    inRest = false;
    restRemaining = 0.f;

    bonus = 0;
    nextEvent = 0;

    warnings.clear();
    allSpawnsDelivered = true;

    buildSchedule();
}

sf::Vector2f WaveController::edgePosition() {
    constexpr int inset = 48;

    const float x =
        static_cast<float>(inset + std::rand() % (SIZE_X - 2 * inset));

    const float y =
        static_cast<float>(inset + std::rand() % (SIZE_Y - 2 * inset));

    switch (std::rand() % 4) {
    case 0:
        return {x, inset};
    case 1:
        return {SIZE_X - inset, y};
    case 2:
        return {x, SIZE_Y - inset};
    default:
        return {inset, y};
    }
}

bool WaveController::safe(sf::Vector2f position, const SpaceShip* players,
                          int first, int end) {
    for (int i = first; i < end; ++i) {
        if (players[i].lives <= 0 || players[i].explosion) {
            continue;
        }

        const auto ship = players[i].shape.getPosition();

        float dx = std::fmod(std::abs(position.x - ship.x),
                             static_cast<float>(SIZE_X));

        float dy = std::fmod(std::abs(position.y - ship.y),
                             static_cast<float>(SIZE_Y));

        dx = std::min(dx, SIZE_X - dx);
        dy = std::min(dy, SIZE_Y - dy);

        if (dx * dx + dy * dy <
            GameRules::SafeSpawnDistance * GameRules::SafeSpawnDistance) {
            return false;
        }
    }

    return true;
}

void WaveController::update(float delta, SpaceShip* players, int first, int end,
                            std::vector<std::unique_ptr<Asteroid>>& asteroids,
                            std::vector<std::unique_ptr<Alien>>& aliens,
                            bool sound) {
    if (inRest) {
        restRemaining -= delta;

        if (restRemaining > 0.f) {
            return;
        }

        beginNextWave(-restRemaining);
    } else {
        elapsed += delta;

        if (elapsed >= GameRules::WaveSeconds) {
            beginNextWave(elapsed - GameRules::WaveSeconds);
        }
    }

    int survivors = 0;

    for (int i = first; i < end; ++i) {
        survivors += players[i].lives > 0;
    }

    const bool twoPlayers = coop && survivors > 1;

    const bool recovery =
        GameRules::waveType(level) == GameRules::WaveType::RECOVERY;

    const int rockCap = (recovery ? 12 : 24) * (twoPlayers ? 3 : 2) / 2;

    const int alienCap = recovery ? 1 : (twoPlayers ? 3 : 2);

    int rockLoad = 0;

    for (const auto& asteroid : asteroids) {
        rockLoad += asteroid->fragmentWeight();
    }

    int alienLoad = static_cast<int>(aliens.size());

    // Process warnings first. Each warning reserves capacity.
    for (auto it = warnings.begin(); it != warnings.end();) {
        const int cost = weight(it->event.kind);

        const bool cancelled =
            (it->event.coopOnly && !twoPlayers) ||
            (cost > 0 ? rockLoad + cost > rockCap : alienLoad >= alienCap);

        if (cancelled) {
            if (!it->event.coopOnly) {
                allSpawnsDelivered = false;
            }

            it = warnings.erase(it);
            continue;
        }

        it->remaining -= delta;

        if (it->remaining <= 0.f) {
            // Recheck safety at the actual spawn time.
            if (!safe(it->position, players, first, end)) {
                allSpawnsDelivered = false;
                it = warnings.erase(it);
                continue;
            }

            if (cost > 0) {
                const auto size = static_cast<AsteroidSize>(it->event.kind);

                auto asteroid = std::make_unique<Asteroid>(size);

                asteroid->initShape(level);
                asteroid->shape.setPosition(it->position);

                asteroids.push_back(std::move(asteroid));
            } else {
                const auto size = it->event.kind == Kind::BIG_ALIEN
                                      ? AlienSize::BIG
                                      : AlienSize::SMALL;

                aliens.push_back(
                    std::make_unique<Alien>(size, level, it->position));

                auto& audio = size == AlienSize::BIG ? sAlienB : sAlienS;

                auto& timer =
                    size == AlienSize::BIG ? alienBTimer : alienSTimer;

                audio.setLoop(true);
                timer.restart();

                if (sound) {
                    playSound(audio);
                }
            }

            if (cost > 0) {
                rockLoad += cost;
            } else {
                ++alienLoad;
            }

            it = warnings.erase(it);
        } else {
            if (cost > 0) {
                rockLoad += cost;
            } else {
                ++alienLoad;
            }

            ++it;
        }
    }

    while (nextEvent < events.size() && events[nextEvent].at <= elapsed) {
        const Event event = events[nextEvent];

        if (event.coopOnly && !twoPlayers) {
            ++nextEvent;
            continue;
        }

        const int cost = weight(event.kind);

        const bool canSpawn =
            cost > 0 ? rockLoad + cost <= rockCap : alienLoad < alienCap;

        sf::Vector2f position;
        bool found = false;

        if (canSpawn) {
            for (int attempt = 0; attempt < 16 && !found; ++attempt) {
                position = edgePosition();
                found = safe(position, players, first, end);
            }
        }

        if (!found) {
            if (elapsed - event.at < GameRules::SpawnRetrySeconds) {
                break;
            }

            allSpawnsDelivered = false;
            ++nextEvent;
            continue;
        }

        warnings.push_back({event, position, GameRules::SpawnWarningSeconds});

        if (cost > 0) {
            rockLoad += cost;
        } else {
            ++alienLoad;
        }

        ++nextEvent;
    }
}

int WaveController::finishFrame(bool worldEmpty) {
    if (inRest || !worldEmpty || nextEvent != events.size() ||
        !warnings.empty()) {
        return 0;
    }

    bonus = allSpawnsDelivered ? GameRules::clearBonus(secondsRemaining()) : 0;

    inRest = true;
    restRemaining = GameRules::RestSeconds;

    return bonus;
}

void WaveController::drawWarnings(sf::RenderWindow& window) const {
    sf::CircleShape marker(36.f);

    marker.setOrigin(36.f, 36.f);
    marker.setFillColor(sf::Color::Transparent);
    marker.setOutlineThickness(2.f);

    for (const auto& warning : warnings) {
        marker.setPosition(warning.position);

        marker.setOutlineColor(weight(warning.event.kind) > 0
                                   ? sf::Color(255, 190, 70)
                                   : sf::Color(255, 90, 90));

        window.draw(marker);
    }
}
