cmake_minimum_required(VERSION 3.22.1)

project(Asteroids VERSION 1.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Asteroids
    src/main.cpp
    src/Asteroid.h
    src/GameConstants.h
    src/MovableEntity.h
    src/SimulationClock.h

    src/spaceship/Spaceship.h
    src/spaceship/Spaceship.cpp

    src/alien/Alien.h
    src/alien/Alien.cpp

    src/scenes/Scenes.h
    src/scenes/Scenes.cpp

    src/HighScores.h
    src/HighScores.cpp

    src/logic/logic.h
    src/logic/logic.cpp
    src/logic/Movement.h
    src/logic/Movement.cpp
    src/logic/Collisions.h
    src/logic/Collisions.cpp
    src/logic/GameRules.h
    src/logic/WaveController.h
    src/logic/WaveController.cpp
    src/logic/GameRenderer.h
    src/logic/GameRenderer.cpp
    src/logic/Progression.h

    src/music/MusicDirector.h
    src/music/MusicDirector.cpp
    src/music/AudioSettings.h
)

find_package(
    SFML 2.6
    COMPONENTS system window graphics audio
    REQUIRED
)

target_link_libraries(Asteroids PRIVATE
    sfml-system
    sfml-window
    sfml-graphics
    sfml-audio
)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(
        CMAKE_INSTALL_PREFIX
        "${CMAKE_BINARY_DIR}/install"
        CACHE PATH "Installation Directory"
        FORCE
    )
endif()

if(APPLE)
    set_target_properties(Asteroids PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Asteroids"
        MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.vnic03.asteroids"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        INSTALL_RPATH "@executable_path/../Frameworks"
    )

    add_custom_command(TARGET Asteroids POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_BUNDLE_DIR:Asteroids>/Contents/Resources/assets"
        VERBATIM
    )

    install(TARGETS Asteroids
        BUNDLE DESTINATION .
    )

    install(FILES "${CMAKE_SOURCE_DIR}/LICENSE"
        DESTINATION "Asteroids.app/Contents/Resources"
    )

    set(
        MACOS_DEPENDENCY_DIRS ""
        CACHE STRING "Additional macOS library search directories"
    )

    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/FixMacBundle.cmake.in"
        "${CMAKE_BINARY_DIR}/FixMacBundle.cmake"
        @ONLY
    )

    install(SCRIPT "${CMAKE_BINARY_DIR}/FixMacBundle.cmake")
else()
    file(
        COPY "${CMAKE_SOURCE_DIR}/assets"
        DESTINATION "${CMAKE_BINARY_DIR}"
    )

    install(TARGETS Asteroids DESTINATION bin)

    install(
        DIRECTORY "${CMAKE_SOURCE_DIR}/assets"
        DESTINATION share/Asteroids
    )
endif()

include(InstallRequiredSystemLibraries)

set(CPACK_RESOURCE_FILE_LICENSE
    "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE"
)

set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_NAME "Asteroids")
set(CPACK_PACKAGE_CONTACT "Nico")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Asteroids - An arcade game built with C++ and SFML"
)

# Release archives; native DEB/RPM packages need separate configuration.
if(APPLE)
    set(CPACK_GENERATOR "ZIP")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

include(CPack)
