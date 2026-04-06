#include <Engine/Component/Model.h>
#include <algorithm>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

struct BSPLeaf {
    int x;
    int y;
    int width;
    int height;
};

struct BSPNode {
    BSPNode *childA = nullptr;
    BSPNode *childB = nullptr;

    BSPLeaf *room = nullptr;

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
    constexpr static int CUTOFF_SIZE = 20;
    constexpr static int CONNECTOR_WIDTH = 4;

    static BSPNode generateDungeon(int x, int y, int width, int height, int max_depth) {
        return generateDungeon(x, y, width, height, 0, max_depth);
    }

    static BSPNode generateDungeon(int x, int y, int width, int height, int depth, int max_depth) {
        BSPNode node = BSPNode{
            .x = x,
            .y = y,
            .width = width,
            .height = height,
        };

        bool canSplitH = width > CUTOFF_SIZE * 2;
        bool canSplitV = height > CUTOFF_SIZE * 2;
        if (depth >= max_depth || (!canSplitH && !canSplitV)) {
            std::cout << "Creating room at depth " << depth << " (" << x << ", " << y << ") with width " << width
                      << " and height " << height << std::endl;
            node.room = new BSPLeaf(makeRoom(x, y, width, height));
            return node;
        }

        int splitVertical;
        if (canSplitH && !canSplitV)
            splitVertical = 1;
        else if (!canSplitH && canSplitV)
            splitVertical = 0;
        else
            splitVertical = std::rand() % 2;

        if (splitVertical) {
            int split = randRange(CUTOFF_SIZE, width - CUTOFF_SIZE);
            node.childA = new BSPNode(generateDungeon(x, y, split, height, depth + 1, max_depth));
            node.childB = new BSPNode(generateDungeon(x + split, y, width - split, height, depth + 1, max_depth));
        } else {
            int split = randRange(CUTOFF_SIZE, height - CUTOFF_SIZE);
            node.childA = new BSPNode(generateDungeon(x, y, width, split, depth + 1, max_depth));
            node.childB = new BSPNode(generateDungeon(x, y + split, width, height - split, depth + 1, max_depth));
        }

        return node;
    }

    static int randRange(int min, int max) {
        // if(min >= max) assert(0 && "Invalid range for randRange");
        if (min >= max)
            return min;
        return min + std::rand() % (max - min + 1);
    }

    static BSPLeaf makeRoom(int x, int y, int areaWidth, int areaHeight) {
        constexpr int PADDING = 2;
        int maxRoomW = std::min(areaWidth - PADDING * 2, CUTOFF_SIZE - 2);
        int maxRoomH = std::min(areaHeight - PADDING * 2, CUTOFF_SIZE - 2);
        int roomWidth = randRange(CUTOFF_SIZE * 2 / 3, maxRoomW);
        int roomHeight = randRange(CUTOFF_SIZE * 2 / 3, maxRoomH);

        // Room must stay within [x+PADDING, x+areaWidth-PADDING]
        int roomX = x + randRange(PADDING, areaWidth - roomWidth - PADDING);
        int roomY = y + randRange(PADDING, areaHeight - roomHeight - PADDING);

        return BSPLeaf{.x = roomX, .y = roomY, .width = roomWidth, .height = roomHeight};
    }

    static Model generateModelFromDungeon(BSPNode *node, unsigned int shaderId) {
        std::vector<WorldMesh> meshes;
        meshesFromDungeon(node, meshes);

        return Model{
            .shaderId = shaderId,
            .meshes = meshes,
        };
    }

