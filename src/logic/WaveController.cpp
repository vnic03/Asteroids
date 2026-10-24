#include "WaveController.h"

#include "GameRules.h"

WaveController::WaveController(bool coop)
    : coop(coop),
      nextAlienSpawnTime(GameRules::nextAlienSpawnDelay(coop, true)) {
}

int WaveController::currentLevel() const {
    return level;
}

bool WaveController::shouldStartNextWave(bool enemiesCleared) const {
    // TODO: add timed wave condition here during balancing.
    return enemiesCleared;
}

void WaveController::beginNextWave() {
    ++level;
    aliensSpawned = 0;
    alienSpawnTimer.restart();

    nextAlienSpawnTime =
        GameRules::nextAlienSpawnDelay(coop, false);
}

std::optional<AlienSize> WaveController::pendingAlienSpawn() const {
    if (aliensSpawned >= GameRules::MaxAliensPerWave) {
        return std::nullopt;
    }

    if (alienSpawnTimer.getElapsedTime().asSeconds() <=
        nextAlienSpawnTime) {
        return std::nullopt;
    }

    return aliensSpawned == 0 ? AlienSize::BIG : AlienSize::SMALL;
}

void WaveController::confirmAlienSpawn() {
    ++aliensSpawned;
    alienSpawnTimer.restart();

    nextAlienSpawnTime = GameRules::nextAlienSpawnDelay(coop, false);
}
