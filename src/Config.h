#pragma once

namespace Config
{
    // Это единицы игрового мира, а не физические пиксели монитора.
    constexpr float WorldWidth = 1280.f;
    constexpr float WorldHeight = 720.f;
    constexpr float FixedStep = 1.f / 120.f;
    constexpr float Gravity = 1800.f;
    constexpr float Acceleration = 2000.f;
    constexpr float Braking = 2400.f;
    constexpr float WalkSpeed = 310.f;
    constexpr float JumpSpeed = 690.f;
    constexpr float EffectDuration = 10.f;
    constexpr float BonusRespawnDelay = 7.f;
    constexpr float DamageCooldown = 1.f;
    constexpr int MaximumHealth = 100;
    constexpr int ContactDamage = 25;
    constexpr int FallDamage = 25;
}
