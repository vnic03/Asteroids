#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

#include "GameConstants.h"
#include "logic/logic.h"
#include "scenes/Scenes.h"

// Game-Sounds
sf::SoundBuffer bBeat1, bBeat2, bSmallEx, bMediumEx, bBigEx, bFire, bThrust, bEx,
    bAlienB, bAlienS, bGameOver;
sf::Sound sBeat1, sBeat2, sSmallEx, sMediumEx, sBigEx, sFire, sThrust, sEx,
        sAlienB, sAlienS, sGameOver;

// Timer to end loop of alien sound
sf::Clock alienBTimer;
sf::Clock alienSTimer;

bool loadSound(sf::SoundBuffer& buffer, sf::Sound& sound, const std::string& file)
{
    if (!buffer.loadFromFile("assets/sound/" + file)) {
        std::cerr << "Error: " << file << std::endl;
        return false;
    }
    sound.setBuffer(buffer);
    return true;
}

void playBeat(sf::Sound& beat1, sf::Sound& beat2) {
    static bool playFirst = true;
    if (playFirst) {
        if (beat1.getStatus() != sf::Sound::Playing) {
            beat1.play();
        }
    } else {
        if (beat2.getStatus() != sf::Sound::Playing) {
            beat2.play();
        }
    }
    playFirst = !playFirst;
}

// Main function
int main() {
    // Window
    sf::RenderWindow window(sf::VideoMode(SIZE_X, SIZE_Y), "Asteroids");
    window.setFramerateLimit(60);

    // Retro-Arcade-Font
    sf::Font font;
    if (!font.loadFromFile("assets/font/arcade-font.otf")) return 1;

    // Load Game-Sound
    if (!loadSound(bSmallEx, sSmallEx, "bang-small.wav") ||
        !loadSound(bMediumEx, sMediumEx, "bang-medium.wav") ||
        !loadSound(bBigEx, sBigEx, "bang-large.wav") ||
        !loadSound(bFire, sFire, "fire.wav") ||
        !loadSound(bThrust, sThrust, "thrust.wav") ||
        !loadSound(bBeat1, sBeat1, "beat1.wav") ||
        !loadSound(bBeat2, sBeat2, "beat2.wav") ||
        !loadSound(bEx, sEx, "spaceship-explosion.wav") ||
        !loadSound(bAlienB, sAlienB, "alien-big.wav") ||
        !loadSound(bAlienS,sAlienS, "alien-small.wav") ||
        !loadSound(bGameOver, sGameOver, "game-over.wav"))
    {
        return 1;
    }
    // Shooting sound is too loud, relative to other sounds
    sFire.setVolume(60);

    // Game-Loop
    GameState state = GameState::START_SCREEN;
    int score = 0;
    std::string player;

    while (window.isOpen()) {
        sf::Event event{ };
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)  window.close();
        }
        switch (state) {
            case GameState::START_SCREEN:
                if (startScreen(window, font)) state = GameState::CHOOSE_NAME;
                else window.close();
                break;
            case GameState::CHOOSE_NAME:
            {
                std::string name = chooseName(window, font);
                if (!name.empty()) {
                    player = name; state = GameState::RUNNING;
                } else return 0;
            }
                break;
            case GameState::RUNNING:
                score = runGame(window, font, state);
                state = GameState::GAME_OVER;
                break;
            case GameState::GAME_OVER:
                sAlienS.stop(); sAlienB.stop();
                gameOver(window, score, font, state, player);
                state = GameState::CHOOSE_NAME;
                break;
        }
    }
    return 0;
}
