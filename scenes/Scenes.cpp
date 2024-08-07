#include "Scenes.h"


bool startScreen(sf::RenderWindow &window, const sf::Font &font) {
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
    createdByText.setPosition(SIZE_X - createdByText.getLocalBounds().width - 20,
                              SIZE_Y - createdByText.getLocalBounds().height - 20);

    int level = 1; // Just for the start screen
    std::vector<std::unique_ptr<Asteroid>> asteroids = Asteroid::initAsteroids(21, level);

    Alien alien(AlienSize::SMALL);
    alien.shape.setPosition(SIZE_X / 2.f + SIZE_X / 4.f, SIZE_Y / 2.5f);

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

        for (auto &asteroid : asteroids) {
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

GameState chooseGameMode(sf::RenderWindow &window, const sf::Font &font) {
    sf::Text instruction("Select Game Mode:", font, 30);
    instruction.setFillColor(WHITE);
    instruction.setPosition(SIZE_X / 2.f - instruction.getLocalBounds().width / 2, SIZE_Y / 4.f);

    sf::Text singlePlayerText("1. Single Player", font, 24);
    singlePlayerText.setFillColor(WHITE);
    singlePlayerText.setPosition(SIZE_X / 2.f - singlePlayerText.getLocalBounds().width / 2, SIZE_Y / 2.f);

    sf::Text coopText("2. Coop Mode", font, 24);
    coopText.setFillColor(WHITE);
    coopText.setPosition(SIZE_X / 2.f - coopText.getLocalBounds().width / 2, SIZE_Y / 2.f + 50);

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return GameState::START_SCREEN;
            }
            if (event.type == sf::Event::KeyPressed) {
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
std::string nameHandler(sf::RenderWindow &window, const sf::Font &font,
                        const std::string& instructionText, const std::string& error = "")
{
    std::vector<HighScore> scores = readHighScores();
    std::sort(scores.begin(), scores.end(), [](const HighScore& a, const HighScore& b) {
        return a.score > b.score;
    });

    int selectIndex = -1; // start with nobody

    sf::Text instruction(instructionText, font, 20);
    instruction.setFillColor(WHITE);
    instruction.setPosition(100, 50);

    sf::Text errorText(error, font, 20);
    errorText.setFillColor(sf::Color::Red);
    errorText.setPosition(100, 200);

    sf::Text nameInput("", font, 20);
    nameInput.setFillColor(sf::Color(191, 64, 191));
    nameInput.setPosition(100, 100);

    sf::Text existingNames("", font, 20);
    existingNames.setFillColor(WHITE);
    existingNames.setPosition(100, 150);

    sf::RectangleShape highlight;
    int width = 0;
    highlight.setFillColor(sf::Color(128, 0, 128));
    highlight.setPosition(90, 150);

    for (const auto& score : scores) {
        sf::Text tempText(score.name + " - Highscore: " + std::to_string(score.score), font, 20);
        float textWidth = tempText.getLocalBounds().width;
        if (textWidth > width) {
            width = textWidth;
        }
    }
    highlight.setSize(sf::Vector2f(width + 50, 24));

    std::string playerNameInput;
    bool nameEntered = false;

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                return "";
            }
            if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode < 128 && event.text.unicode != 8 &&
                    event.text.unicode != 127) {
                    playerNameInput += static_cast<char>(event.text.unicode);
                    nameInput.setString(playerNameInput);
                    nameEntered = true;

                } else if (event.text.unicode == 8 &&
                           !playerNameInput.empty()) {
                    playerNameInput.pop_back();
                    nameInput.setString(playerNameInput);
                    nameEntered = !playerNameInput.empty();
                }
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Down) {
                    selectIndex = std::min(selectIndex + 1, (int) scores.size() - (nameEntered ? 0 : 1));

                } else if (event.key.code == sf::Keyboard::Up) {
                    selectIndex = std::max(selectIndex - 1, -1);

                } else if (event.key.code == sf::Keyboard::Delete) {
                    if (selectIndex >= 0 && selectIndex < scores.size()) {
                        removePlayer(scores, scores[selectIndex].name);
                        selectIndex = std::max(0, selectIndex - 1);
                        scores = readHighScores();
                    }

                } else if (event.key.code == sf::Keyboard::LAlt || event.key.code == sf::Keyboard::RAlt) {
                    if (!playerNameInput.empty() && nameEntered) {
                        auto it = std::find_if(scores.begin(), scores.end(), [&](const HighScore &s) {
                            return s.name == playerNameInput;
                        });
                        if (it == scores.end()) {
                            writeHighScore(playerNameInput, 0);
                            scores.push_back({playerNameInput, 0});
                            playerNameInput.clear();
                            nameInput.setString("");
                            nameEntered = false;
                            scores = readHighScores();
                        }
                    }
                } else if (event.key.code == sf::Keyboard::Enter) {
                    if (selectIndex >= 0 && selectIndex < scores.size()) {
                        return scores[selectIndex].name;
                    } else if (!playerNameInput.empty()) {
                        auto it = std::find_if(scores.begin(), scores.end(), [&](const HighScore &s) {
                            return s.name == playerNameInput;
                        });
                        if (it == scores.end()) {
                            writeHighScore(playerNameInput, 0);
                            return playerNameInput;
                        }
                    }
                }
            }

            window.clear();
            std::string existingNamesText;

            existingNamesText.clear();
            for (size_t i = 0; i < scores.size(); i++) {
                if (i == selectIndex) {
                    existingNamesText += "> ";
                }
                existingNamesText += scores[i].name + " - Highscore: " + std::to_string(scores[i].score) + "\n";
            }
            existingNames.setString(existingNamesText);

            window.draw(instruction);
            window.draw(nameInput);

            if (!error.empty()) {
                window.draw(errorText);
            }

            std::istringstream iss(existingNamesText);
            std::string line;
            size_t i = 0;
            while (std::getline(iss, line)) {
                sf::Text textLine(line, font, 20);
                textLine.setFillColor(WHITE);
                textLine.setPosition(100, 150 + i * 24);

                if (i == selectIndex) {
                    highlight.setPosition(90, 150 + i * 24);
                    window.draw(highlight);
                }
                window.draw(textLine);
                i++;
            }

            window.display();
        }
    }
    return "";
}


