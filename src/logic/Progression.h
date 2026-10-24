#pragma once

#include <array>
#include <cstdint>
#include <limits>

#include "../spaceship/Spaceship.h"
#include "GameRules.h"

class Progression {
  public:
    static void addScore(int& score, int amount) {
        const auto sum = static_cast<std::int64_t>(score) + amount;

        score = static_cast<int>(
            std::min<std::int64_t>(sum, std::numeric_limits<int>::max()));
    }

    void update(SpaceShip& ship, int score) {
        auto& next = thresholds[ship.id];

        while (score >= next) {
            if (ship.lives > 0 && ship.lives < GameRules::MaxLives) {
                ++ship.lives;
            }

            if (next == 3000)
                next = 7000;
            else if (next == 7000)
                next = 12000;
            else
                next += 6000;
        }
    }

    std::int64_t nextLife(int id) const { return thresholds[id]; }

  private:
    std::array<std::int64_t, 3> thresholds{3000, 3000, 3000};
};
