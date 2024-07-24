#ifndef ASTEROIDS_SPACESHIP_H
#define ASTEROIDS_SPACESHIP_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include "../MovableEntity.h"


// Represents a fragment of an explosion, seen as lines in game
struct ExplosionFragment {
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed;
    sf::Color color;

    ExplosionFragment(sf::Vector2f pos, sf::Vector2f dir, float speed, sf::Color color)
            : position(pos), direction(dir), speed(speed), color(color) { }
};

class SpaceShip : public MovableEntity {
public:
    explicit SpaceShip(int id = 0);

    // id to differ between single and coop mode
    int id; // 0: Single-Player, 1&2: Coop-Mode

    sf::ConvexShape engine; // engine shape

    // Engine on/off flags - for keyboard and controller
    bool engineOn = false;
    bool engineOnC = false;

    float radius = 10.f;

    sf::Clock respawnTime; // respawn delay timer for spaceship

    std::vector<ExplosionFragment> fragments;

    // Gives player time to get ready after respawn
    bool invincible = false;
    sf::Clock invincibleTimer;

    // Makes spamming the shooting key impossible
    bool keyClicked = false;

    SpaceShip();

    void draw(sf::RenderWindow &window) override;

    void update(float delta) override;

    void shoot(std::optional<sf::Vector2f> pos = std::nullopt) override;

    void explode() override;

    // Makes the spaceship able to rotate
    void rotate(float delta);

    // toggles the engine on and off
    void toggleEngine(bool active);
    // for Controller
    void toggleEngineC(bool active);

    // Single-Player: White
    // COOP: Player 1 gets Red and Player 2 gets Cyan
     sf::Color handleColor() const {
        return (id == 1) ? PLAYER_1_COLOR : ((id == 2) ? PLAYER_2_COLOR : WHITE);
    }

private:
    void initShape() override;

    // initializes the shape of the engine.
    void initEngineShape();

    // makes the engine more realistic.
    void updateEngine();

    // handles respawn
    void respawn();
};

#endif //ASTEROIDS_SPACESHIP_H
