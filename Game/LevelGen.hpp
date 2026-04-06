#include <Engine/Component/Model.h>
#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <map>
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
    constexpr static int CUTOFF_SIZE = 8;
    constexpr static int CONNECTOR_WIDTH = 2;
    constexpr static int ROOM_PADDING = 1;
    constexpr static int MIN_ROOM_FILL_PERCENT = 75;
    constexpr static float TEXTURE_TILE_WORLD_SIZE = 2.0f;
    constexpr static float WALL_HEIGHT = 2.5f;
    constexpr static float WALL_TILE_WORLD_SIZE = 2.0f;

    enum WallSide {
        North = 0,
        East = 1,
        South = 2,
        West = 3,
    };

    using RoomOpenings = std::map<BSPLeaf *, std::array<std::vector<int>, 4>>;

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
        int maxRoomW = std::max(1, areaWidth - ROOM_PADDING * 2);
        int maxRoomH = std::max(1, areaHeight - ROOM_PADDING * 2);

        int minRoomW = std::max(1, (maxRoomW * MIN_ROOM_FILL_PERCENT) / 100);
        int minRoomH = std::max(1, (maxRoomH * MIN_ROOM_FILL_PERCENT) / 100);

        int roomWidth = randRange(minRoomW, maxRoomW);
        int roomHeight = randRange(minRoomH, maxRoomH);

        // Room stays within the partition while biasing toward high fill.
        int roomX = x + randRange(ROOM_PADDING, areaWidth - roomWidth - ROOM_PADDING);
        int roomY = y + randRange(ROOM_PADDING, areaHeight - roomHeight - ROOM_PADDING);

        return BSPLeaf{.x = roomX, .y = roomY, .width = roomWidth, .height = roomHeight};
    }

    static Model generateModelFromDungeon(BSPNode *node, unsigned int shaderId, const Texture *wallTexture = nullptr) {
        std::vector<WorldMesh> meshes;
        RoomOpenings roomOpenings;
        meshesFromDungeon(node, meshes, wallTexture, roomOpenings);
        addRoomWalls(node, meshes, wallTexture, roomOpenings);

        return Model{
            .shaderId = shaderId,
            .meshes = meshes,
        };
    }

    static void addFloorMesh(std::vector<WorldMesh> &meshes, int x, int y, int width, int height, const std::string &prefix) {
        if (width <= 0 || height <= 0)
            return;

        float u0 = (float)x / TEXTURE_TILE_WORLD_SIZE;
        float v0 = (float)y / TEXTURE_TILE_WORLD_SIZE;
        float u1 = (float)(x + width) / TEXTURE_TILE_WORLD_SIZE;
        float v1 = (float)(y + height) / TEXTURE_TILE_WORLD_SIZE;

        WorldMesh mesh;
        mesh.vertices = {
            {{(float)x, 0.0f, (float)y}, {0.0f, 1.0f, 0.0f}, {u0, v0}},
            {{(float)(x + width), 0.0f, (float)y}, {0.0f, 1.0f, 0.0f}, {u1, v0}},
            {{(float)x, 0.0f, (float)(y + height)}, {0.0f, 1.0f, 0.0f}, {u0, v1}},
            {{(float)(x + width), 0.0f, (float)(y + height)}, {0.0f, 1.0f, 0.0f}, {u1, v1}},
        };
        mesh.indices = {0, 1, 2, 1, 3, 2};
        mesh.name = prefix + std::to_string(meshes.size());
        meshes.push_back(mesh);
    }

    static void addWallQuad(std::vector<WorldMesh> &meshes,
                            float x0,
                            float z0,
                            float x1,
                            float z1,
                            const std::string &prefix,
                            const Texture *wallTexture) {
        float dx = x1 - x0;
        float dz = z1 - z0;
        float len = std::max(1.0f, std::sqrt(dx * dx + dz * dz));

        float nx = dz / len;
        float nz = -dx / len;

        float u1 = len / WALL_TILE_WORLD_SIZE;
        float v1 = WALL_HEIGHT / WALL_TILE_WORLD_SIZE;

        WorldMesh wall;
        wall.vertices = {
            {{x0, 0.0f, z0}, {nx, 0.0f, nz}, {0.0f, 0.0f}},
            {{x1, 0.0f, z1}, {nx, 0.0f, nz}, {u1, 0.0f}},
            {{x0, WALL_HEIGHT, z0}, {nx, 0.0f, nz}, {0.0f, v1}},
            {{x1, WALL_HEIGHT, z1}, {nx, 0.0f, nz}, {u1, v1}},
        };
        wall.indices = {0, 1, 2, 1, 3, 2};
        wall.name = prefix + std::to_string(meshes.size());
        if (wallTexture) {
            wall.material.textures.push_back(*wallTexture);
        }
        meshes.push_back(wall);
    }

    static void addWallsForRect(std::vector<WorldMesh> &meshes,
                                int x,
                                int y,
                                int width,
                                int height,
                                const std::string &prefix,
                                const Texture *wallTexture,
                                bool addNorthWall = true,
                                bool addEastWall = true,
                                bool addSouthWall = true,
                                bool addWestWall = true) {
        float x0 = (float)x;
        float x1 = (float)(x + width);
        float z0 = (float)y;
        float z1 = (float)(y + height);

        if (addNorthWall)
            addWallQuad(meshes, x0, z0, x1, z0, prefix, wallTexture);
        if (addEastWall)
            addWallQuad(meshes, x1, z0, x1, z1, prefix, wallTexture);
        if (addSouthWall)
            addWallQuad(meshes, x1, z1, x0, z1, prefix, wallTexture);
        if (addWestWall)
            addWallQuad(meshes, x0, z1, x0, z0, prefix, wallTexture);
    }

    static void addFloorAndWalls(std::vector<WorldMesh> &meshes,
                                 int x,
                                 int y,
                                 int width,
                                 int height,
                                 const std::string &floorPrefix,
                                 const std::string &wallPrefix,
                                 const Texture *wallTexture,
                                 bool addNorthWall = true,
                                 bool addEastWall = true,
                                 bool addSouthWall = true,
                                 bool addWestWall = true) {
        addFloorMesh(meshes, x, y, width, height, floorPrefix);
        addWallsForRect(
            meshes, x, y, width, height, wallPrefix, wallTexture, addNorthWall, addEastWall, addSouthWall, addWestWall);
    }

    static void registerRoomOpening(RoomOpenings &roomOpenings, BSPLeaf *room, WallSide side, int axisCoord) {
        if (!room)
            return;
        roomOpenings[room][(int)side].push_back(axisCoord);
    }

    static std::vector<int> getSortedOpenings(const RoomOpenings &roomOpenings,
                                              BSPLeaf *room,
                                              WallSide side,
                                              int minAxis,
                                              int maxAxis) {
        std::vector<int> openings;
        auto roomIt = roomOpenings.find(room);
        if (roomIt == roomOpenings.end())
            return openings;

        openings = roomIt->second[(int)side];
        for (int &value : openings) {
            value = std::clamp(value, minAxis, maxAxis);
        }
        std::sort(openings.begin(), openings.end());
        openings.erase(std::unique(openings.begin(), openings.end()), openings.end());
        return openings;
    }

    static void addHorizontalWallWithOpenings(std::vector<WorldMesh> &meshes,
                                              float xStart,
                                              float xEnd,
                                              float z,
                                              bool reverse,
                                              const std::vector<int> &openings,
                                              const std::string &prefix,
                                              const Texture *wallTexture) {
        float cursor = xStart;
        float holeHalf = CONNECTOR_WIDTH * 0.5f;

        for (int opening : openings) {
            float holeStart = std::clamp((float)opening - holeHalf, xStart, xEnd);
            float holeEnd = std::clamp((float)opening + holeHalf, xStart, xEnd);

            if (holeStart - cursor > 0.01f) {
                if (reverse)
                    addWallQuad(meshes, holeStart, z, cursor, z, prefix, wallTexture);
                else
                    addWallQuad(meshes, cursor, z, holeStart, z, prefix, wallTexture);
            }
            cursor = std::max(cursor, holeEnd);
        }

        if (xEnd - cursor > 0.01f) {
            if (reverse)
                addWallQuad(meshes, xEnd, z, cursor, z, prefix, wallTexture);
            else
                addWallQuad(meshes, cursor, z, xEnd, z, prefix, wallTexture);
        }
    }

    static void addVerticalWallWithOpenings(std::vector<WorldMesh> &meshes,
                                            float zStart,
                                            float zEnd,
                                            float x,
                                            bool reverse,
                                            const std::vector<int> &openings,
                                            const std::string &prefix,
                                            const Texture *wallTexture) {
        float cursor = zStart;
        float holeHalf = CONNECTOR_WIDTH * 0.5f;

        for (int opening : openings) {
            float holeStart = std::clamp((float)opening - holeHalf, zStart, zEnd);
            float holeEnd = std::clamp((float)opening + holeHalf, zStart, zEnd);

            if (holeStart - cursor > 0.01f) {
                if (reverse)
                    addWallQuad(meshes, x, holeStart, x, cursor, prefix, wallTexture);
                else
                    addWallQuad(meshes, x, cursor, x, holeStart, prefix, wallTexture);
            }
            cursor = std::max(cursor, holeEnd);
        }

        if (zEnd - cursor > 0.01f) {
            if (reverse)
                addWallQuad(meshes, x, zEnd, x, cursor, prefix, wallTexture);
            else
                addWallQuad(meshes, x, cursor, x, zEnd, prefix, wallTexture);
        }
    }

    static void addRoomWallsForLeaf(std::vector<WorldMesh> &meshes,
                                    BSPLeaf *room,
                                    const Texture *wallTexture,
                                    const RoomOpenings &roomOpenings) {
        if (!room)
            return;

        float x0 = (float)room->x;
        float x1 = (float)(room->x + room->width);
        float z0 = (float)room->y;
        float z1 = (float)(room->y + room->height);

        auto northOpenings = getSortedOpenings(roomOpenings, room, North, room->x, room->x + room->width);
        auto southOpenings = getSortedOpenings(roomOpenings, room, South, room->x, room->x + room->width);
        auto eastOpenings = getSortedOpenings(roomOpenings, room, East, room->y, room->y + room->height);
        auto westOpenings = getSortedOpenings(roomOpenings, room, West, room->y, room->y + room->height);

        addHorizontalWallWithOpenings(meshes, x0, x1, z0, false, northOpenings, "wall_", wallTexture);
        addVerticalWallWithOpenings(meshes, z0, z1, x1, false, eastOpenings, "wall_", wallTexture);
        addHorizontalWallWithOpenings(meshes, x0, x1, z1, true, southOpenings, "wall_", wallTexture);
        addVerticalWallWithOpenings(meshes, z0, z1, x0, true, westOpenings, "wall_", wallTexture);
    }

    static void addRoomWalls(BSPNode *node,
                             std::vector<WorldMesh> &meshes,
                             const Texture *wallTexture,
                             const RoomOpenings &roomOpenings) {
        if (!node)
            return;

        if (node->room) {
            addRoomWallsForLeaf(meshes, node->room, wallTexture, roomOpenings);
            return;
        }

        addRoomWalls(node->childA, meshes, wallTexture, roomOpenings);
        addRoomWalls(node->childB, meshes, wallTexture, roomOpenings);
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
            outMin = std::max(a->y + 1, b->y + 1);
            outMax = std::min(a->y + a->height - 1, b->y + b->height - 1);
        } else {
            outMin = std::max(a->x + 1, b->x + 1);
            outMax = std::min(a->x + a->width - 1, b->x + b->width - 1);
        }
        return outMin <= outMax;
    }

    static bool pickBestOverlappingPair(const std::vector<BSPLeaf *> &roomsA,
                                        const std::vector<BSPLeaf *> &roomsB,
                                        bool verticalSplit,
                                        BSPLeaf *&outA,
                                        BSPLeaf *&outB,
                                        int &outMin,
                                        int &outMax) {
        bool found = false;
        int bestScore = std::numeric_limits<int>::min();

        for (BSPLeaf *a : roomsA) {
            for (BSPLeaf *b : roomsB) {
                int interMin = 0;
                int interMax = 0;
                if (!overlapRange(a, b, verticalSplit, interMin, interMax))
                    continue;

                int overlapSpan = interMax - interMin;
                int gap = verticalSplit ? std::abs((a->x + a->width) - b->x)
                                        : std::abs((a->y + a->height) - b->y);
                int score = overlapSpan * 100 - gap;

                if (!found || score > bestScore) {
                    found = true;
                    bestScore = score;
                    outA = a;
                    outB = b;
                    outMin = interMin;
                    outMax = interMax;
                }
            }
        }

        return found;
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

    static void connectSiblingSubtrees(std::vector<WorldMesh> &meshes,
                                       BSPNode *parent,
                                       const Texture *wallTexture,
                                       RoomOpenings &roomOpenings) {
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
            int broadOverlapY = pickRandomOverlapValue(
                roomsA,
                roomsB,
                true,
                std::max(parent->childA->y, parent->childB->y),
                std::min(parent->childA->y + parent->childA->height, parent->childB->y + parent->childB->height));

            BSPLeaf *leftRoom = rayPickRoomHorizontal(roomsA, broadOverlapY, true);
            BSPLeaf *rightRoom = rayPickRoomHorizontal(roomsB, broadOverlapY, false);

            int overlapMin = 0;
            int overlapMax = 0;
            if (!leftRoom || !rightRoom || !overlapRange(leftRoom, rightRoom, true, overlapMin, overlapMax)) {
                if (!pickBestOverlappingPair(roomsA, roomsB, true, leftRoom, rightRoom, overlapMin, overlapMax))
                    return;
            }

            int overlapY = randRange(overlapMin, overlapMax);

            auto [ax, ay] = horizontalEndpoint(leftRoom, overlapY, true);
            auto [bx, by] = horizontalEndpoint(rightRoom, overlapY, false);
            int y = overlapY;
            int corridorX = std::min(ax, bx);
            int corridorWidth = std::max(1, std::abs(ax - bx));

            registerRoomOpening(roomOpenings, leftRoom, East, ay);
            registerRoomOpening(roomOpenings, rightRoom, West, by);

            addFloorAndWalls(
                meshes,
                corridorX,
                y - CONNECTOR_WIDTH / 2,
                corridorWidth,
                CONNECTOR_WIDTH,
                "corridor_h_",
                "wall_",
                wallTexture,
                true,
                false,
                true,
                false);
        } else {
            int broadOverlapX = pickRandomOverlapValue(
                roomsA,
                roomsB,
                false,
                std::max(parent->childA->x, parent->childB->x),
                std::min(parent->childA->x + parent->childA->width, parent->childB->x + parent->childB->width));

            BSPLeaf *topRoom = rayPickRoomVertical(roomsA, broadOverlapX, true);
            BSPLeaf *bottomRoom = rayPickRoomVertical(roomsB, broadOverlapX, false);

            int overlapMin = 0;
            int overlapMax = 0;
            if (!topRoom || !bottomRoom || !overlapRange(topRoom, bottomRoom, false, overlapMin, overlapMax)) {
                if (!pickBestOverlappingPair(roomsA, roomsB, false, topRoom, bottomRoom, overlapMin, overlapMax))
                    return;
            }

            int overlapX = randRange(overlapMin, overlapMax);

            auto [ax, ay] = verticalEndpoint(topRoom, overlapX, true);
            auto [bx, by] = verticalEndpoint(bottomRoom, overlapX, false);
            int x = overlapX;
            int corridorY = std::min(ay, by);
            int corridorHeight = std::max(1, std::abs(ay - by));

            registerRoomOpening(roomOpenings, topRoom, South, ax);
            registerRoomOpening(roomOpenings, bottomRoom, North, bx);

            addFloorAndWalls(
                meshes,
                x - CONNECTOR_WIDTH / 2,
                corridorY,
                CONNECTOR_WIDTH,
                corridorHeight,
                "corridor_v_",
                "wall_",
                wallTexture,
                false,
                true,
                false,
                true);
        }
    }

    static void meshesFromDungeon(BSPNode *node,
                                  std::vector<WorldMesh> &meshes,
                                  const Texture *wallTexture,
                                  RoomOpenings &roomOpenings) {
        if (!node)
            return;

        if (node->room) {
            addFloorMesh(meshes, node->room->x, node->room->y, node->room->width, node->room->height, "room");
            return;
        }

        meshesFromDungeon(node->childA, meshes, wallTexture, roomOpenings);
        meshesFromDungeon(node->childB, meshes, wallTexture, roomOpenings);
        connectSiblingSubtrees(meshes, node, wallTexture, roomOpenings);
    }
};