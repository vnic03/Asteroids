# Changelog

## Unreleased

### Added
- Separate solo, cooperative and legacy highscore records.
- Versioned score storage in the user data directory (only for Linux).
- Backup files and best-effort migration of existing scores.
- Visible messages when scores cannot be saved.
- Fallback font loading and configurable asset paths.

### Changed
- Keyboard and controller firing now produce one shot per button press.
- Rapid individual presses fire without a player shot cooldown.
- Player creation and selection now use Enter; the separate Alt action was removed.
- Small aliens emit one aimed projectile instead of overlapping duplicates.

### Fixed
- Enemy movement being updated twice in cooperative mode.
- Gameplay timers continuing while paused.
- Alien projectiles disappearing before their damage was applied.
- Destroyed aliens awarding points more than once.
- Rammed aliens remaining active.
- Player deletion corrupting remaining highscore records.
- Score display failing at 100,000 points.
- Alien projectiles remaining active indefinitely outside the screen.
- Missing sound files preventing startup.
- Busy waiting in name selection and game-over screens.
