#pragma once

#include <SFML/System/Time.hpp>

class SimulationClock {
public:
    SimulationClock() : startedAt(now) {}

    sf::Time getElapsedTime() const {
        return sf::seconds(static_cast<float>(now - startedAt));
    }

    sf::Time restart() {
        const auto elapsed = getElapsedTime();
        startedAt = now;
        return elapsed;
    }

    static void advance(float seconds) {
        if (seconds > 0.f) {
            now += seconds;
        }
    }

private:
    inline static double now = 0.0;
    double startedAt;
};
