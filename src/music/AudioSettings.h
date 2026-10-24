#pragma once

namespace AudioSettings {

// Multipliers: 0.0 = silent, 1.0 = full volume.
inline constexpr float Master = 1.f;
inline constexpr float Music = 1.f;
inline constexpr float Effects = 1.f;

// Individual effect volumes: 0 to 100.
inline constexpr float Thrust = 30.f;
inline constexpr float Fire = 35.f;
inline constexpr float OtherEffects = 100.f;

} // namespace AudioSettings