std::string chooseName(sf::RenderWindow &window, const sf::Font &font) {
    return nameHandler(window, font, "Enter name or select existing:");
}

std::pair<std::string, std::string> chooseNames(sf::RenderWindow &window, const sf::Font &font) {
    std::string name1 = nameHandler(window, font, "Enter name or select existing for Player 1:");
    if (name1.empty()) {
        return {"", ""};
    }
    std::string name2;
    for (;;) {
        name2 = nameHandler(window, font, "Enter name or select existing for Player 2:");
        if (name2.empty() || name2 != name1) {
            break;
        }
        nameHandler(window, font, "Enter name or select existing for Player 2:",
                    "Names must be different!");
    }
    return {name1, name2};
}


void removePlayer(std::vector<HighScore>& scores, const std::string& name) {
    scores.erase(std::remove_if(scores.begin(), scores.end(), [&](const HighScore& score) {
        return score.name == name;
    }), scores.end());

    std::ofstream file(".highscores.txt");
    for (const auto& score : scores) {
        file << score.name << "," << score.score << std::endl;
    }
}


// Formats an integer with thousand separators, e.g. 10000 -> 10.000
std::string format(int value) {
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << value;
    return ss.str();
}

void gameOver(sf::RenderWindow &window, const std::pair<int, int>& scores, const sf::Font& font,
              GameState& state, const std::string& name1, const std::string& name2)
{
    // Can't check for state so this is a temporary Solution
    bool coop = scores.second != -1;

    sGameOver.play();
    // "GAME OVER" Text
    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(60);
    gameOverText.setFillColor(WHITE);
    gameOverText.setPosition(SIZE_X / 2.f - gameOverText.getLocalBounds().width / 2, SIZE_Y / 3.f);

    // High Score for Player 1
    std::vector<HighScore> scoresList = readHighScores();
    auto it1 = std::find_if(scoresList.begin(), scoresList.end(), [&](const HighScore& s) {
        return s.name == name1;
    });
    int highscore1 = (it1 != scoresList.end()) ? it1->score : 0;

    if (scores.first > highscore1) {
        writeHighScore(name1, scores.first);
        highscore1 = scores.first;
    }

    // High Score for Player 2 if coop
    int highscore2 = 0;
    if (coop) {
        auto it2 = std::find_if(scoresList.begin(), scoresList.end(), [&](const HighScore& s) {
            return s.name == name2;
        });
        highscore2 = (it2 != scoresList.end()) ? it2->score : 0;

        if (scores.second > highscore2) {
            writeHighScore(name2, scores.second);
            highscore2 = scores.second;
        }
    }

    // High Score Text for Player 1
    sf::Text highscoreText1;
    highscoreText1.setFont(font);
    highscoreText1.setString((coop ? "Player 1 Highscore: " : "Highscore: ") + format(highscore1));
    highscoreText1.setCharacterSize(32);
    highscoreText1.setFillColor(WHITE);
    highscoreText1.setPosition(
            SIZE_X / 2.f - highscoreText1.getLocalBounds().width / 2, SIZE_Y / 2.f);

    // Score Text for Player 1
    sf::Text scoreText1;
    scoreText1.setFont(font);
    scoreText1.setString((coop ? "Player 1 Score: " : "Your score: ") + format(scores.first));
    scoreText1.setCharacterSize(28);
    scoreText1.setFillColor(WHITE);
    scoreText1.setPosition(
            SIZE_X / 2.f - scoreText1.getLocalBounds().width / 2,
            highscoreText1.getPosition().y + highscoreText1.getLocalBounds().height + 40);

    // High Score and Score Text for Player 2 if coop
    sf::Text highscoreText2;
    sf::Text scoreText2;
    if (coop) {
        highscoreText2.setFont(font);
        highscoreText2.setString("Player 2 Highscore: " + format(highscore2));
        highscoreText2.setCharacterSize(32);
        highscoreText2.setFillColor(WHITE);
        highscoreText2.setPosition(
                SIZE_X / 2.f - highscoreText2.getLocalBounds().width / 2,
                scoreText1.getPosition().y + scoreText1.getLocalBounds().height + 50 // increased spacing
        );

        scoreText2.setFont(font);
        scoreText2.setString("Player 2 Score: " + format(scores.second));
        scoreText2.setCharacterSize(28);
        scoreText2.setFillColor(WHITE);
        scoreText2.setPosition(
                SIZE_X / 2.f - scoreText2.getLocalBounds().width / 2,
                highscoreText2.getPosition().y + highscoreText2.getLocalBounds().height + 40);
    }

    // "Play Again" Button
    sf::Text playAgainText;
    playAgainText.setFont(font);
    playAgainText.setString("Play Again");
    playAgainText.setCharacterSize(30);
    playAgainText.setFillColor(WHITE);
    playAgainText.setPosition(
            SIZE_X / 2.f - playAgainText.getLocalBounds().width / 2, SIZE_Y / 1.5);

    // "(press enter)" Text
    sf::Text pressEnterText;
    pressEnterText.setFont(font);
    pressEnterText.setString("(press enter)");
    pressEnterText.setCharacterSize(18);
    pressEnterText.setFillColor(WHITE);
    pressEnterText.setPosition(
            playAgainText.getPosition().x +
            playAgainText.getLocalBounds().width / 2 -
            pressEnterText.getLocalBounds().width / 2,
            playAgainText.getPosition().y +
            playAgainText.getLocalBounds().height + 10);

    window.clear();
    window.draw(gameOverText);

    window.draw(highscoreText1);
    window.draw(scoreText1);

    if (coop) {
        window.draw(highscoreText2);
        window.draw(scoreText2);
    }

    if (!coop) window.draw(playAgainText);
    if (!coop) window.draw(pressEnterText);

    window.display();

    bool wait = true;
    while (wait) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed &&
                 event.key.code == sf::Keyboard::Escape))
            {
                window.close();
                wait = false;
            }
            // Restarts the game if the player presses enter
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Enter)
            {
                state = GameState::RUNNING;
                wait = false;
            }
        }
    }
}


