# Asteroids Clone in C++

This project is my own version of the classic arcade game Asteroids, 
implemented in C++ using the SFML library. Experience the nostalgia 
of dodging and shooting asteroids in space and avoiding alien UFOs!

![Start Screen](assets/gameplay/start-screen.png)

### Choose Your Name
Before starting the game, you can now enter a name or select an 
existing one from the list.

![Choose Name](assets/gameplay/choose-name.png)
*Choose your name and make it to the top of the leaderboard!*

### New Feature: Co-op Mode

Now you can play together with a friend in the same round! 

![COOP](assets/gameplay/coop.png)
*COOP*


### Highscores
Compete against yourself or friends by beating highscores! Each name has 
its own highscore, making it fun to try and top the leaderboard.
Highscores and Player-Names  are saved, even after closing the game!

## Gameplay 

Experience intense asteroid dodging and shooting action.
Navigate through fields of asteroids and fight off alien 
invaders to achieve high scores!

[![Gameplay Screenshot](assets/gameplay/gameplay.png)](https://www.youtube.com/watch?v=26BmzsI9pHM)
*The Picture does not do the game justice, klick on it to see GamePlay!*

## Game Over 

Don't let the asteroids hit you! If you get hit, it's game over!

![Game Over Screenshot](assets/gameplay/game-over.png)
*Don't worry you can always try again!*

## Controls

The game is playable with both the keyboard and a
Controller, offering a flexible arcade experience.

### Keyboard Controls:

- **Arrow Keys or A W D:** Move and Rotate the spaceship
- **Space:** Shoot
- **Enter:** Start the game
- **P:** Pause the game
- **Esc:** Exit the game

### Controller Controls:

*Only for PS4 Controller right now.*

- **Left Stick:** Rotate the spaceship
- **X Button:** Shoot
- **R2 Button:** Accelerate the spaceship

### Choosing/Creating a Name:

- **Arrow Keys:** Navigate through the existing names list.
- **Enter:** Select the highlighted name to start the game.
- **Alt:** Save a new name to the list (after typing it).
- **Del:** Deletes a player and it's highscore.

## Running the Game

### Linux

The game is available for download as a ready-to-play package for Linux users. 
You can find the latest release [here](https://github.com/vnic03/Asteroids/releases/tag/v1.0.0). 
Download the appropriate file for your system:

- `Asteroids-1.0.0-Linux.sh` for a shell script installer.
- `Asteroids-1.0.0-Linux.tar.gz` for a gzipped tar archive.
- `Asteroids-1.0.0-Linux.tar.Z` for a compressed tar archive.

Follow the installation instructions in the release notes to install and start playing "Asteroids"!

## Others

Clone the repository and build the game using CMake. Ensure you have the SFML library installed on your system.

```bash
git clone https://github.com/yourusername/asteroids-clone.git
cd asteroids-clone
./run.sh
```

*Will be a downloadable for windows and mac soon!*

### Note

This is my first C++ project, built from scratch and aimed at learning the language. Thanks for playing^^
