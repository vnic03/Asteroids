# Set the minimum required version of cmake for this project
cmake_minimum_required(VERSION 3.22.1)

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
        scenes/Scenes.h                 # Manages the logic and design of the scenes
        scenes/Scenes.cpp               # Implementation for logic and design
        Config.cpp                      # Handles the .env file
        )

# SFML
# used for graphics, window management, system events, and audio
find_package(SFML COMPONENTS graphics window system audio REQUIRED)
target_link_libraries(Asteroids sfml-graphics sfml-window sfml-system sfml-audio)

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Installation Directory" FORCE)
endif()

# Install the executable
install(TARGETS Asteroids DESTINATION bin)

# Install assets directory
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION share/Asteroids)

include(InstallRequiredSystemLibraries)

set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_NAME "Asteroids")
set(CPACK_PACKAGE_RELEASE 1)

set(CPACK_PACKAGE_CONTACT "Nico")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Asteroids - My own version of the classic arcade game Asteroids")

set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsfml-graphics, libsfml-window, libsfml-system, libsfml-audio")
set(CPACK_RPM_PACKAGE_REQUIRES "sfml-graphics, sfml-window, sfml-system, sfml-audio")

include(CPack)