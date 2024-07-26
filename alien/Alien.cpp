#include "Alien.h"


Alien::Alien(AlienSize size) : alienSize(size) {
    float startX; float startY;

    if (size == AlienSize::BIG) {
        shootInterval = 3.f; // 3 sec
        lives = 6;

        // Randomly choose a side to spawn the alien
        int side = std::rand() % 4;
        switch (side) {
            case 0: // up
                startX = std::rand() % SIZE_X;
                startY = 0;
                break;
            case 1: // right
                startX = SIZE_X;
                startY = std::rand() % SIZE_Y;
                break;
            case 2: // down
                startX = std::rand() % SIZE_X;
                startY = SIZE_Y;
                break;
            case 3: // left
                startX = 0;
                startY = std::rand() % SIZE_Y;
                break;
        }
        shape.setPosition(startX, startY);

        sf::Vector2f center(SIZE_X / 2.f, SIZE_Y / 2.f);
        sf::Vector2f startPos(startX, startY);
        sf::Vector2f direction = center - startPos;

        float length = std::sqrt(direction.x * direction.x +
                direction.y * direction.y);
        direction /= length;
        velocity = direction * 50.f;

    } else { // SMALL
        velocity = sf::Vector2f(100.f, 0.f);
        shootInterval = 1.5f;
        lives = 2;
    }
    initShape();
}

void Alien::draw(sf::RenderWindow &window) {
    line.setPosition(shape.getPosition());
    float cockpitOffsetY = (-r_y) - (cockpitHeight / 2) + 1.9f;
    cockpit.setPosition(shape.getPosition().x, shape.getPosition().y + cockpitOffsetY);

    window.draw(shape);
    window.draw(line);
    window.draw(cockpit);

    // Draw projectiles
    for (const auto& p : projectiles) {
        sf::CircleShape circle(p.radius);
        circle.setPosition(p.position - sf::Vector2f(p.radius, p.radius));
        circle.setFillColor(p.color);
        window.draw(circle);
    }
}

void Alien::update(float delta, bool sound) {
    sf::Vector2f pos = shape.getPosition();
    pos += velocity * delta; // Update position based on velocity and time

    // Makes SMALL aliens move in random directions
    if (alienSize == AlienSize::SMALL) {
        static float changeDInterval = 0.5f; // 5 sec
        static float lastChange = 0.f; // Time since last change

        lastChange += delta;

        // Change direction every 5 seconds
        if (lastChange >= changeDInterval) {
            auto angle = static_cast<float>((std::rand() % 360) *
                                            (M_PI / 180.0));
            velocity = sf::Vector2f(std::cos(angle) * 100.f,
                                    std::sin(angle) * 100.f);
            lastChange = 0.f;
        }
    }

    for (auto& p : projectiles) {
        p.position += p.direction * p.speed * delta;
    }

    wrapAroundScreen(shape, pos);

    line.setPosition(pos);

    // Positions the cockpit at the Top-Center of the alien
    float cockpitOffsetY = (-r_y) - (cockpitHeight / 2) + 1.9f;
    cockpit.setPosition(pos.x, pos.y + cockpitOffsetY);
}

void Alien::shoot(bool sound, std::optional<sf::Vector2f> playerPos) {
    if (shootTimer.getElapsedTime().asSeconds() > shootInterval) {
        sf::Vector2f startPos = shape.getPosition();

        // BIG shoots larger, less often; SMALL shoots smaller, more often
        int shootCount = alienSize == AlienSize::BIG ? 3 : 4;
        float radius = alienSize == AlienSize::BIG ? 8.f : 5.f;

        // Randomize the direction of the projectiles
        for (int i = 0; i < shootCount; i++) {
            sf::Vector2f direction;

            if (alienSize == AlienSize::BIG) {
                auto randomAngle = static_cast<float>(
                        (std::rand() % 360) * (M_PI / 180.0)
                );
                direction = sf::Vector2f(std::cos(randomAngle), std::sin(randomAngle));
            } else { // SMALL
                if (playerPos) {
                    direction = *playerPos - startPos;
                    float length = std::sqrt(direction.x * direction.x +
                                             direction.y * direction.y);
                    direction /= length;
                }
            }
            // projectile velocity
            float pVel = alienSize == AlienSize::BIG ? 80.f : 120.f;
            projectiles.emplace_back(startPos, direction, WHITE, pVel, radius);
        }
        shootTimer.restart();
        if (sound) sFire.play();
    }
}

void Alien::explode(bool sound) {
    explosion = true;
    if (sound) sEx.play();
}

// Creates the shape of the alien, hexagon with a trapezoid cockpit
void Alien::initShape() {
    const int NUM_POINTS = 6;
    r = alienSize == AlienSize::BIG ? 24.f : 12.f;
    r_y = r * 0.5f;
    shape.setPointCount(NUM_POINTS);
    float angle = 2 * M_PI / NUM_POINTS;

    for (int i = 0; i < NUM_POINTS; ++i) {
        float x = cosf(angle * i) * r;
        float y = sinf(angle * i) * r_y;
        shape.setPoint(i, sf::Vector2f(x, y));
    }

    float lineLength = 2 * r;
    float thickness = 3.f;
    line.setSize(sf::Vector2f(lineLength, thickness));
    line.setOrigin(lineLength / 2, thickness / 2);

    shape.setOutlineColor(WHITE);
    shape.setFillColor(sf::Color::Transparent);
    shape.setOutlineThickness(1.5f);

    cockpit.setPointCount(4);

    float cockpitWidthTop = r * 0.6f;
    float cockpitWidthBottom = r * 0.8f;
    cockpitHeight = r_y * 0.6f;

    cockpit.setPoint(0, sf::Vector2f(-cockpitWidthTop / 2, -cockpitHeight));
    cockpit.setPoint(1, sf::Vector2f(cockpitWidthTop / 2, -cockpitHeight));
    cockpit.setPoint(2, sf::Vector2f(cockpitWidthBottom / 2, 0));
    cockpit.setPoint(3, sf::Vector2f(-cockpitWidthBottom / 2, 0));

    cockpit.setFillColor(sf::Color::Transparent);
    cockpit.setOutlineColor(WHITE);
    cockpit.setOutlineThickness(1.5f);
}
