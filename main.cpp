#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>

#include "GameConstants.h"
#include "logic/logic.h"
#include "scenes/Scenes.h"


#define COOP_SIZE_X (SIZE_X + 250)
#define COOP_SIZE_Y (SIZE_Y + 100)

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
        !loadSound(bAlienS, sAlienS, "alien-small.wav") ||
        !loadSound(bGameOver, sGameOver, "game-over.wav"))
    {
        return 1;
    }
    // Shooting sound is too loud, relative to other sounds
    sFire.setVolume(60);

    // Game-Loop
    GameState state = GameState::START_SCREEN;
    std::pair<int, int> scores;
    int score = 0, score2 = 0;
    std::string player1, player2;

    while (window.isOpen()) {
        sf::Event event{ };
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)  window.close();
        }

        switch (state) {

            case GameState::START_SCREEN:
                if (startScreen(window, font)) {
                    state = chooseGameMode(window, font);

                } else window.close();
                break;

                case GameState::CHOOSE_NAME: {
                if (window.getSize().x > SIZE_X || window.getSize().y > SIZE_Y) {
                    window.setSize(sf::Vector2u(SIZE_X, SIZE_Y));
                }
                player1 = chooseName(window, font);
                if (!player1.empty()) {
                    state = GameState::RUNNING;
                } else return 0;
                break;
            }

            case GameState::COOP_CHOOSE_NAME: {
                window.setSize(sf::Vector2u(COOP_SIZE_X, COOP_SIZE_Y));
                auto names = chooseNames(window, font);
                player1 = names.first; player2 = names.second;
                if (!player1.empty() && !player2.empty()) {
                    state = GameState::COOP_RUNNING;
                } else return 0;
                break;
            }

            case GameState::RUNNING:
            case GameState::COOP_RUNNING:
                scores = runGame(window, font, state, score, score2);
                state = GameState::GAME_OVER;
                break;

            case GameState::GAME_OVER:
                sAlienS.stop(); sAlienB.stop();
                gameOver(window, scores, font, state, player1, player2);
                state = chooseGameMode(window, font);
                break;
        }
    }
    return 0;
}
