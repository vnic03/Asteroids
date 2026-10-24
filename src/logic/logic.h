#pragma once

#include <utility>
#include <SFML/Graphics.hpp>

#include "../GameConstants.h"

std::pair<int, int> runGame(
    sf::RenderWindow& window,
    const sf::Font& font,
    GameState& state,
    int& score,
    int& score2
);
