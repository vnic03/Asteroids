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
#include "../HighScores.h"


bool startScreen(sf::RenderWindow &window, const sf::Font &font);

GameState chooseGameMode(sf::RenderWindow &window, const sf::Font &font);

std::string chooseName(sf::RenderWindow &window, const sf::Font &font);

// chooseName but double it
std::pair<std::string, std::string> chooseNames(sf::RenderWindow &window, const sf::Font &font);

void gameOver(sf::RenderWindow &window, const std::pair<int, int>& scores, const sf::Font& font,
              GameState& state, const std::string& name1, const std::string& name2 = "");


#endif //ASTEROIDS_SCENES_H