/*
 * Secures a string by XOR-ing each char with a random byte, seeded by `key`
 */
std::string secure(const std::string& data, const unsigned long key) {
    std::string result = data;
    srand(key);

    for (size_t i = 0; i < data.size(); i++) {
        // random byte
        auto byte = static_cast<unsigned char>(rand() % 256);
        // XOR it with the char
        result[i] = static_cast<char>(data[i] ^ byte);
    }
    return result;
}

std::vector<HighScore>readHighScores() {
    Config config(".env");
    Config::createEnv(".env");
    auto key = config.getKey();

    std::vector<HighScore> scores;
    std::ifstream file(".highscores.txt");
    std::string line;

    while (std::getline(file, line)) {
        line = secure(line, key);
        std::istringstream iss(line);
        std::string name;
        int score;
        if (std::getline(iss, name, ',') && (iss >> score)) {
            scores.push_back({name, score});
        }
    }
    file.close();
    return scores;
}

void writeHighScore(const std::string& name, int score) {
    try {
        Config config(".env");
        Config::createEnv(".env");
        auto key = config.getKey();

        std::vector<HighScore> scores = readHighScores();
        auto it = std::find_if(scores.begin(), scores.end(), [&](const HighScore& s) {
            return s.name == name;
        });

        if (it != scores.end()) {
            if (it->score < score) it->score = score;

        } else {
            scores.push_back({name, score});
        }

        std::ofstream file(".highscores.txt");
        for (const auto& s : scores) {
            std::string line = s.name + "," + std::to_string(s.score);
            line = secure(line, key);
            file << line << std::endl;
        }
        file.close();

    } catch (const std::exception& e) {
        std::cerr << "Error while writing Highscore: " << e.what() << std::endl;
    }
}
