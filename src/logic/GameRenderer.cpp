#include "GameRenderer.h"

#include <algorithm>
#include <string>
#include <cmath>

namespace {

void drawNumber(
    sf::RenderWindow& window,
    int number,
    sf::Vector2f position,
    int id
) {
    const auto& points = NUMBER_LINES[number];
    sf::VertexArray vertices(sf::Lines, points.size());

    const sf::Color color =
        id == 1 ? PLAYER_1_COLOR :
        id == 2 ? PLAYER_2_COLOR :
                  WHITE;

    for (std::size_t i = 0; i < points.size(); ++i) {
        vertices[i].position = {
            position.x + points[i].x * NUMBER_SCALE,
            position.y + points[i].y * NUMBER_SCALE
        };

        vertices[i].color = color;
    }

    window.draw(vertices);
}

} // namespace

GameRenderer::GameRenderer(const sf::Font& font)
    : pauseText("Pause", font, 40), hudText("", font, 18)
{
    pauseText.setFillColor(WHITE);
    pauseText.setPosition(
        SIZE_X / 2.f - pauseText.getLocalBounds().width / 2.f,
        SIZE_Y / 2.f
    );

    lifeShape.setPointCount(5);
    lifeShape.setPoint(0, {-10.f, -12.f});
    lifeShape.setPoint(1, {0.f, 10.f});
    lifeShape.setPoint(2, {10.f, -12.f});
    lifeShape.setPoint(3, {8.f, -10.f});
    lifeShape.setPoint(4, {-8.f, -10.f});

    lifeShape.setFillColor(sf::Color::Transparent);
    lifeShape.setOutlineThickness(1.f);
    lifeShape.setRotation(180.f);
}

void GameRenderer::drawScore(
    sf::RenderWindow& window,
    int id,
    int score
) {
    std::string text = std::to_string(std::max(0, score));

    if (text.size() < 5) {
        text.insert(0, 5 - text.size(), '0');
    }

    const float spacing = NUMBER_SCALE * 1.2f;

    sf::Vector2f position(
        SIZE_X - 20.f - spacing * static_cast<float>(text.size()),
        id == 2 ? 60.f : 20.f
    );

    for (char digit : text) {
        drawNumber(window, digit - '0', position, id);
        position.x += spacing;
    }
}

void GameRenderer::drawLives(
    sf::RenderWindow& window,
    const SpaceShip& spaceship,
    float y
) {
    lifeShape.setOutlineColor(spaceship.handleColor());

    for (int i = 0; i < spaceship.lives; ++i) {
        lifeShape.setPosition(25.f + i * 40.f, y);
        window.draw(lifeShape);
    }
}

void GameRenderer::draw(
    sf::RenderWindow& window,
    const std::vector<std::unique_ptr<Asteroid>>& asteroids,
    const std::vector<std::unique_ptr<Alien>>& aliens,
    SpaceShip* spaceships,
    bool coop,
    int score,
    int score2,
    bool paused,
    const WaveController& waves,
    const Progression&
) {
    window.clear(sf::Color::Black);

    for (const auto& asteroid : asteroids) {
        window.draw(asteroid->shape);
    }

    for (const auto& alien : aliens) {
        alien->draw(window);
    }

    const int firstPlayer = coop ? 1 : 0;
    const int endPlayer = coop ? 3 : 1;

    for (int i = firstPlayer; i < endPlayer; ++i) {
        spaceships[i].draw(window);

        drawScore(
            window,
            spaceships[i].id,
            i == 2 ? score2 : score
        );
    }

    if (coop) {
        drawLives(window, spaceships[1], 28.f);
        drawLives(window, spaceships[2], 68.f);
    } else {
        drawLives(window, spaceships[0], 28.f);
    }

    auto drawCenteredText = [&](
        const std::string& text,
        float y
    ) {
        hudText.setString(text);
        hudText.setFillColor(WHITE);

        const auto bounds = hudText.getLocalBounds();

        hudText.setPosition(
            SIZE_X / 2.f - bounds.left - bounds.width / 2.f,
            y
        );

        window.draw(hudText);
    };

    drawCenteredText(
        "WAVE " + std::to_string(waves.currentLevel()),
        12.f
    );

    const float remaining = waves.secondsRemaining();

    if (remaining > 0.f && remaining <= 5.f) {
        const int seconds = static_cast<int>(
            std::ceil(remaining)
        );

        drawCenteredText(
            "NEXT WAVE IN " + std::to_string(seconds) + "s",
            40.f
        );
    }

    if (paused) {
        window.draw(pauseText);
    }

    window.display();
}
