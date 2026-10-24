#pragma once

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace GameRules {

inline constexpr float Pi = 3.14159265358979323846f;

// Simulation
inline constexpr float MaxFrameDelta = 0.05f;

// Waves and spawning
inline constexpr float WaveSeconds = 30.f;
inline constexpr float RestSeconds = 5.f;
inline constexpr float SpawnWarningSeconds = 1.2f;
inline constexpr float SpawnRetrySeconds = 3.f;
inline constexpr float SafeSpawnDistance = 180.f;

// Player
inline constexpr int StartingLives = 3;
inline constexpr int MaxLives = 5;
inline constexpr float RespawnSeconds = 2.f;
inline constexpr float InvincibleSeconds = 3.f;

// Player projectiles
inline constexpr float PlayerProjectileLifetime = 1.2f;
inline constexpr float PlayerProjectileSpeed = 500.f;
inline constexpr float PlayerProjectileRadius = 2.f;

// Aliens
inline constexpr int BigAlienLives = 6;
inline constexpr int SmallAlienLives = 2;

inline constexpr float BigAlienSpeed = 50.f;
inline constexpr float SmallAlienSpeed = 100.f;
inline constexpr float SmallAlienDirectionInterval = 0.9f;

inline constexpr float AlienChargeSeconds = 0.55f;
inline constexpr float AlienSpawnGrace = 1.5f;

inline constexpr float AlienProjectileLifetime = 30.f;
inline constexpr float BigAlienProjectileSpeed = 80.f;
inline constexpr float SmallAlienProjectileSpeed = 120.f;
inline constexpr float BigAlienProjectileRadius = 8.f;
inline constexpr float SmallAlienProjectileRadius = 5.f;

inline constexpr int BigAlienScore = 300;
inline constexpr int SmallAlienScore = 600;

// Audio
inline constexpr float AlienSoundSeconds = 1.8f;
inline constexpr float InitialBeatInterval = 1.f;
inline constexpr float MinimumBeatInterval = 0.35f;
inline constexpr float BeatAccelerationInterval = 5.f;
inline constexpr float BeatAccelerationFactor = 0.9f;

enum class WaveType { INTRO, MIXED, FIELD, HUNT, DEBRIS, RECOVERY, ASSAULT };

inline WaveType waveType(int wave) {
    if (wave == 1)
        return WaveType::INTRO;
    if (wave <= 5)
        return WaveType::MIXED;

    switch (wave % 5) {
    case 0:
        return WaveType::ASSAULT;
    case 1:
        return WaveType::RECOVERY;
    case 2:
        return WaveType::FIELD;
    case 3:
        return WaveType::DEBRIS;
    default:
        return WaveType::HUNT;
    }
}

inline const char* waveName(int wave) {
    switch (waveType(wave)) {
    case WaveType::INTRO:
        return "FIRST CONTACT";
    case WaveType::FIELD:
        return "ASTEROID FIELD";
    case WaveType::HUNT:
        return "ALIEN HUNT";
    case WaveType::DEBRIS:
        return "FAST DEBRIS";
    case WaveType::RECOVERY:
        return "BREATHER";
    case WaveType::ASSAULT:
        return "ALIEN ASSAULT";
    default:
        return "MIXED ATTACK";
    }
}

inline int asteroidCount(int wave) {
    if (wave <= 5) {
        constexpr int counts[] = {4, 5, 6, 6, 8};
        return counts[wave - 1];
    }

    const int growth = std::min(3, (wave - 6) / 5);

    switch (waveType(wave)) {
    case WaveType::RECOVERY:
        return 5;
    case WaveType::FIELD:
        return 8 + growth;
    case WaveType::DEBRIS:
        return 7 + growth;
    case WaveType::HUNT:
        return 5 + growth;
    default:
        return 6 + growth;
    }
}

// size: BIG = 0, MEDIUM = 1, SMALL = 2.
// The returned value is the actual velocity magnitude.
inline float asteroidSpeed(int size, int wave) {
    constexpr float base[] = {45.f, 80.f, 120.f};
    constexpr float maximum[] = {80.f, 140.f, 200.f};

    const float progress = std::clamp((wave - 1) / 19.f, 0.f, 1.f);

    const float speed = base[size] + (maximum[size] - base[size]) * progress;

    const float variation = 0.9f + (std::rand() % 201) / 1000.f;

    return std::min(maximum[size], speed * variation);
}

inline float alienShootInterval(bool big, int wave) {
    const int progress = std::clamp(wave - 1, 0, 20);

    return big ? std::max(2.5f, 3.f - progress * 0.025f)
               : std::max(1.4f, 2.2f - progress * 0.04f);
}

inline int clearBonus(float remaining) {
    return 100 + 10 * static_cast<int>(std::max(0.f, remaining));
}

} // namespace GameRules
