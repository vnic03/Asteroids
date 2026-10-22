#ifndef ASTEROIDS_LOGIC_H
#define ASTEROIDS_LOGIC_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <SFML/Graphics/Shape.hpp>
#include <sstream>
#include <vector>
#include <memory>

#include "../Asteroid.h"
#include "../spaceship/Spaceship.h"
#include "../alien/Alien.h"


std::pair<int, int> runGame(
        sf::RenderWindow &window,
        const sf::Font& font,
        GameState& state,
        int &score,
        int &score2
);

void handlePlayerInput(
        SpaceShip& spaceship,
        float delta,
        sf::Keyboard::Key left, sf::Keyboard::Key right,
        sf::Keyboard::Key thrust
);

bool checkCollision(
        const sf::Shape& object1, float radius1,
        const sf::Shape& object2, float radius2
);

void resolveAsteroidCollisions(
    std::vector<std::unique_ptr<Asteroid>>& asteroids,
    SpaceShip& spaceship,
    int& score,
    int& level,
    bool sound
);

void resolveAlienCollisions(
    std::vector<std::unique_ptr<Alien>>& aliens,
    SpaceShip& spaceship,
    int& score,
    bool sound
);

void addAlien(
    std::vector<std::unique_ptr<Alien>>& aliens,
    AlienSize size,
    bool sound
);

// Draws the current score on the screen
void drawScore(sf::RenderWindow &window, int id, int score);

// Draws the remaining lives of the spaceship on the screen
void drawLives(sf::RenderWindow &window, const SpaceShip& spaceship, int y = 28);

// For Coop (calls drawLives method twice, with different y-offset)
void drawLivesCoop(sf::RenderWindow &window, const SpaceShip* spaceships);


#endif //ASTEROIDS_LOGIC_H
