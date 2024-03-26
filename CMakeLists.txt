# Set the minimum required version of cmake for this project
cmake_minimum_required(VERSION 3.27)

project(Asteroids)
set(CMAKE_CXX_STANDARD 20)

add_executable(Asteroids main.cpp
        Asteroid.h                      # Header for asteroid entities
        GameConstants.h                 # Header for game-wide constants
        spaceship/Spaceship.h           # Header for the spaceship entity
        spaceship/Spaceship.cpp         # Implementation for the spaceship entity
        logic/logic.h                   # Header for game logic functions
        logic/logic.cpp                 # Implementation for game logic functions
        alien/Alien.cpp                 # Implementation for alien entities
        alien/Alien.h                   # Header for alien entities
        MovableEntity.h                 # Base abstract class for spaceship and alien
        )

# SFML
# used for graphics, window management, system events, and audio
find_package(SFML COMPONENTS graphics window system audio REQUIRED)
target_link_libraries(Asteroids sfml-graphics sfml-window sfml-system sfml-audio)

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})