#include <vector>
#include <string>
#include <Engine/Component/Model.h>

const int DUNGEON_WIDTH = 50;
const int DUNGEON_HEIGHT = 50;

struct BSPLeaf {
    int x;
    int y;
    int width;
    int height;
};

struct BSPNode {
    BSPNode* childA = nullptr;
    BSPNode* childB = nullptr;

    BSPLeaf* room = nullptr;

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
    const static int ROOM_MIN_DIMENSION = 8;
    const static int ROOM_MAX_DIMENSION = 15;
    const static int MIN_MARGIN = 1;

    static BSPNode generateDungeon(int x, int y, int width, int height) {
        BSPNode node = BSPNode{
            .x = x,
            .y = y,
            .width = width,
            .height = height,
        };

        std::cout << "\nwidth: " << width << " height: " << height << std::endl;

        int splitVertical = std::rand() % 2;
        if(splitVertical && width >= ROOM_MIN_DIMENSION * 2 + MIN_MARGIN * 2) {
            int split = width / 2;
            // int split = randRange(ROOM_MIN_DIMENSION, width - ROOM_MIN_DIMENSION);
            node.childA = new BSPNode(generateDungeon(x, y, split, height));
            node.childB = new BSPNode(generateDungeon(x + split, y, width - split, height));
        } else if (height >= ROOM_MIN_DIMENSION * 2 + MIN_MARGIN * 2) {
            int split = height / 2;
            // int split = randRange(ROOM_MIN_DIMENSION, height - ROOM_MIN_DIMENSION);
            node.childA = new BSPNode(generateDungeon(x, y, width, split));
            node.childB = new BSPNode(generateDungeon(x, y + split, width, height - split));
        } else {
            std::cout << "Creating room at (" << x << ", " << y << ") with dimensions " << width << "x" << height << std::endl;
            node.room = new BSPLeaf(makeRoom(x, y, width, height));
            std::cout << "Created room at (" << node.room->x << ", " << node.room->y << ") with dimensions " << node.room->width << "x" << node.room->height << std::endl;
        }


        return node;
    }

    static int randRange(int min, int max) {
        if(min >= max) assert(0 && "Invalid range for randRange");
        return min + std::rand() % (max - min + 1);
    }

    static BSPLeaf makeRoom(int x, int y, int areaWidth, int areaHeight) {
        int roomWidth = randRange(ROOM_MIN_DIMENSION, areaWidth - MIN_MARGIN * 2);
        int roomHeight = randRange(ROOM_MIN_DIMENSION, areaHeight - MIN_MARGIN * 2);

        int wMargin = areaWidth - roomWidth;
        int hMargin = areaHeight - roomHeight;


        // int roomX = x + randRange(MIN_MARGIN, wMargin - MIN_MARGIN);
        // int roomY = y + randRange(MIN_MARGIN, hMargin - MIN_MARGIN);
        return BSPLeaf {
            .x = x,
            .y = y,
            .width = roomWidth,
            .height = roomHeight
        };
    }

    static Model generateModelFromDungeon(BSPNode* node, unsigned int shaderId) {
        std::vector<WorldMesh> meshes;
        meshesFromDungeon(node, meshes);

        return Model{
            .shaderId = shaderId,
            .meshes = meshes,
        };
    }

    static void meshesFromDungeon(BSPNode* node, std::vector<WorldMesh>& meshes) {
        if(node->room) {
            WorldMesh mesh;
            mesh.vertices = {
                {{node->room->x, 0.0f, node->room->y}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                {{node->room->x + node->room->width, 0.0f, node->room->y}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
                {{node->room->x, 0.0f, node->room->y + node->room->height}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
                {{node->room->x + node->room->width, 0.0f, node->room->y + node->room->height}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            };
            mesh.indices = {0, 1, 2, 1, 3, 2};
            mesh.name = "room" + std::to_string(meshes.size());
            meshes.push_back(mesh);
        } else {
            // create model for corridor between childA and childB
            meshesFromDungeon(node->childA, meshes);
            meshesFromDungeon(node->childB, meshes);
        }
    }

};