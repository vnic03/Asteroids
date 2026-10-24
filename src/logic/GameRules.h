#pragma once

#include <cstdlib>

namespace GameRules {

// Frame timing
inline constexpr float MaxFrameDelta = 0.05f;

// Player
inline constexpr int StartingLives = 3;
inline constexpr float RespawnSeconds = 2.f;
inline constexpr float InvincibleSeconds = 3.f;

// Projectiles
inline constexpr float PlayerProjectileLifetime = 1.2f;
inline constexpr float AlienProjectileLifetime = 30.f;
inline constexpr float PlayerProjectileSpeed = 500.f;
inline constexpr float PlayerProjectileRadius = 2.f;

// Asteroids
inline constexpr float BigAsteroidSpeed = 50.f;
inline constexpr float MediumAsteroidSpeed = 120.f;
inline constexpr float SmallAsteroidSpeed = 200.f;

// Aliens
inline constexpr int MaxAliensPerWave = 2;

inline constexpr int BigAlienLives = 10;
inline constexpr int SmallAlienLives = 2;

inline constexpr float BigAlienSpeed = 50.f;
inline constexpr float SmallAlienSpeed = 100.f;

inline constexpr float BigAlienShootInterval = 3.f;
inline constexpr float SmallAlienShootInterval = 1.5f;
inline constexpr float SmallAlienDirectionInterval = 0.5f;

inline constexpr int BigAlienShotCount = 3;
inline constexpr int SmallAlienShotCount = 1;

inline constexpr float BigAlienProjectileSpeed = 80.f;
inline constexpr float SmallAlienProjectileSpeed = 120.f;

inline constexpr float BigAlienProjectileRadius = 8.f;
inline constexpr float SmallAlienProjectileRadius = 5.f;

// Audio timing
inline constexpr float AlienSoundSeconds = 1.8f;
inline constexpr float InitialBeatInterval = 1.f;
inline constexpr float MinimumBeatInterval = 0.1f;
inline constexpr float BeatAccelerationInterval = 5.f;
inline constexpr float BeatAccelerationFactor = 0.9f;

inline int baseAsteroidAmount(bool coop) {
    return coop ? 12 : 10;
}

// Preserve the existing behavior:
// from level 4 onward, add one asteroid to the base amount.
inline int asteroidAmount(int baseAmount, int level) {
    if (level > 3 && baseAmount <= 20) {
        return baseAmount + 1;
    }

    return baseAmount;
}

inline float asteroidSpeedMultiplier(int level) {
    if (level <= 10) {
        return 1.f + (level - 1) * 0.1f;
    }

    if (level <= 20) {
        return 1.9f + (level - 10) * 0.05f;
    }

    return 2.4f + (level - 20) * 0.01f;
}

inline bool grantsExtraLife(int level) {
    return (level % 2 == 0 && level < 10) ||
           (level % 4 == 2 && level >= 10);
}

inline float nextAlienSpawnDelay(bool coop, bool firstSpawn) {
    // The first spawn of a new game previously used 8–12 seconds,
    // including in cooperative mode.
    const int minimum = firstSpawn ? 8 : (coop ? 6 : 8);

    return static_cast<float>((std::rand() % 5) + minimum);
}

} // namespace GameRules
