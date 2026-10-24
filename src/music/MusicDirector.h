#pragma once

#include <SFML/Audio/Sound.hpp>

#include <memory>
#include <vector>

class Asteroid;
class Alien;
class SpaceShip;
class WaveController;

class MusicDirector {
  public:
    MusicDirector(sf::Sound& first, sf::Sound& second);

    void update(float delta, const WaveController& waves,
                const std::vector<std::unique_ptr<Asteroid>>& asteroids,
                const std::vector<std::unique_ptr<Alien>>& aliens,
                const SpaceShip* players, int firstPlayer, int endPlayer,
                bool enabled);

    void suspend();

  private:
    sf::Sound& first;
    sf::Sound& second;

    float intensity = 0.08f;
    float phase = 0.f;

    int beatIndex = 0;

    bool denseRhythm = false;
    bool offbeatPlayed = false;

    float evaluate(const WaveController& waves,
                   const std::vector<std::unique_ptr<Asteroid>>& asteroids,
                   const std::vector<std::unique_ptr<Alien>>& aliens,
                   const SpaceShip* players, int firstPlayer,
                   int endPlayer) const;

    void play(sf::Sound& voice, float volume, float pitch);
};
