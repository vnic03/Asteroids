#ifndef ASTEROIDS_SPACESHIP_H
#define ASTEROIDS_SPACESHIP_H

#include "../MovableEntity.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>

struct ExplosionFragment {
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed;
    sf::Color color;

    ExplosionFragment(sf::Vector2f pos, sf::Vector2f dir, float speed,
                      sf::Color color)
        : position(pos), direction(dir), speed(speed), color(color) {}
};

class SpaceShip : public MovableEntity {
  public:
    explicit SpaceShip(int id = 0);

    // id to differ between single and coop mode
    int id; // 0: Single-Player, 1&2: Coop-Mode

    sf::ConvexShape engine;

    bool engineOn = false;
    bool engineOnC = false; // another flag for controller

    float radius = 10.f;

    SimulationClock respawnTime;

    SimulationClock invincibleTimer;

    std::vector<ExplosionFragment> fragments;

    bool invincible = false;

    void draw(sf::RenderWindow& window) override;

    void update(float delta, bool sound) override;

    void shoot(bool sound,
               std::optional<sf::Vector2f> pos = std::nullopt) override;

    void explode(bool sound) override;

    void rotate(float delta);

    void toggleEngine(bool active);

    void toggleEngineC(bool active); // for controller

    // Single-Player: White
    // COOP: Player 1 gets Red and Player 2 gets Cyan
    sf::Color handleColor() const {
        return (id == 1) ? PLAYER_1_COLOR
                         : ((id == 2) ? PLAYER_2_COLOR : WHITE);
    }

  private:
    void initShape() override;

    void initEngineShape();

    // makes the engine more realistic.
    void updateEngine();

    void respawn();
};

#endif // ASTEROIDS_SPACESHIP_H
