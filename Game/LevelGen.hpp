#include <vector>

const int DUNGEON_WIDTH = 50;
const int DUNGEON_HEIGHT = 50;

struct BSPNode {
    BSPNode* childA = nullptr;
    BSPNode* childB = nullptr;

    BSPLeaf* room = nullptr;

    int x;
    int y;
    int width;
    int height;
};

struct BSPLeaf {
    int x;
    int y;
    int width;
    int height;
};

// struct Connector {
//     BSPLeaf source;
//     BSPLeaf dest;
//     int x;
//     int y;
//     int width;
// };

struct LevelGenerator {
    const static int ROOM_MIN_DIMENSION = 4;
    const static int MIN_MARGIN = 1;
    const static int MIN_LEAF_DIMENSION = ROOM_MIN_DIMENSION - 2 * MIN_MARGIN;

    static BSPNode generateDungeon(int x, int y, int width, int height) {
        BSPNode node = BSPNode{
            .x = x,
            .y = y,
            .width = width,
            .height = height,
        };

        if(width < MIN_LEAF_DIMENSION * 2 || height < MIN_LEAF_DIMENSION * 2) {
            node.room = &makeRoom(x, y, width, height);
            return node;
        }

        int splitDir = std::rand() % 2;
        if(splitDir) {
            int split = randRange(x + ROOM_MIN_DIMENSION, x + width - ROOM_MIN_DIMENSION);
            node.childA = &generateDungeon(x, y, split, height);
            node.childB = &generateDungeon(x + split, y, width - split, height);
        } else {
            int split = randRange(y + ROOM_MIN_DIMENSION, y + width - ROOM_MIN_DIMENSION);
            node.childA = &generateDungeon(x, y, width, split);
            node.childB = &generateDungeon(x, y + split, width, height - split);
        }
        return node;
    }

    static int randRange(int min, int max) {
        return min + std::rand() % (max - min + 1);
    }

    static BSPLeaf makeRoom(int x, int y, int areaWidth, int areaHeight) {
        int roomWidth = randRange(ROOM_MIN_DIMENSION, areaWidth - MIN_MARGIN * 2);
        int roomHeight = randRange(ROOM_MIN_DIMENSION, areaHeight - MIN_MARGIN * 2);

        int wMargin = areaWidth - roomWidth;
        int hMargin = areaHeight - roomHeight;

        int roomX = x + randRange(MIN_MARGIN, wMargin - MIN_MARGIN);
        int roomY = y + randRange(MIN_MARGIN, hMargin - MIN_MARGIN);
        return BSPLeaf {
            .x = roomX,
            .y = roomY,
            .width = roomWidth,
            .height = roomHeight
        };
    }

};