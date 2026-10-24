#pragma once

#include <SFML/Graphics.hpp>
#include <utility>

#include "../GameConstants.h"

std::pair<int, int> runGame(sf::RenderWindow& window, const sf::Font& font,
                            GameState& state, int& score, int& score2);
