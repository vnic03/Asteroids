# Set the minimum required version of cmake for this project
cmake_minimum_required(VERSION 3.22.1)

project(Asteroids)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Asteroids src/main.cpp
    src/Asteroid.h
    src/GameConstants.h
    src/spaceship/Spaceship.h
    src/spaceship/Spaceship.cpp
    src/logic/logic.h
    src/logic/logic.cpp
    src/alien/Alien.cpp
    src/alien/Alien.h
    src/MovableEntity.h
    src/scenes/Scenes.h
    src/scenes/Scenes.cpp
    src/HighScores.cpp
    src/HighScores.h
    src/SimulationClock.h
)

target_sources(Asteroids PRIVATE
    src/logic/Movement.h
    src/logic/Movement.cpp
    src/logic/Collisions.h
    src/logic/Collisions.cpp
    src/logic/GameRules.h
    src/logic/WaveController.h
    src/logic/WaveController.cpp
    src/logic/GameRenderer.h
    src/logic/GameRenderer.cpp
)

# SFML
# used for graphics, window management, system events, and audio
find_package(SFML 2.6 COMPONENTS system window graphics audio REQUIRED)
target_link_libraries(Asteroids PRIVATE
  sfml-system
  sfml-window
  sfml-graphics
  sfml-audio
)

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
