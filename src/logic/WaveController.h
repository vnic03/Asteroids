#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "../Asteroid.h"
#include "../alien/Alien.h"
#include "../spaceship/Spaceship.h"


class WaveController {
public:
    explicit WaveController(bool coop);

    int currentLevel() const {
        return level;
    }

    bool resting() const {
        return inRest;
    }

    float secondsRemaining() const;

    int lastBonus() const {
        return bonus;
    }

    void update(
        float delta,
        SpaceShip* players,
        int first,
        int end,
        std::vector<std::unique_ptr<Asteroid>>& asteroids,
        std::vector<std::unique_ptr<Alien>>& aliens,
        bool sound
    );

    int finishFrame(bool worldEmpty);

    void drawWarnings(sf::RenderWindow& window) const;

private:
    enum class Kind {
        BIG_ROCK,
        MEDIUM_ROCK,
        SMALL_ROCK,
        BIG_ALIEN,
        SMALL_ALIEN
    };

    struct Event {
        float at;
        Kind kind;
        bool coopOnly = false;
    };

    struct Warning {
        Event event;
        sf::Vector2f position;
        float remaining;
    };

    bool coop;

    int level = 1;
    float elapsed = 0.f;
    float restRemaining = 0.f;

    bool inRest = false;
    bool allSpawnsDelivered = true;

    int bonus = 0;
    std::size_t nextEvent = 0;

    std::vector<Event> events;
    std::vector<Warning> warnings;

    void buildSchedule();
    void beginNextWave(float carry);

    static int weight(Kind kind);

    static bool safe(
        sf::Vector2f position,
        const SpaceShip* players,
        int first,
        int end
    );

    static sf::Vector2f edgePosition();
};
