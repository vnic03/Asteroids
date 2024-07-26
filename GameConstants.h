#ifndef ASTEROIDS_GAMECONSTANTS_H
#define ASTEROIDS_GAMECONSTANTS_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio.hpp>


// Tracks the current state of the game
enum class GameState {
    START_SCREEN,
    CHOOSE_NAME,
    RUNNING,
    GAME_OVER,
    COOP_CHOOSE_NAME,
    COOP_RUNNING
};

// Window-Size
const int SIZE_X = 900;
const int SIZE_Y = 700;

// Space-Ship
const float MOVE_SPEED = 250.f;
const float ROTATION = 320.f;
const float DRAG = 0.2f;
const float RECOIL = 12.5f;

/*
    Controls
*/
const sf::Keyboard::Key PAUSE = sf::Keyboard::Key::P;

// Turns Sound on/off when pressing M
const sf::Keyboard::Key SOUND = sf::Keyboard::Key::M;

// Single-Player and Player 1
const sf::Keyboard::Key UP = sf::Keyboard::Key::W;
const sf::Keyboard::Key LEFT = sf::Keyboard::Key::A;
const sf::Keyboard::Key RIGHT = sf::Keyboard::Key::D;
const sf::Keyboard::Key SHOOT = sf::Keyboard::Key::Space;
// COOP Player 2
const sf::Keyboard::Key COOP_UP = sf::Keyboard::Key::Up;
const sf::Keyboard::Key COOP_LEFT = sf::Keyboard::Key::Left;
const sf::Keyboard::Key COOP_RIGHT = sf::Keyboard::Key::Right;
const sf::Keyboard::Key COOP_SHOOT = sf::Keyboard::Key::RControl;


// COLORS
inline const sf::Color& white() {
    static const sf::Color white = sf::Color::White;
    return white;
}
inline const sf::Color& player1Color() {
    static const sf::Color player1Color = sf::Color::Red;
    return player1Color;
}
inline const sf::Color& player2Color() {
    static const sf::Color player2Color = sf::Color::Cyan;
    return player2Color;
}
// Single-Player
const sf::Color WHITE = white();
// COOP
const sf::Color PLAYER_1_COLOR = player1Color();
const sf::Color PLAYER_2_COLOR = player2Color();


// Size Of The Numbers
const float NUMBER_SCALE = 30.f;

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

// Game-Sounds
extern sf::SoundBuffer bBeat1, bBeat2, bSmallEx, bMediumEx, bBigEx, bFire,
bThrust, bEx, bAlienB, bAlienS, bGameOver;
extern sf::Sound sBeat1, sBeat2, sSmallEx, sMediumEx, sBigEx, sFire,
sThrust, sEx, sAlienB, sAlienS, sGameOver;

// Load sound from file
bool loadSound(sf::SoundBuffer& buffer, sf::Sound& sound, const std::string& file);

// Alternates between playing two beat sounds to create a rhythmic background
void playBeat(sf::Sound& beat1, sf::Sound& beat2, bool sound);

// Timer to end loop of alien sound
extern sf::Clock alienBTimer;
extern sf::Clock alienSTimer;

#endif //ASTEROIDS_GAMECONSTANTS_H
