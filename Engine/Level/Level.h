#include <string>
#include <vector>

struct Level {
    Room* startingRoom;
};

struct Room {
    int id;
    std::string name;
    std::vector<int> neighbors; // IDs of adjacent rooms
    // Add other properties as needed, e.g., description, entities, etc.
};