#ifndef ASTEROIDS_SCENES_H
#define ASTEROIDS_SCENES_H

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "../Asteroid.h"
#include "../GameConstants.h"
#include "../HighScores.h"
#include "../alien/Alien.h"

bool startScreen(sf::RenderWindow& window, const sf::Font& font);

GameState chooseGameMode(sf::RenderWindow& window, const sf::Font& font);

std::string chooseName(sf::RenderWindow& window, const sf::Font& font);

// chooseName but double it
std::pair<std::string, std::string> chooseNames(sf::RenderWindow& window,
                                                const sf::Font& font);

void gameOver(sf::RenderWindow& window, const std::pair<int, int>& scores,
              const sf::Font& font, GameState& state, const std::string& name1,
              const std::string& name2 = "");

#endif // ASTEROIDS_SCENES_H
