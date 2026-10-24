# Changelog

## Unreleased

### Added
- Separate solo, cooperative and legacy highscore records.
- Versioned score storage in the user data directory (only for Linux).
- Backup files and best-effort migration of existing scores.
- Visible messages when scores cannot be saved.
- Fallback font loading and configurable asset paths.
- Timed 30-second waves with next-wave warnings.
- Five-second breaks after clearing all enemies and scheduled spawns early.
- One-time early-clear score bonuses.
- Distinct asteroid field, alien hunt, debris, recovery and assault waves.
- Spawn warning markers and player-distance checks before enemies appear.
- Score-based extra lives with visible progress and a maximum of five lives.

### Changed
- Keyboard and controller firing now produce one shot per button press.
- Rapid individual presses fire without a player shot cooldown.
- Player creation and selection now use Enter; the separate Alt action was removed.
- Small aliens emit one aimed projectile instead of overlapping duplicates.
- Separated movement, collision handling, wave scheduling and rendering from the main gameplay loop.
- Centralized existing wave, enemy and projectile rules without intentionally changing gameplay balance.
- Replaced square-root collision distance calculations with squared-distance comparisons.
- Cached asteroid and alien collision radii after shape creation.
- Changed expired player projectile cleanup to a single removal pass.
- Reused projectile drawing shapes, the pause label and the life indicator shape.
- Moved alien visual positioning into the update phase.
- Avoided repeated stop calls for sounds that are already stopped.
- Reduced initial asteroid pressure and introduced aliens progressively.
- Distributed enemy arrivals throughout each wave.
- Limited incoming enemies using capacity reservations that account for asteroid splitting.
- Preserved surviving enemies and their original difficulty across wave transitions.
- Normalized asteroid movement speeds and introduced gradual speed scaling with upper limits.
- Gave large aliens three-shot fans and small aliens aimed shots.
- Reduced large alien health and adjusted alien score rewards.
- Added approximately 50 percent more asteroid arrivals for two surviving cooperative players.
- Removed wave-transition invulnerability while retaining start and respawn protection.
- Split longer frames into smaller simulation steps instead of discarding elapsed gameplay time.

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
