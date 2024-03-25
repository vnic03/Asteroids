#ifndef ASTEROIDS_GAMECONSTANTS_H
#define ASTEROIDS_GAMECONSTANTS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio.hpp>

// Tracks the current state of the game
enum class GameState {
    START_SCREEN,
    RUNNING,
    GAME_OVER
};

// Window-Size
const int SIZE_X = 900;
const int SIZE_Y = 700;

// Space-Ship
const float MOVE_SPEED = 250.f;
const float ROTATION = 320.f;
const float DRAG = 0.2f;
const float RECOIL = 12.5f;

// Score-Numbers
const std::vector<std::vector<sf::Vector2f>> NUMBER_LINES = {
        { // 0
            {0.2, 0}, {0.8, 0}, {0.8, 0}, {0.8, 0.8}, {0.8, 0.8}, {0.2, 0.8},
            {0.2, 0.8}, {0.2, 0},
        },

        // 1
        { {0.5, 0}, {0.5, 0.8} },

        // 2
        { {0.1, 0}, {0.9, 0}, {0.9, 0}, {0.9, 0.4}, {0.9, 0.4}, {0.1, 0.4},
         {0.1, 0.4}, {0.1, 0.8}, {0.1, 0.8}, {0.9, 0.8} },

        { // 3
            {0.1, 0}, {0.9, 0}, {0.9, 0}, {0.9, 0.8}, {0.9, 0.8}, {0.1, 0.8},
            {0.1, 0.4}, {0.9, 0.4}
        },

        // 4
        { {0.1, 0}, {0.1, 0.4}, {0.1, 0.4}, {0.9, 0.4}, {0.5, 0}, {0.5, 0.8} },

        { // 5
            {0.9, 0}, {0.1, 0}, {0.1, 0}, {0.1, 0.4}, {0.1, 0.4}, {0.9, 0.4},
            {0.9, 0.4}, {0.9, 0.8}, {0.9, 0.8}, {0.1, 0.8}
        },

        // 6
        {
            {0.9, 0}, {0.1, 0}, {0.1, 0}, {0.1, 0.8}, {0.1, 0.8}, {0.9, 0.8},
            {0.9, 0.8}, {0.9, 0.4}, {0.9, 0.4}, {0.1, 0.4}
        },

        // 7
        { {0.1, 0}, {0.9, 0}, {0.9, 0}, {0.9, 0.8} },

        { // 8
            {0.1, 0}, {0.9, 0}, {0.9, 0}, {0.9, 0.8}, {0.9, 0.8}, {0.1, 0.8},
            {0.1, 0.8}, {0.1, 0}, {0.5, 0.4}, {0.9, 0.4}, {0.1, 0.4}, {0.5, 0.4}
        },

        { // 9
            {0.1, 0}, {0.9, 0}, {0.9, 0}, {0.9, 0.8}, {0.9, 0.8}, {0.5, 0.8},
            {0.1, 0}, {0.1, 0.4}, {0.1, 0.4}, {0.9, 0.4}
        }
};

// Size Of The Numbers
const float SCALE = 30.f;

// Game-Sounds
extern sf::SoundBuffer bBeat1, bBeat2, bSmallEx, bMediumEx, bBigEx, bFire,
bThrust, bEx, bAlienB, bAlienS, bGameOver;
extern sf::Sound sBeat1, sBeat2, sSmallEx, sMediumEx, sBigEx, sFire,
sThrust, sEx, sAlienB, sAlienS, sGameOver;

// Load sound from file
bool loadSound(sf::SoundBuffer& buffer, sf::Sound& sound, const std::string& file);

// Alternates between playing two beat sounds to create a rhythmic background
void playBeat(sf::Sound& beat1, sf::Sound& beat2);

// Timer to end loop of alien sound
extern sf::Clock alienBTimer;
extern sf::Clock alienSTimer;

#endif //ASTEROIDS_GAMECONSTANTS_H