    static void addFloorMesh(std::vector<WorldMesh> &meshes, int x, int y, int width, int height, const std::string &prefix) {
        if (width <= 0 || height <= 0)
            return;

        WorldMesh mesh;
        mesh.vertices = {
            {{(float)x, 0.0f, (float)y}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{(float)(x + width), 0.0f, (float)y}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{(float)x, 0.0f, (float)(y + height)}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
            {{(float)(x + width), 0.0f, (float)(y + height)}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        };
        mesh.indices = {0, 1, 2, 1, 3, 2};
        mesh.name = prefix + std::to_string(meshes.size());
        meshes.push_back(mesh);
    }

    static std::pair<int, int> roomCenter(const BSPLeaf *room) {
        return {room->x + room->width / 2, room->y + room->height / 2};
    }

    static BSPLeaf *findAnyRoom(BSPNode *node) {
        if (!node)
            return nullptr;
        if (node->room)
            return node->room;

        BSPLeaf *left = findAnyRoom(node->childA);
        if (left)
            return left;
        return findAnyRoom(node->childB);
    }

    static void collectRooms(BSPNode *node, std::vector<BSPLeaf *> &outRooms) {
        if (!node)
            return;
        if (node->room) {
            outRooms.push_back(node->room);
            return;
        }
        collectRooms(node->childA, outRooms);
        collectRooms(node->childB, outRooms);
    }

    static bool isVerticalSplit(BSPNode *node) {
        return node && node->childA && node->childB && node->childA->x != node->childB->x;
    }

    static bool overlapRange(const BSPLeaf *a,
                             const BSPLeaf *b,
                             bool verticalSplit,
                             int &outMin,
                             int &outMax) {
        if (verticalSplit) {
            outMin = std::max(a->y, b->y);
            outMax = std::min(a->y + a->height, b->y + b->height);
        } else {
            outMin = std::max(a->x, b->x);
            outMax = std::min(a->x + a->width, b->x + b->width);
        }
        return outMin < outMax;
    }

    static int pickRandomOverlapValue(const std::vector<BSPLeaf *> &leftRooms,
                                      const std::vector<BSPLeaf *> &rightRooms,
                                      bool verticalSplit,
                                      int fallbackMin,
                                      int fallbackMax) {
        int bestMin = fallbackMin;
        int bestMax = fallbackMax;
        int bestSpan = std::max(0, fallbackMax - fallbackMin);

        for (const BSPLeaf *a : leftRooms) {
            for (const BSPLeaf *b : rightRooms) {
                int interMin = 0;
                int interMax = 0;
                if (!overlapRange(a, b, verticalSplit, interMin, interMax))
                    continue;

                int span = interMax - interMin;
                if (span > bestSpan) {
                    bestSpan = span;
                    bestMin = interMin;
                    bestMax = interMax;
                }
            }
        }

        return randRange(bestMin, bestMax);
    }

    static BSPLeaf *rayPickRoomHorizontal(const std::vector<BSPLeaf *> &rooms, int yCoord, bool pickRightMost) {
        BSPLeaf *selected = nullptr;
        int selectedScore = pickRightMost ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

        for (BSPLeaf *room : rooms) {
            if (yCoord < room->y || yCoord > room->y + room->height)
                continue;

            int score = pickRightMost ? (room->x + room->width) : room->x;
            if (!selected || (pickRightMost && score > selectedScore) || (!pickRightMost && score < selectedScore)) {
                selected = room;
                selectedScore = score;
            }
        }

        if (!selected && !rooms.empty()) {
            selected = rooms[0];
            for (BSPLeaf *room : rooms) {
                if (pickRightMost) {
                    if (room->x + room->width > selected->x + selected->width)
                        selected = room;
                } else {
                    if (room->x < selected->x)
                        selected = room;
                }
            }
        }

        return selected;
    }

    static BSPLeaf *rayPickRoomVertical(const std::vector<BSPLeaf *> &rooms, int xCoord, bool pickBottomMost) {
        BSPLeaf *selected = nullptr;
        int selectedScore = pickBottomMost ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

        for (BSPLeaf *room : rooms) {
            if (xCoord < room->x || xCoord > room->x + room->width)
                continue;

            int score = pickBottomMost ? (room->y + room->height) : room->y;
            if (!selected || (pickBottomMost && score > selectedScore) || (!pickBottomMost && score < selectedScore)) {
                selected = room;
                selectedScore = score;
            }
        }

        if (!selected && !rooms.empty()) {
            selected = rooms[0];
            for (BSPLeaf *room : rooms) {
                if (pickBottomMost) {
                    if (room->y + room->height > selected->y + selected->height)
                        selected = room;
                } else {
                    if (room->y < selected->y)
                        selected = room;
                }
            }
        }

        return selected;
    }

    static std::pair<int, int> horizontalEndpoint(BSPLeaf *room, int yCoord, bool useRightWall) {
        if (!room)
            return {0, 0};

        int minY = room->y + 1;
        int maxY = room->y + std::max(1, room->height - 1);
        int y = std::clamp(yCoord, minY, maxY);
        int x = useRightWall ? (room->x + room->width) : room->x;
        return {x, y};
    }

    static std::pair<int, int> verticalEndpoint(BSPLeaf *room, int xCoord, bool useBottomWall) {
        if (!room)
            return {0, 0};

        int minX = room->x + 1;
        int maxX = room->x + std::max(1, room->width - 1);
        int x = std::clamp(xCoord, minX, maxX);
        int y = useBottomWall ? (room->y + room->height) : room->y;
        return {x, y};
    }

    static void connectSiblingSubtrees(std::vector<WorldMesh> &meshes, BSPNode *parent) {
        if (!parent || !parent->childA || !parent->childB)
            return;

        std::vector<BSPLeaf *> roomsA;
        std::vector<BSPLeaf *> roomsB;
        collectRooms(parent->childA, roomsA);
        collectRooms(parent->childB, roomsB);
        if (roomsA.empty() || roomsB.empty())
            return;

        bool verticalSplit = isVerticalSplit(parent);

        if (verticalSplit) {
            int overlapY = pickRandomOverlapValue(
                roomsA,
                roomsB,
                true,
                std::max(parent->childA->y, parent->childB->y),
                std::min(parent->childA->y + parent->childA->height, parent->childB->y + parent->childB->height));

            BSPLeaf *leftRoom = rayPickRoomHorizontal(roomsA, overlapY, true);
            BSPLeaf *rightRoom = rayPickRoomHorizontal(roomsB, overlapY, false);
            if (!leftRoom || !rightRoom) {
                leftRoom = findAnyRoom(parent->childA);
                rightRoom = findAnyRoom(parent->childB);
                if (!leftRoom || !rightRoom)
                    return;
                auto [lx, ly] = roomCenter(leftRoom);
                auto [rx, ry] = roomCenter(rightRoom);
                overlapY = (ly + ry) / 2;
            }

            auto [ax, ay] = horizontalEndpoint(leftRoom, overlapY, true);
            auto [bx, by] = horizontalEndpoint(rightRoom, overlapY, false);
            int y = (ay + by) / 2;
            int corridorX = std::min(ax, bx);
            int corridorWidth = std::max(1, std::abs(ax - bx));
            addFloorMesh(meshes, corridorX, y - CONNECTOR_WIDTH / 2, corridorWidth, CONNECTOR_WIDTH, "corridor_h_");
        } else {
            int overlapX = pickRandomOverlapValue(
                roomsA,
                roomsB,
                false,
                std::max(parent->childA->x, parent->childB->x),
                std::min(parent->childA->x + parent->childA->width, parent->childB->x + parent->childB->width));

            BSPLeaf *topRoom = rayPickRoomVertical(roomsA, overlapX, true);
            BSPLeaf *bottomRoom = rayPickRoomVertical(roomsB, overlapX, false);
            if (!topRoom || !bottomRoom) {
                topRoom = findAnyRoom(parent->childA);
                bottomRoom = findAnyRoom(parent->childB);
                if (!topRoom || !bottomRoom)
                    return;
                auto [tx, ty] = roomCenter(topRoom);
                auto [bx, by] = roomCenter(bottomRoom);
                overlapX = (tx + bx) / 2;
            }

            auto [ax, ay] = verticalEndpoint(topRoom, overlapX, true);
            auto [bx, by] = verticalEndpoint(bottomRoom, overlapX, false);
            int x = (ax + bx) / 2;
            int corridorY = std::min(ay, by);
            int corridorHeight = std::max(1, std::abs(ay - by));
            addFloorMesh(meshes, x - CONNECTOR_WIDTH / 2, corridorY, CONNECTOR_WIDTH, corridorHeight, "corridor_v_");
        }
    }

    static void meshesFromDungeon(BSPNode *node, std::vector<WorldMesh> &meshes) {
        if (!node)
            return;

        if (node->room) {
            addFloorMesh(meshes, node->room->x, node->room->y, node->room->width, node->room->height, "room");
            return;
        }

        meshesFromDungeon(node->childA, meshes);
        meshesFromDungeon(node->childB, meshes);
        connectSiblingSubtrees(meshes, node);
    }
};