#include "Scenes.h"

bool startScreen(sf::RenderWindow &window, const sf::Font &font) {
    // Title Text
    sf::Text title("Asteroids", font, 40);
    title.setFillColor(sf::Color::White);
    title.setPosition(200, 200);

    // Start-Game Text
    sf::Text prompt("Press enter to start", font, 20);
    prompt.setFillColor(sf::Color::White);
    prompt.setPosition(250, 300);

    // Credit Text
    sf::Text createdByText;
    createdByText.setFont(font);
    createdByText.setString("Created by Nico");
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


std::string chooseName(sf::RenderWindow &window, const sf::Font &font) {
    std::vector<Score> scores = readHighScores();
    std::sort(scores.begin(), scores.end(), [](const Score& a, const Score& b) {
        return a.score > b.score;
    });

    int selectIndex = -1; // start with nobody

    sf::Text instruction("Enter name or select existing:", font, 20);
    instruction.setFillColor(sf::Color::White);
    instruction.setPosition(100, 50);

    sf::Text nameInput("", font, 20);
    nameInput.setFillColor(sf::Color(191, 64, 191));
    nameInput.setPosition(100, 100);

    sf::Text existingNames("", font, 20);
    existingNames.setFillColor(sf::Color::White);
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
                    selectIndex =
                            std::min(selectIndex + 1, (int) scores.size() -
                                                      (nameEntered ? 0 : 1));

                } else if (event.key.code == sf::Keyboard::Up) {
                    selectIndex = std::max(selectIndex - 1, -1);

                } else if (event.key.code == sf::Keyboard::Delete) {
                    removePlayer(scores, scores[selectIndex].name);
                    selectIndex = std::max(0, selectIndex - 1);
                    scores = readHighScores();

                    // saves the name
                } else if (event.key.code == sf::Keyboard::LAlt ||
                            event.key.code == sf::Keyboard::RAlt) {
                    if (!playerNameInput.empty() && nameEntered) {
                        auto it = std::find_if(scores.begin(), scores.end(),
                                               [&](const Score &s) {
                                                   return s.name ==
                                                          playerNameInput;
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
                existingNamesText +=
                        scores[i].name + " - Highscore: " +
                        std::to_string(scores[i].score) + "\n";
            }
            existingNames.setString(existingNamesText);

            window.draw(instruction);
            window.draw(nameInput);

            std::istringstream iss(existingNamesText);
            std::string line;
            size_t i = 0;
            while (std::getline(iss, line)) {
                sf::Text textLine(line, font, 20);
                textLine.setFillColor(sf::Color::White);
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

void removePlayer(std::vector<Score>& scores, const std::string& name) {
    scores.erase(std::remove_if(scores.begin(), scores.end(), [&](const Score& score) {
        return score.name == name;
    }), scores.end());

    std::ofstream file("highscores.txt");
    for (const auto& score : scores) {
        file << score.name << "," << score.score << std::endl;
    }
}


void gameOver(sf::RenderWindow &window, const int score, const sf::Font& font,
              GameState& state, const std::string& name) {
    sGameOver.play();
    // "GAME OVER" Text
    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setString("GAME OVER");
    gameOverText.setCharacterSize(60);
    gameOverText.setFillColor(sf::Color::White);
    gameOverText.setPosition(
            SIZE_X / 2.f - gameOverText.getLocalBounds().width / 2, SIZE_Y / 3.f);

    // High Score
    std::vector<Score> scores = readHighScores();
    auto it = std::find_if(scores.begin(), scores.end(), [&](const Score& s) {
        return s.name == name;
    });
    int highscore = (it != scores.end()) ? it->score : 0;

    if(score > highscore) {
        writeHighScore(name, score);
        highscore = score;
    }

    // High Score Text
    sf::Text highscoreText;
    highscoreText.setFont(font);
    highscoreText.setString("Highscore: " + std::to_string(highscore));
    highscoreText.setCharacterSize(32);
    highscoreText.setFillColor(sf::Color::White);
    highscoreText.setPosition(
            SIZE_X / 2.f - highscoreText.getLocalBounds().width / 2, SIZE_Y / 2.f);

    // Score Text
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setString("your score: " + std::to_string(score));
    scoreText.setCharacterSize(28);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(
            SIZE_X / 2.f - scoreText.getLocalBounds().width / 2,
            highscoreText.getPosition().y + highscoreText.getLocalBounds().height + 40);


    // "Play Again" Button
    sf::Text playAgainText;
    playAgainText.setFont(font);
    playAgainText.setString("Play Again");
    playAgainText.setCharacterSize(30);
    playAgainText.setFillColor(sf::Color::White);
    playAgainText.setPosition(
            SIZE_X / 2.f - playAgainText.getLocalBounds().width / 2, SIZE_Y / 1.5);

    // "(press enter)" Text
    sf::Text pressEnterText;
    pressEnterText.setFont(font);
    pressEnterText.setString("(press enter)");
    pressEnterText.setCharacterSize(18);
    pressEnterText.setFillColor(sf::Color::White);
    pressEnterText.setPosition(
            playAgainText.getPosition().x +
            playAgainText.getLocalBounds().width / 2 -
            pressEnterText.getLocalBounds().width / 2,

            playAgainText.getPosition().y +
            playAgainText.getLocalBounds().height + 10);

    window.clear();
    window.draw(gameOverText);
    window.draw(highscoreText);
    window.draw(scoreText);
    window.draw(playAgainText);
    window.draw(pressEnterText);
    window.display();

    bool wait = true;
    while (wait) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed ||
                (event.type == sf::Event::KeyPressed &&
                 event.key.code == sf::Keyboard::Escape)) {
                window.close();
                wait = false;
            }
            // Restarts the game if the player presses enter
            if (event.type == sf::Event::KeyPressed &&
                event.key.code == sf::Keyboard::Enter) {
                state = GameState::RUNNING;
                wait = false;
            }
        }
    }
}

std::vector<Score>readHighScores() {
    std::vector<Score> scores;
    std::ifstream file("highscores.txt");
    std::string line;

    while (std::getline(file, line)) {
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
    std::vector<Score> scores = readHighScores();
    auto it = std::find_if(scores.begin(), scores.end(), [&](const Score& s) {
        return s.name == name;
    });
    if (it != scores.end()) {
        if (it->score < score) it->score = score;
    } else {
        scores.push_back({name, score});
    }

    std::ofstream file("highscores.txt");
    for (const auto& s : scores) {
        file << s.name << "," << s.score << std::endl;
    }
    file.close();
}