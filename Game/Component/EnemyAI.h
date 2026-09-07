#pragma once

#include <string>

enum class EnemyState {
    Passive,
    Following,
    Attacking
};

inline std::string EnemyStateToString(EnemyState state) {
    switch (state) {
    case EnemyState::Passive:
        return "Passive";
    case EnemyState::Following:
        return "Following";
    case EnemyState::Attacking:
        return "Attacking";
    default:
        return "Unknown";
    }
}

struct EnemyAI {
    EnemyState state = EnemyState::Passive;
    float timeSinceSeenPlayer = 0.0f;
    float lostPlayerCooldown = 2.0f;
    float viewDistance;
    float fovAngle;
    float moveSpeed;
    float rotationSpeed;
    float attackRange;
};