# Changelog

## Unreleased

### Added

- Local two-player cooperative mode.
- Separate solo, cooperative and legacy highscores.
- Versioned Linux score storage with backups, legacy migration and visible save errors.
- Fallback font loading and configurable asset paths.
- Timed 30-second waves with varied enemy compositions.
- Five-second breaks and a one-time bonus after clearing a wave early.
- Score-based extra lives, capped at five.
- Player-distance checks before enemies spawn.
- Sound toggle with M and centralized volume settings in code.
- Linux launch script and shared clang-format configuration.

### Changed

- Expanded the playfield from 900 × 700 to 1460 × 800.
- Rebalanced asteroid counts, movement speeds, alien health and score rewards.
- Introduced aliens progressively and distributed spawns throughout each wave.
- Limited enemy arrivals while accounting for future asteroid fragments.
- Preserved surviving enemies and their original difficulty across waves.
- Gave large aliens three-shot fans and small aliens single aimed shots.
- Increased asteroid arrivals for two surviving cooperative players.
- Replaced level-based extra lives with score thresholds.
- Removed wave-transition invulnerability; start and respawn protection remain.
- Firing produces one shot per button press without a player shot cooldown.
- Player creation and selection now use Enter instead of the separate Alt action.
- Simplified the HUD to show the wave number and only the final five seconds
  before the next wave.
- Replaced wave-reset music with adaptive tempo, volume and rhythm based on
  current threats and wave progression.
- Reduced thrust and shared player/alien firing sound volumes.
- Separated movement, collisions, wave scheduling, rendering and music control.
- Centralized gameplay rules and reduced repeated collision, projectile
  cleanup and rendering work.
- Split long frames into smaller simulation steps.

### Fixed

- Enemy movement updating twice in cooperative mode.
- Gameplay timers continuing while paused.
- Alien projectiles disappearing before applying damage or remaining active
  indefinitely outside the screen.
- Destroyed aliens awarding points repeatedly and rammed aliens staying active.
- Player deletion corrupting remaining highscore records.
- Score display failing at 100,000 points.
- Missing sound files preventing startup.
- Busy waiting in name selection and game-over screens.
- Catch-up music bursts after frame stalls.
