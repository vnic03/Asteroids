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
std::pair<int, int> runGame(sf::RenderWindow &window, const sf::Font& font,
                            GameState& state, int &score, int &score2);

// Handles each Players controller input
void handlePlayerInput(SpaceShip& spaceship, float delta, bool sound,
                       sf::Keyboard::Key left, sf::Keyboard::Key right,
                       sf::Keyboard::Key thrust, sf::Keyboard::Key fire);

// Checks for collision between two circular objects
bool checkCollision(const sf::Shape& object1, float radius1,
                    const sf::Shape& object2, float radius2);

// Updates the positions and states of all asteroids, handles collision with the spaceship
void updateAsteroids(std::vector<std::unique_ptr<Asteroid>>& asteroids,
                     SpaceShip& spaceship, float delta, int &score, int &level, bool sound);

// Updates the positions and states of all aliens, handles alien logic and collision with the spaceship
void updateAliens(std::vector<std::unique_ptr<Alien>>& aliens,
                     SpaceShip& spaceship, float delta, int &score, bool sound);

// Adds a new alien to the game at a random location
void addAlien(std::vector<std::unique_ptr<Alien>>& aliens, AlienSize size, float delta, bool sound);

// Draws the current score on the screen
void drawScore(sf::RenderWindow &window, int id, int score);

// Draws the remaining lives of the spaceship on the screen
void drawLives(sf::RenderWindow &window, const SpaceShip& spaceship, int y = 28);

// For Coop (calls drawLives method twice, with different y-offset)
void drawLivesCoop(sf::RenderWindow &window, const SpaceShip* spaceships);

// Handles user input for controlling the spaceship with a Controller
void controllerInput(SpaceShip* spaceships, bool& paused, float delta, bool& coop, bool sound);


#endif //ASTEROIDS_LOGIC_H
