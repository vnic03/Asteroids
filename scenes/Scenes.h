#ifndef ASTEROIDS_SCENES_H
#define ASTEROIDS_SCENES_H

#include <string>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include "../GameConstants.h"
#include "../Asteroid.h"
#include "../alien/Alien.h"
#include "../Config.cpp"

// Tracks highscores for different players
struct Score {
    std::string name;
    int score;
};

// Displays the start screen and waits for user input to start the game
bool startScreen(sf::RenderWindow &window, const sf::Font &font);

// Let the user decide between single player and co-op mode
GameState chooseGameMode(sf::RenderWindow &window, const sf::Font &font);

/*
 *  Makes the user to choose or enter a name.
 * - Type to enter a new name.
 * - Use up/down arrows to select from existing names.
 * - Press Enter to confirm.
 * - Press Delete to remove a selected name.
 */
std::string chooseName(sf::RenderWindow &window, const sf::Font &font);

// chooseName but double it
std::pair<std::string, std::string> chooseNames(sf::RenderWindow &window, const sf::Font &font);

// removes a player and their score
void removePlayer(std::vector<Score>& scores, const std::string& name);

// Displays the game over screen, showing the final score and a prompt to play again
void gameOver(sf::RenderWindow &window, int score,
              const sf::Font& font, GameState& state, const std::string& name);

// Reads the current Highscores from a text file
std::vector<Score>readHighScores();

// Writes the Highscore in the same text file
void writeHighScore(const std::string& name, int score);

#endif //ASTEROIDS_SCENES_H
