#include "MusicDirector.h"

#include <algorithm>
#include <cmath>

#include "../Asteroid.h"
#include "../alien/Alien.h"
#include "../logic/WaveController.h"
#include "../spaceship/Spaceship.h"
#include "AudioSettings.h"

namespace {

// Tempo: main pulses per minute.
constexpr float MinBpm = 60.f;
constexpr float MaxBpm = 168.f;

// SFML volume range: 0 to 100.
constexpr float MinVolume = 32.f;
constexpr float MaxVolume = 62.f;

// Response times for smoothing intensity.
constexpr float AttackSeconds = 0.8f;
constexpr float ReleaseSeconds = 3.5f;
constexpr float RestReleaseSeconds = 1.5f;

// Different thresholds prevent rapid rhythm switching.
constexpr float DenseEnter = 0.74f;
constexpr float DenseExit = 0.60f;

// Audio never catches up by playing a backlog of beats.
constexpr float MaxAudioDelta = 0.25f;

float proximity(sf::Vector2f a, sf::Vector2f b, float radius, float range,
                bool wraps) {
    float dx = std::abs(a.x - b.x);
    float dy = std::abs(a.y - b.y);

    if (wraps) {
        dx = std::fmod(dx, static_cast<float>(SIZE_X));
        dy = std::fmod(dy, static_cast<float>(SIZE_Y));

        dx = std::min(dx, SIZE_X - dx);
        dy = std::min(dy, SIZE_Y - dy);
    }

    const float clearance =
        std::max(0.f, std::sqrt(dx * dx + dy * dy) - radius);

    return std::clamp(1.f - clearance / range, 0.f, 1.f);
}

} // namespace

MusicDirector::MusicDirector(sf::Sound& first, sf::Sound& second)
    : first(first), second(second) {}

float MusicDirector::evaluate(
    const WaveController& waves,
    const std::vector<std::unique_ptr<Asteroid>>& asteroids,
    const std::vector<std::unique_ptr<Alien>>& aliens, const SpaceShip* players,
    int firstPlayer, int endPlayer) const {
    if (waves.resting()) {
        return 0.03f;
    }

    const float progress =
        std::clamp((waves.currentLevel() - 1) / 19.f, 0.f, 1.f);

    if (asteroids.empty() && aliens.empty()) {
        return 0.06f + progress * 0.04f;
    }

    int survivors = 0;
    bool critical = false;

    for (int i = firstPlayer; i < endPlayer; ++i) {
        if (players[i].lives <= 0) {
            continue;
        }

        ++survivors;

        if (!players[i].explosion && !players[i].invincible) {
            critical |= players[i].lives == 1;
        }
    }

    float load = 0.f;
    float nearby = 0.f;

    for (const auto& asteroid : asteroids) {
        const float speed =
            std::sqrt(asteroid->velocity.x * asteroid->velocity.x +
                      asteroid->velocity.y * asteroid->velocity.y);

        const float sizeWeight = asteroid->size == AsteroidSize::BIG ? 0.8f
                                 : asteroid->size == AsteroidSize::MEDIUM
                                     ? 1.f
                                     : 1.3f;

        load += sizeWeight * std::clamp(speed / 100.f, 0.7f, 1.5f);

        for (int i = firstPlayer; i < endPlayer; ++i) {
            const auto& ship = players[i];

            if (ship.lives <= 0 || ship.explosion || ship.invincible) {
                continue;
            }

            nearby = std::max(nearby,
                              proximity(asteroid->shape.getPosition(),
                                        ship.shape.getPosition(),
                                        asteroid->collisionRadius + ship.radius,
                                        220.f, true));
        }
    }

    for (const auto& alien : aliens) {
        if (alien->isDestroyed()) {
            continue;
        }

        load += alien->alienSize == AlienSize::BIG ? 3.f : 4.5f;

        load += static_cast<float>(alien->projectiles.size()) * 0.25f;

        for (int i = firstPlayer; i < endPlayer; ++i) {
            const auto& ship = players[i];

            if (ship.lives <= 0 || ship.explosion || ship.invincible) {
                continue;
            }

            nearby = std::max(
                nearby,
                proximity(alien->shape.getPosition(), ship.shape.getPosition(),
                          alien->collisionRadius + ship.radius, 200.f, true));

            for (const auto& shot : alien->projectiles) {
                nearby = std::max(
                    nearby, proximity(shot.position, ship.shape.getPosition(),
                                      shot.radius + ship.radius, 180.f, false));
            }
        }
    }

    // Cooperative enemy counts should not immediately max out the music.
    const float capacity = survivors > 1 ? 27.f : 18.f;

    const float pressure = std::clamp(load / capacity, 0.f, 1.f);
    const float danger = nearby * (critical ? 0.32f : 0.24f);

    return std::clamp(0.12f + progress * 0.18f + pressure * 0.43f + danger, 0.f,
                      1.f);
}

void MusicDirector::play(sf::Sound& voice, float volume, float pitch) {
    if (voice.getBuffer() == nullptr ||
        voice.getStatus() == sf::Sound::Playing) {
        return;
    }

    voice.setLoop(false);
    voice.setVolume(std::clamp(
        volume * AudioSettings::Master * AudioSettings::Music, 0.f, 100.f));
    voice.setPitch(pitch);
    voice.play();
}

void MusicDirector::suspend() {
    if (first.getStatus() != sf::Sound::Stopped) {
        first.stop();
    }

    if (second.getStatus() != sf::Sound::Stopped) {
        second.stop();
    }

    phase = 0.f;
    offbeatPlayed = false;
}

void MusicDirector::update(
    float delta, const WaveController& waves,
    const std::vector<std::unique_ptr<Asteroid>>& asteroids,
    const std::vector<std::unique_ptr<Alien>>& aliens, const SpaceShip* players,
    int firstPlayer, int endPlayer, bool enabled) {
    const float dt = std::clamp(delta, 0.f, MaxAudioDelta);

    const float target =
        evaluate(waves, asteroids, aliens, players, firstPlayer, endPlayer);

    const float response =
        target > intensity
            ? AttackSeconds
            : (waves.resting() ? RestReleaseSeconds : ReleaseSeconds);

    const float blend = 1.f - std::exp(-dt / response);

    intensity += (target - intensity) * blend;

    if (!denseRhythm && intensity >= DenseEnter) {
        denseRhythm = true;
    } else if (denseRhythm && intensity <= DenseExit) {
        denseRhythm = false;
    }

    // Keep following gameplay while muted, but do not queue sounds.
    if (!enabled) {
        suspend();
        return;
    }

    if (dt <= 0.f) {
        return;
    }

    const float bpm = MinBpm + (MaxBpm - MinBpm) * intensity;

    const float volume = MinVolume + (MaxVolume - MinVolume) * intensity;

    const float pitch = 0.98f + intensity * 0.06f;

    phase += dt * bpm / 60.f;

    if (phase >= 1.f) {
        phase = std::fmod(phase, 1.f);

        // A stronger first pulse creates a four-beat phrase.
        const float accent = beatIndex == 0 ? 1.f : 0.82f;

        auto& voice = beatIndex % 2 == 0 ? first : second;

        play(voice, volume * accent, pitch);

        beatIndex = (beatIndex + 1) % 4;
        offbeatPlayed = false;
    } else if (denseRhythm && !offbeatPlayed && phase >= 0.5f) {
        // Quiet extra pulses during sustained high pressure.
        play(second, volume * 0.32f, pitch);
        offbeatPlayed = true;
    }
}
