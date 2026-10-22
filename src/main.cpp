#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#include "GameConstants.h"
#include "HighScores.h"
#include "logic/logic.h"
#include "scenes/Scenes.h"

namespace fs = std::filesystem;

sf::SoundBuffer bBeat1, bBeat2, bSmallEx, bMediumEx, bBigEx;
sf::SoundBuffer bFire, bThrust, bEx, bAlienB, bAlienS, bGameOver;

sf::Sound sBeat1, sBeat2, sSmallEx, sMediumEx, sBigEx;
sf::Sound sFire, sThrust, sEx, sAlienB, sAlienS, sGameOver;

SimulationClock alienBTimer;
SimulationClock alienSTimer;

bool soundEnabled = true;

namespace {

fs::path assetRoot;

fs::path findAssetRoot(const char* executableArgument) {
    std::error_code error;

    auto executable = fs::read_symlink("/proc/self/exe", error);

    if (error) {
        executable = fs::absolute(executableArgument);
    }

    const auto executableDirectory = executable.parent_path();

    std::vector<fs::path> candidates;

    if (const char* custom = std::getenv("ASTEROIDS_ASSET_DIR");
        custom && *custom) {
        candidates.emplace_back(custom);
    }

    candidates.push_back(executableDirectory / "assets");

    candidates.push_back(
        executableDirectory / ".." /
        "share" / "Asteroids" / "assets"
    );

    candidates.push_back(fs::current_path() / "assets");

    for (const auto& candidate : candidates) {
        error.clear();

        if (fs::is_directory(candidate, error)) {
            return candidate;
        }
    }

    return executableDirectory / "assets";
}

bool loadFont(sf::Font& font) {
    std::vector<fs::path> candidates;

    if (const char* custom = std::getenv("ASTEROIDS_FONT");
        custom && *custom) {
        candidates.emplace_back(custom);
    }

    candidates.push_back(assetRoot / "font" / "arcade-font.otf");

    candidates.emplace_back(
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    );

    candidates.emplace_back(
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf"
    );

    for (const auto& candidate : candidates) {
        if (font.loadFromFile(candidate.string())) {
            return true;
        }
    }

    std::cerr
        << "No usable font found. Restore assets/font/arcade-font.otf\n"
        << "or set ASTEROIDS_FONT to a valid font file.\n";

    return false;
}

}

void playSound(sf::Sound& sound) {
    if (soundEnabled && sound.getBuffer() != nullptr) {
        sound.play();
    }
}

void stopGameSounds() {
    sf::Sound* sounds[] = {
        &sBeat1, &sBeat2,
        &sSmallEx, &sMediumEx, &sBigEx,
        &sFire, &sThrust, &sEx,
        &sAlienB, &sAlienS, &sGameOver
    };

    for (auto* sound : sounds) {
        sound->stop();
    }
}

bool loadSound(
    sf::SoundBuffer& buffer,
    sf::Sound& sound,
    const std::string& filename
) {
    const auto path = assetRoot / "sound" / filename;

    if (!buffer.loadFromFile(path.string())) {
        std::cerr << "Optional sound unavailable: " << path << '\n';
        return false;
    }

    sound.setBuffer(buffer);
    return true;
}

void playBeat(sf::Sound& beat1, sf::Sound& beat2, bool enabled) {
    static bool first = true;

    if (!enabled) {
        return;
    }

    auto& sound = first ? beat1 : beat2;

    if (sound.getStatus() != sf::Sound::Playing) {
        playSound(sound);
    }

    first = !first;
}

int main(int argc, char** argv) {
    try {
        assetRoot = findAssetRoot(argc > 0 ? argv[0] : "Asteroids");

        sf::Font font;

        if (!loadFont(font)) {
            return 1;
        }

        // Independent calls: one missing file must not skip other sounds.
        loadSound(bSmallEx, sSmallEx, "bang-small.wav");
        loadSound(bMediumEx, sMediumEx, "bang-medium.wav");
        loadSound(bBigEx, sBigEx, "bang-large.wav");
        loadSound(bFire, sFire, "fire.wav");
        loadSound(bThrust, sThrust, "thrust.wav");
        loadSound(bBeat1, sBeat1, "beat1.wav");
        loadSound(bBeat2, sBeat2, "beat2.wav");
        loadSound(bEx, sEx, "spaceship-explosion.wav");
        loadSound(bAlienB, sAlienB, "alien-big.wav");
        loadSound(bAlienS, sAlienS, "alien-small.wav");
        loadSound(bGameOver, sGameOver, "game-over.wav");

        sFire.setVolume(60.f);
        sThrust.setLoop(true);

        // Load or migrate before gameplay seeds its random generator.
        readHighScores();

        sf::RenderWindow window(
            sf::VideoMode(SIZE_X, SIZE_Y),
            "Asteroids"
        );

        window.setFramerateLimit(60);
        window.setKeyRepeatEnabled(false);

        GameState state = GameState::START_SCREEN;

        std::pair<int, int> scores{0, -1};
        int score = 0;
        int score2 = 0;

        std::string player1;
        std::string player2;

        while (window.isOpen()) {
            switch (state) {
                case GameState::START_SCREEN:
                    if (startScreen(window, font)) {
                        state = chooseGameMode(window, font);
                    } else {
                        window.close();
                    }
                    break;

                case GameState::CHOOSE_NAME:
                    window.setSize(sf::Vector2u(SIZE_X, SIZE_Y));
                    player1 = chooseName(window, font);

                    if (player1.empty()) {
                        window.close();
                        break;
                    }

                    player2.clear();
                    score = 0;
                    score2 = 0;
                    state = GameState::RUNNING;
                    break;

                case GameState::COOP_CHOOSE_NAME: {
                    window.setSize(
                        sf::Vector2u(SIZE_X + 250, SIZE_Y + 100)
                    );

                    const auto names = chooseNames(window, font);
                    player1 = names.first;
                    player2 = names.second;

                    if (player1.empty() || player2.empty()) {
                        window.close();
                        break;
                    }

                    score = 0;
                    score2 = 0;
                    state = GameState::COOP_RUNNING;
                    break;
                }

                case GameState::RUNNING:
                case GameState::COOP_RUNNING: {
                    const bool coop =
                        state == GameState::COOP_RUNNING;

                    scores = runGame(
                        window, font, state, score, score2
                    );

                    if (!window.isOpen()) {
                        const auto mode =
                            coop ? ScoreMode::Coop : ScoreMode::Solo;

                        writeHighScore(player1, scores.first, mode);

                        if (coop) {
                            writeHighScore(player2, scores.second, mode);
                        }
                    }

                    state = GameState::GAME_OVER;
                    break;
                }

                case GameState::GAME_OVER:
                    gameOver(
                        window, scores, font, state, player1, player2
                    );

                    if (window.isOpen()) {
                        state = chooseGameMode(window, font);
                    }
                    break;
            }
        }

        stopGameSounds();
        return 0;

    } catch (const std::exception& exception) {
        std::cerr << "Asteroids: " << exception.what() << '\n';
        return 1;
    }
}
