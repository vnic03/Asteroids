#ifndef ASTEROIDS_LOGIC_H
#define ASTEROIDS_LOGIC_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <SFML/Graphics/Shape.hpp>
#include <sstream>

#include "../Asteroid.h"
#include "../spaceship/Spaceship.h"
#include "../alien/Alien.h"

// Main game loop: handles game logic, rendering, and user input
int runGame(sf::RenderWindow &window, const sf::Font& font, GameState& state);

// Checks for collision between two circular objects
bool checkCollision(const sf::Shape& object1, float radius1,
                    const sf::Shape& object2, float radius2);

// Updates the positions and states of all asteroids, handles collision with the spaceship
void updateAsteroids(std::vector<std::unique_ptr<Asteroid>>& asteroids,
                     SpaceShip& spaceship, float delta, int &score, int &level);

// Updates the positions and states of all aliens, handles alien logic and collision with the spaceship
void updateAliens(std::vector<std::unique_ptr<Alien>>& aliens,
                     SpaceShip& spaceship, float delta, int &score);

// Adds a new alien to the game at a random location
void addAlien(std::vector<std::unique_ptr<Alien>>& aliens, AlienSize size,
              float delta);

// Draws the current score on the screen
void drawScore(sf::RenderWindow &window, int score);

// Draws a single digit of the score on the screen at the specified position
void drawNumber(sf::RenderWindow &window, int n, sf::Vector2f pos);

// Draws the remaining lives of the spaceship on the screen
void drawLives(sf::RenderWindow &window, const SpaceShip& spaceship);

// Handles user input for controlling the spaceship with a Controller
void controllerInput(SpaceShip& spaceship, bool& paused, float delta);

#endif //ASTEROIDS_LOGIC_H
