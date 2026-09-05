#include "Level.h"

class LevelSerializer
{
    public:
        static void Serialize(const std::string& filePath, const Level& level);
        static Level Deserialize(const std::string& filePath);
}