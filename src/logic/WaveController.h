#pragma once

#include <optional>

#include "../SimulationClock.h"
#include "../alien/Alien.h"

class WaveController {
public:
    explicit WaveController(bool coop);

    int currentLevel() const;

    bool shouldStartNextWave(bool enemiesCleared) const;

    void beginNextWave();

    std::optional<AlienSize> pendingAlienSpawn() const;

    void confirmAlienSpawn();

private:
    bool coop;
    int level = 1;
    int aliensSpawned = 0;

    SimulationClock alienSpawnTimer;
    float nextAlienSpawnTime;
};
