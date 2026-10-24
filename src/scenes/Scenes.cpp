#include "Scenes.h"
#include <algorithm>

bool startScreen(sf::RenderWindow& window, const sf::Font& font) {
    // Title Text
    sf::Text title("Asteroids", font, 40);
    title.setFillColor(WHITE);
    title.setPosition(200, 200);

    // Start-Game Text
    sf::Text prompt("Press enter to start", font, 20);
    prompt.setFillColor(WHITE);
    prompt.setPosition(250, 300);

    // Credit Text
    sf::Text createdByText;
    createdByText.setFont(font);
    createdByText.setString("Created by @Nico");
    createdByText.setCharacterSize(20);
    createdByText.setFillColor(sf::Color(255, 255, 255, 128));
    createdByText.setPosition(
        SIZE_X - createdByText.getLocalBounds().width - 20,
        SIZE_Y - createdByText.getLocalBounds().height - 20);

    int level = 1; // Just for the start screen
    std::vector<std::unique_ptr<Asteroid>> asteroids =
        Asteroid::initAsteroids(21, level);

    Alien alien(AlienSize::SMALL);
    alien.shape.setPosition(SIZE_X / 2.f + SIZE_X / 4.f, SIZE_Y / 2.5f);
    alien.update(0.f, false);

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return false; // Fenster geschlossen
            }
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Enter) {
                return true; // Spiel starten
            }
        }

        window.clear();

        for (auto& asteroid : asteroids) {
            window.draw(asteroid->shape);
        }

        alien.draw(window);

        window.draw(title);
        window.draw(prompt);
        window.draw(createdByText);

        window.display();
    }
    return false;
}

GameState chooseGameMode(sf::RenderWindow& window, const sf::Font& font) {
    sf::Text instruction("Select Game Mode:", font, 30);
    instruction.setFillColor(WHITE);
    instruction.setPosition(
        SIZE_X / 2.f - instruction.getLocalBounds().width / 2, SIZE_Y / 4.f);

    sf::Text singlePlayerText("1. Single Player", font, 24);
    singlePlayerText.setFillColor(WHITE);
    singlePlayerText.setPosition(
        SIZE_X / 2.f - singlePlayerText.getLocalBounds().width / 2,
        SIZE_Y / 2.f);

    sf::Text coopText("2. Coop Mode", font, 24);
    coopText.setFillColor(WHITE);
    coopText.setPosition(SIZE_X / 2.f - coopText.getLocalBounds().width / 2,
                         SIZE_Y / 2.f + 50);

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return GameState::START_SCREEN;
            }
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::Num1) {
                    return GameState::CHOOSE_NAME;

                } else if (event.key.code == sf::Keyboard::Num2) {
                    return GameState::COOP_CHOOSE_NAME;
                }
            }
        }

        window.clear();
        window.draw(instruction);
        window.draw(singlePlayerText);
        window.draw(coopText);
        window.display();
    }
    return GameState::START_SCREEN;
}

