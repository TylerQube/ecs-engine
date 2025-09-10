#pragma once
#include <cstdint>
#include <bitset>

const int MAX_COMPONENTS = 32;
const int MAX_ENTITIES = 10000;

using Entity = std::uint32_t;
using Signature = std::bitset<MAX_COMPONENTS>;

enum KeyCode { W, A, S, D, SPACE, LEFT_SHIFT, ESCAPE }; 
enum KeyAction { PRESS, RELEASE, REPEAT };