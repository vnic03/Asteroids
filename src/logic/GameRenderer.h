#pragma once

#include <memory>
#include <vector>

#include "../Asteroid.h"
#include "../alien/Alien.h"
#include "../spaceship/Spaceship.h"

class GameRenderer {
public:
    explicit GameRenderer(const sf::Font& font);

    void draw(
        sf::RenderWindow& window,
        const std::vector<std::unique_ptr<Asteroid>>& asteroids,
        const std::vector<std::unique_ptr<Alien>>& aliens,
        SpaceShip* spaceships,
        bool coop,
        int score,
        int score2,
        bool paused
    );

private:
    sf::Text pauseText;
    sf::ConvexShape lifeShape;

    void drawScore(sf::RenderWindow& window, int id, int score);

    void drawLives(
        sf::RenderWindow& window,
        const SpaceShip& spaceship,
        float y
    );
};