// helper method for choosing name
std::string nameHandler(sf::RenderWindow& window, const sf::Font& font,
                        const std::string& instruction, ScoreMode mode,
                        const std::string& forbiddenName = "") {
    auto scores = readHighScores(mode);
    int selected = -1;
    std::string input;
    std::string error;

    auto refresh = [&]() {
        scores = readHighScores(mode);
        selected = -1;
    };

    while (window.isOpen()) {
        sf::Event event{};

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return "";
            }

            if (event.type == sf::Event::TextEntered) {
                const auto character = event.text.unicode;

                if (character == 8 && !input.empty()) {
                    input.pop_back();
                    selected = -1;
                } else if (character >= 32 && character <= 126 &&
                           input.size() < 32) {
                    input += static_cast<char>(character);
                    selected = -1;
                }
            }

            if (event.type != sf::Event::KeyPressed) {
                continue;
            }

            if (event.key.code == sf::Keyboard::Escape) {
                window.close();
                return "";
            }

            if (event.key.code == sf::Keyboard::Down) {
                if (!scores.empty()) {
                    selected = std::min(selected + 1,
                                        static_cast<int>(scores.size()) - 1);
                }
            } else if (event.key.code == sf::Keyboard::Up) {
                selected = std::max(-1, selected - 1);

            } else if (event.key.code == sf::Keyboard::Delete) {
                if (selected >= 0) {
                    if (scores[selected].name == forbiddenName) {
                        error = "Player 1 is already using this profile.";
                    } else {
                        removePlayer(scores[selected].name);
                        refresh();
                    }
                }

            } else if (event.key.code == sf::Keyboard::Enter) {
                const std::string name =
                    selected >= 0 ? scores[selected].name : input;

                if (!validPlayerName(name)) {
                    error = "Use 1-32 printable characters.";
                    continue;
                }

                if (name == forbiddenName) {
                    error = "Players must use different names.";
                    continue;
                }

                writeHighScore(name, 0, mode);
                return name;
            }
        }

        if (!window.isOpen()) {
            break;
        }

        window.clear(sf::Color::Black);

        sf::Text title(instruction, font, 24);
        title.setPosition(80.f, 40.f);
        window.draw(title);

        sf::Text inputText("Name: " + input, font, 22);
        inputText.setPosition(80.f, 90.f);
        window.draw(inputText);

        sf::Text help(
            "Type name or use arrows | Enter: play | Del: delete profile", font,
            16);
        help.setPosition(80.f, 130.f);
        window.draw(help);

        constexpr int visibleRows = 17;
        const int firstVisible = std::max(0, selected - visibleRows + 1);

        for (int row = 0; row < visibleRows; ++row) {
            const int index = firstVisible + row;

            if (index >= static_cast<int>(scores.size())) {
                break;
            }

            const auto& entry = scores[index];

            std::string label = (index == selected ? "> " : "  ") + entry.name +
                                " - " + std::to_string(entry.score);

            if (entry.legacyScore > 0) {
                label += " | Legacy: " + std::to_string(entry.legacyScore);
            }

            sf::Text text(label, font, 20);
            text.setPosition(80.f, 180.f + row * 26.f);
            text.setFillColor(index == selected ? sf::Color::Cyan : WHITE);

            window.draw(text);
        }

        sf::Text errorText(error, font, 18);
        errorText.setFillColor(sf::Color::Red);
        errorText.setPosition(80.f, 650.f);
        window.draw(errorText);

        sf::Text status(highScoreStatus(), font, 16);
        status.setFillColor(sf::Color(255, 200, 80));
        status.setPosition(80.f, 690.f);
        window.draw(status);

        window.display();
    }

    return "";
}

std::string chooseName(sf::RenderWindow& window, const sf::Font& font) {
    return nameHandler(window, font, "Choose your name", ScoreMode::Solo);
}

std::pair<std::string, std::string> chooseNames(sf::RenderWindow& window,
                                                const sf::Font& font) {
    const auto first =
        nameHandler(window, font, "Choose Player 1", ScoreMode::Coop);

    if (first.empty()) {
        return {"", ""};
    }

    const auto second =
        nameHandler(window, font, "Choose Player 2", ScoreMode::Coop, first);

    return {first, second};
}

void gameOver(sf::RenderWindow& window, const std::pair<int, int>& scores,
              const sf::Font& font, GameState& state, const std::string& name1,
              const std::string& name2) {
    const bool coop = scores.second != -1;
    const auto mode = coop ? ScoreMode::Coop : ScoreMode::Solo;

    writeHighScore(name1, scores.first, mode);

    if (coop) {
        writeHighScore(name2, scores.second, mode);
    }

    const auto records = readHighScores(mode);

    auto best = [&](const std::string& name) {
        const auto found = std::find_if(
            records.begin(), records.end(),
            [&](const HighScore& entry) { return entry.name == name; });

        return found != records.end() ? found->score : 0;
    };

    std::string results = name1 + "\nScore: " + std::to_string(scores.first) +
                          "\nBest: " + std::to_string(best(name1));

    if (coop) {
        results += "\n\n" + name2 +
                   "\nScore: " + std::to_string(scores.second) +
                   "\nBest: " + std::to_string(best(name2));
    }

    playSound(sGameOver);

    while (window.isOpen()) {
        sf::Event event{};

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed &&
                 event.key.code == sf::Keyboard::Escape)) {
                window.close();
                return;
            }

            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Enter) {
                state = GameState::START_SCREEN;
                return;
            }
        }

        if (!window.isOpen()) {
            break;
        }

        window.clear(sf::Color::Black);

        sf::Text title("GAME OVER", font, 60);
        title.setPosition(100.f, 80.f);
        window.draw(title);

        sf::Text resultText(results, font, 26);
        resultText.setPosition(100.f, 210.f);
        window.draw(resultText);

        sf::Text prompt("Enter: play again | Esc: quit", font, 22);
        prompt.setPosition(100.f, 620.f);
        window.draw(prompt);

        sf::Text status(highScoreStatus(), font, 16);
        status.setFillColor(sf::Color(255, 200, 80));
        status.setPosition(100.f, 690.f);
        window.draw(status);

        window.display();
    }
}
