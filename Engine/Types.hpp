#pragma once
#include <cstdint>
#include <bitset>

const int MAX_COMPONENTS = 32;
const int MAX_ENTITIES = 10000;

using Entity = std::uint32_t;
using Signature = std::bitset<MAX_COMPONENTS>;