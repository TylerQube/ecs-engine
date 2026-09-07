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
    float viewDistance;
    float fovAngle;
    float moveSpeed;
    float rotationSpeed;
    float attackRange;
};