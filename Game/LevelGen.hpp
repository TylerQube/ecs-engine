#pragma once

#include <Engine/Component/Model.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <utility>
#include <vector>

// -----------------------------------------------------------------------------
// LevelGenerator
//
// Procedurally builds a dungeon by recursively splitting a rectangular area
// (binary space partitioning), turning the leaves into rooms, and connecting
// sibling subtrees with corridors. The result is converted into renderable
// meshes (floors, ceilings, walls with doorway cut-outs) and a set of point
// light spawn locations.
//
// Pipeline overview:
//   1. generateDungeon()          -> build the BSP tree (rooms as leaves)
//   2. generateModelFromDungeon() -> walk the tree to build floor/ceiling/
//                                     corridor meshes, then wall meshes with
//                                     doorway openings carved out
//   3. generatePointLightSpawns() -> walk the finished meshes and decide
//                                     where to place point lights
// -----------------------------------------------------------------------------

struct BSPLeaf {
    int x;
    int y;
    int width;
    int height;
};

struct BSPNode {
    BSPNode *childA = nullptr;
    BSPNode *childB = nullptr;

    BSPLeaf *room = nullptr;  // Non-null only for leaf nodes.

    // Bounds of this partition (not the room itself - see makeRoom()).
    int x;
    int y;
    int width;
    int height;
};

struct LevelGenerator {
    // --- Tunable generation parameters -------------------------------------
    constexpr static int CUTOFF_SIZE = 8;                  // Partitions at or below this size (in either
                                                             // axis) stop splitting and become rooms.
    constexpr static int CONNECTOR_WIDTH = 2;               // Width of corridors, in world units.
    constexpr static int ROOM_PADDING = 1;                  // Minimum gap between a room and the edge of
                                                             // the partition it was carved from.
    constexpr static int MIN_ROOM_FILL_PERCENT = 80;        // Rooms are sized to fill at least this much
                                                             // of their partition (after padding), keeping
                                                             // rooms large and partitions from feeling empty.
    constexpr static float TEXTURE_TILE_WORLD_SIZE = 2.0f;  // World units per floor/ceiling texture tile.
    constexpr static float WALL_HEIGHT = 2.0f;
    constexpr static float WALL_TILE_WORLD_SIZE = 2.0f;     // World units per wall texture tile.
    constexpr static float VISUAL_DOORWAY_CLEARANCE = 0.0f; // Extra half-width added around doorway cut-outs
                                                             // purely for visual breathing room.
    constexpr static float WALL_GAP_EPSILON = 0.01f;        // Below this size, a wall segment between two
                                                             // doorway cut-outs is skipped entirely.

    // Set to true to log each room's placement as it's carved out. Off by
    // default to keep generation quiet; flip this on when debugging layout issues.
    constexpr static bool LOG_ROOM_CREATION = false;

    enum WallSide {
        North = 0,
        East = 1,
        South = 2,
        West = 3,
    };

    // Per-room list of doorway coordinates (along the wall) for each side,
    // collected while corridors are being carved so walls can be built with
    // the right holes in them afterwards.
    using RoomOpenings = std::map<BSPLeaf *, std::array<std::vector<int>, 4>>;

    struct PointLightSpawn {
        glm::vec3 position;
        glm::vec3 color;
        float intensity;
    };

    // Shared context passed around while building a single corridor, so the
    // corridor-building functions don't need a long, repeated parameter list.
    struct BuildParams {
        const Texture *floorTexture = nullptr;
        const Texture *wallTexture = nullptr;
        const Texture *ceilingTexture = nullptr;
        RoomOpenings *roomOpenings = nullptr;
        const std::vector<BSPLeaf *> *roomsA = nullptr;
        const std::vector<BSPLeaf *> *roomsB = nullptr;
        std::string corridorPrefix = "corridor_";
        std::string wallPrefix = "wall_";
        std::string ceilingPrefix = "ceiling_";
    };

    // =========================================================================
    // 1. BSP tree generation
    // =========================================================================

    // Public entry point: builds a BSP tree of the given size, recursively
    // splitting up to max_depth times (or until partitions are too small to
    // split further, whichever comes first).
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

        bool canSplitWidth = width > CUTOFF_SIZE * 2;
        bool canSplitHeight = height > CUTOFF_SIZE * 2;

        bool reachedMaxDepth = depth >= max_depth;
        bool tooSmallToSplit = !canSplitWidth && !canSplitHeight;
        if (reachedMaxDepth || tooSmallToSplit) {
            node.room = new BSPLeaf(makeRoom(x, y, width, height));
            if constexpr (LOG_ROOM_CREATION) {
                std::cout << "Creating room at depth " << depth << " (" << x << ", " << y << ") with width " << width
                          << " and height " << height << std::endl;
            }
            return node;
        }

        bool splitVertical = chooseSplitAxis(canSplitWidth, canSplitHeight);
        if (splitVertical) {
            // Vertical cut: split width into a left and right partition.
            int splitX = randRange(CUTOFF_SIZE, width - CUTOFF_SIZE);
            node.childA = new BSPNode(generateDungeon(x, y, splitX, height, depth + 1, max_depth));
            node.childB = new BSPNode(generateDungeon(x + splitX, y, width - splitX, height, depth + 1, max_depth));
        } else {
            // Horizontal cut: split height into a top and bottom partition.
            int splitY = randRange(CUTOFF_SIZE, height - CUTOFF_SIZE);
            node.childA = new BSPNode(generateDungeon(x, y, width, splitY, depth + 1, max_depth));
            node.childB = new BSPNode(generateDungeon(x, y + splitY, width, height - splitY, depth + 1, max_depth));
        }

        return node;
    }

    // Decides which axis to split a partition along. If only one axis is
    // large enough to split, that axis is used; otherwise the choice is random.
    static bool chooseSplitAxis(bool canSplitWidth, bool canSplitHeight) {
        if (canSplitWidth && !canSplitHeight) return true;
        if (!canSplitWidth && canSplitHeight) return false;
        return (std::rand() % 2) == 0;
    }

    static int randRange(int min, int max) {
        if (min >= max)
            return min;
        return min + std::rand() % (max - min + 1);
    }

    // Carves a room out of a partition. The room is sized to fill at least
    // MIN_ROOM_FILL_PERCENT of the space left after padding, then placed at a
    // random offset within whatever slack remains - biasing toward large,
    // well-filled rooms while still varying room size and position.
    static BSPLeaf makeRoom(int x, int y, int areaWidth, int areaHeight) {
        int maxRoomW = std::max(1, areaWidth - ROOM_PADDING * 2);
        int maxRoomH = std::max(1, areaHeight - ROOM_PADDING * 2);

        int minRoomW = std::max(1, (maxRoomW * MIN_ROOM_FILL_PERCENT) / 100);
        int minRoomH = std::max(1, (maxRoomH * MIN_ROOM_FILL_PERCENT) / 100);

        int roomWidth = randRange(minRoomW, maxRoomW);
        int roomHeight = randRange(minRoomH, maxRoomH);

        int roomX = x + randRange(ROOM_PADDING, areaWidth - roomWidth - ROOM_PADDING);
        int roomY = y + randRange(ROOM_PADDING, areaHeight - roomHeight - ROOM_PADDING);

        return BSPLeaf{.x = roomX, .y = roomY, .width = roomWidth, .height = roomHeight};
    }

    // =========================================================================
    // 2. Mesh building primitives (floor / ceiling / wall quads)
    // =========================================================================

    static void assignTexture(WorldMesh &mesh, const Texture *texture) {
        if (texture) {
            mesh.material.textures.push_back(*texture);
        }
    }

    static void addFloorMesh(std::vector<WorldMesh> &meshes,
                             int x,
                             int y,
                             int width,
                             int height,
                             const std::string &prefix,
                             const Texture *floorTexture = nullptr) {
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
        assignTexture(mesh, floorTexture);
        meshes.push_back(mesh);
    }

    static void addCeilingMesh(std::vector<WorldMesh> &meshes,
                               int x,
                               int y,
                               int width,
                               int height,
                               const std::string &prefix,
                               const Texture *ceilingTexture = nullptr) {
        if (width <= 0 || height <= 0)
            return;

        float u0 = (float)x / TEXTURE_TILE_WORLD_SIZE;
        float v0 = (float)y / TEXTURE_TILE_WORLD_SIZE;
        float u1 = (float)(x + width) / TEXTURE_TILE_WORLD_SIZE;
        float v1 = (float)(y + height) / TEXTURE_TILE_WORLD_SIZE;

        WorldMesh mesh;
        mesh.vertices = {
            {{(float)x, WALL_HEIGHT, (float)y}, {0.0f, -1.0f, 0.0f}, {u0, v0}},
            {{(float)x, WALL_HEIGHT, (float)(y + height)}, {0.0f, -1.0f, 0.0f}, {u0, v1}},
            {{(float)(x + width), WALL_HEIGHT, (float)y}, {0.0f, -1.0f, 0.0f}, {u1, v0}},
            {{(float)(x + width), WALL_HEIGHT, (float)(y + height)}, {0.0f, -1.0f, 0.0f}, {u1, v1}},
        };
        mesh.indices = {0, 1, 2, 1, 3, 2};
        mesh.name = prefix + std::to_string(meshes.size());
        assignTexture(mesh, ceilingTexture);
        meshes.push_back(mesh);
    }

    // Builds a single flat wall quad from (x0,z0) to (x1,z1), with its
    // outward normal computed from the direction of travel (so winding /
    // normal direction stay consistent regardless of which way the quad runs).
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
        assignTexture(wall, wallTexture);
        meshes.push_back(wall);
    }

    // Builds up to four solid wall quads around a rectangle. Each side can be
    // individually skipped, which corridors use to leave their long edges open.
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
                                 const std::string &ceilingPrefix,
                                 const Texture *floorTexture,
                                 const Texture *wallTexture,
                                 const Texture *ceilingTexture,
                                 bool addNorthWall = true,
                                 bool addEastWall = true,
                                 bool addSouthWall = true,
                                 bool addWestWall = true) {
        addFloorMesh(meshes, x, y, width, height, floorPrefix, floorTexture);
        addCeilingMesh(meshes, x, y, width, height, ceilingPrefix, ceilingTexture);
        addWallsForRect(
            meshes, x, y, width, height, wallPrefix, wallTexture, addNorthWall, addEastWall, addSouthWall, addWestWall);
    }

    // =========================================================================
    // 3. Wall openings (doorways)
    //
    // Corridors register a doorway coordinate on each room they touch; once
    // all corridors are built, each room's walls are built with holes carved
    // out at those coordinates instead of solid quads.
    // =========================================================================

    static void registerRoomOpening(RoomOpenings &roomOpenings, BSPLeaf *room, WallSide side, int axisCoord) {
        if (!room)
            return;
        roomOpenings[room][(int)side].push_back(axisCoord);
    }

    // Returns the doorway coordinates registered for one side of a room,
    // clamped to the wall's extent, sorted, and de-duplicated.
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

    // Builds a wall running along a fixed Z, from xStart to xEnd, skipping a
    // doorway-sized gap at each coordinate in `openings`. `reverse` flips the
    // quad winding so the wall's outward normal still points away from the room
    // regardless of which direction (N vs S) it's being built in.
    static void addHorizontalWallWithOpenings(std::vector<WorldMesh> &meshes,
                                              float xStart,
                                              float xEnd,
                                              float z,
                                              bool reverse,
                                              const std::vector<int> &openings,
                                              const std::string &prefix,
                                              const Texture *wallTexture) {
        float cursor = xStart;
        float holeHalfWidth = CONNECTOR_WIDTH * 0.5f + VISUAL_DOORWAY_CLEARANCE;

        for (int opening : openings) {
            float holeStart = std::clamp((float)opening - holeHalfWidth, xStart, xEnd);
            float holeEnd = std::clamp((float)opening + holeHalfWidth, xStart, xEnd);

            if (holeStart - cursor > WALL_GAP_EPSILON) {
                if (reverse)
                    addWallQuad(meshes, holeStart, z, cursor, z, prefix, wallTexture);
                else
                    addWallQuad(meshes, cursor, z, holeStart, z, prefix, wallTexture);
            }
            cursor = std::max(cursor, holeEnd);
        }

        if (xEnd - cursor > WALL_GAP_EPSILON) {
            if (reverse)
                addWallQuad(meshes, xEnd, z, cursor, z, prefix, wallTexture);
            else
                addWallQuad(meshes, cursor, z, xEnd, z, prefix, wallTexture);
        }
    }

    // Same as addHorizontalWallWithOpenings, but for a wall running along a
    // fixed X, from zStart to zEnd.
    static void addVerticalWallWithOpenings(std::vector<WorldMesh> &meshes,
                                            float zStart,
                                            float zEnd,
                                            float x,
                                            bool reverse,
                                            const std::vector<int> &openings,
                                            const std::string &prefix,
                                            const Texture *wallTexture) {
        float cursor = zStart;
        float holeHalfWidth = CONNECTOR_WIDTH * 0.5f + VISUAL_DOORWAY_CLEARANCE;

        for (int opening : openings) {
            float holeStart = std::clamp((float)opening - holeHalfWidth, zStart, zEnd);
            float holeEnd = std::clamp((float)opening + holeHalfWidth, zStart, zEnd);

            if (holeStart - cursor > WALL_GAP_EPSILON) {
                if (reverse)
                    addWallQuad(meshes, x, holeStart, x, cursor, prefix, wallTexture);
                else
                    addWallQuad(meshes, x, cursor, x, holeStart, prefix, wallTexture);
            }
            cursor = std::max(cursor, holeEnd);
        }

        if (zEnd - cursor > WALL_GAP_EPSILON) {
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

    // Recursively builds walls (with doorway cut-outs already registered by
    // the corridor pass) for every room in the tree.
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

    // =========================================================================
    // 4. Tree / geometry query helpers shared by the corridor pass
    // =========================================================================

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

    // A node's two children are side-by-side (vertical split, i.e. the cut ran
    // vertically) if they differ in X; otherwise they're stacked (horizontal split).
    static bool isVerticalSplit(BSPNode *node) {
        return node && node->childA && node->childB && node->childA->x != node->childB->x;
    }

    // Computes how far two rooms overlap along the axis perpendicular to the
    // split (the axis a corridor between them would run "across"). Returns
    // false if they don't overlap at all on that axis.
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

    // Combines overlapRange() across every (roomA, roomB) pair to find the
    // full range of coordinates where at least one pair of rooms overlaps.
    // This is the range of coordinates connectSiblingSubtrees() will scan
    // when looking for a place to put a corridor.
    static bool computeOverlapInterval(const std::vector<BSPLeaf *> &roomsA,
                                       const std::vector<BSPLeaf *> &roomsB,
                                       bool verticalSplit,
                                       int &outMin,
                                       int &outMax) {
        outMin = std::numeric_limits<int>::max();
        outMax = std::numeric_limits<int>::min();
        for (BSPLeaf *a : roomsA) {
            for (BSPLeaf *b : roomsB) {
                int oMin = 0, oMax = 0;
                if (!overlapRange(a, b, verticalSplit, oMin, oMax))
                    continue;
                outMin = std::min(outMin, oMin);
                outMax = std::max(outMax, oMax);
            }
        }
        return outMin <= outMax;
    }

    static bool rectsOverlap(float ax, float ay, float aw, float ah, int bx, int by, int bw, int bh) {
        float ax1 = ax + aw;
        float ay1 = ay + ah;
        float bx1 = (float)(bx + bw);
        float by1 = (float)(by + bh);
        return ax < bx1 && ax1 > (float)bx && ay < by1 && ay1 > (float)by;
    }

    // Checks whether a proposed corridor rectangle would cut through any room
    // other than the two it's meant to connect.
    static bool corridorHitsOtherRooms(float corridorX,
                                       float corridorY,
                                       float corridorW,
                                       float corridorH,
                                       BSPLeaf *roomA,
                                       BSPLeaf *roomB,
                                       const std::vector<BSPLeaf *> &roomsA,
                                       const std::vector<BSPLeaf *> &roomsB) {
        auto overlapsRoom = [&](BSPLeaf *room) {
            if (!room || room == roomA || room == roomB)
                return false;
            return rectsOverlap(corridorX, corridorY, corridorW, corridorH, room->x, room->y, room->width, room->height);
        };

        for (BSPLeaf *room : roomsA) {
            if (overlapsRoom(room))
                return true;
        }
        for (BSPLeaf *room : roomsB) {
            if (overlapsRoom(room))
                return true;
        }
        return false;
    }

    // =========================================================================
    // 5. Corridor construction
    //
    // connectSiblingSubtrees() is called once per internal BSP node, after
    // both children have already been fully built. It scans outward from the
    // midpoint of the overlapping region between the two sides looking for a
    // pair of rooms it can join with a straight corridor without cutting
    // through a third room. If no safe route exists anywhere in the overlap,
    // the two subtrees are simply left unconnected (dungeon connectivity as a
    // whole is still fine as long as *some* path connects every room, which
    // higher-level connectors elsewhere in the tree provide).
    // =========================================================================

    // Given a Y coordinate, finds whichever room in `rooms` sits at that
    // height and is furthest right (pickRightMost) or furthest left. Used to
    // find the room on each side of a vertical split that a horizontal
    // corridor at that Y would actually reach.
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
        return selected;
    }

    // Same idea as rayPickRoomHorizontal, but for a horizontal split: given an
    // X coordinate, finds the furthest-down (pickBottomMost) or furthest-up room.
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
        return selected;
    }

    // Where a horizontal corridor should attach to a room's east or west wall:
    // clamps the target Y into the wall's interior (so it never lands exactly
    // on a corner) and returns the wall's X coordinate on the requested side.
    static std::pair<int, int> horizontalEndpoint(BSPLeaf *room, int yCoord, bool useRightWall) {
        if (!room)
            return {0, 0};

        int minY = room->y + 1;
        int maxY = room->y + std::max(1, room->height - 1);
        int y = std::clamp(yCoord, minY, maxY);
        int x = useRightWall ? (room->x + room->width) : room->x;
        return {x, y};
    }

    // Same as horizontalEndpoint, but for a vertical corridor attaching to a
    // room's north or south wall.
    static std::pair<int, int> verticalEndpoint(BSPLeaf *room, int xCoord, bool useBottomWall) {
        if (!room)
            return {0, 0};

        int minX = room->x + 1;
        int maxX = room->x + std::max(1, room->width - 1);
        int x = std::clamp(xCoord, minX, maxX);
        int y = useBottomWall ? (room->y + room->height) : room->y;
        return {x, y};
    }

    // Attempts to build a horizontal corridor between leftRoom's east wall and
    // rightRoom's west wall at height `y`. Fails (returns false, builds
    // nothing) if the corridor would cut through an unrelated room.
    static bool tryBuildHorizontalCorridor(std::vector<WorldMesh> &meshes,
                                           BSPLeaf *leftRoom,
                                           BSPLeaf *rightRoom,
                                           int y,
                                           const BuildParams &ctx) {
        auto [ax, ay] = horizontalEndpoint(leftRoom, y, true);
        auto [bx, by] = horizontalEndpoint(rightRoom, y, false);
        int corridorX = std::min(ax, bx);
        int corridorWidth = std::max(1, std::abs(ax - bx));
        float corridorY = (float)y - CONNECTOR_WIDTH * 0.5f;

        if (corridorHitsOtherRooms((float)corridorX,
                                   corridorY,
                                   (float)corridorWidth,
                                   (float)CONNECTOR_WIDTH,
                                   leftRoom,
                                   rightRoom,
                                   *ctx.roomsA,
                                   *ctx.roomsB)) {
            return false;
        }

        registerRoomOpening(*ctx.roomOpenings, leftRoom, East, ay);
        registerRoomOpening(*ctx.roomOpenings, rightRoom, West, by);

        addFloorAndWalls(
            meshes,
            corridorX,
            y - CONNECTOR_WIDTH / 2,
            corridorWidth,
            CONNECTOR_WIDTH,
            ctx.corridorPrefix + "h_",
            ctx.wallPrefix,
            ctx.ceilingPrefix,
            ctx.floorTexture,
            ctx.wallTexture,
            ctx.ceilingTexture,
            /*addNorthWall=*/true,
            /*addEastWall=*/false,
            /*addSouthWall=*/true,
            /*addWestWall=*/false);
        return true;
    }

    // Attempts to build a vertical corridor between topRoom's south wall and
    // bottomRoom's north wall at horizontal position `x`. Fails (returns
    // false, builds nothing) if the corridor would cut through an unrelated room.
    static bool tryBuildVerticalCorridor(std::vector<WorldMesh> &meshes,
                                         BSPLeaf *topRoom,
                                         BSPLeaf *bottomRoom,
                                         int x,
                                         const BuildParams &ctx) {
        auto [ax, ay] = verticalEndpoint(topRoom, x, true);
        auto [bx, by] = verticalEndpoint(bottomRoom, x, false);
        int corridorY = std::min(ay, by);
        int corridorHeight = std::max(1, std::abs(ay - by));
        float corridorX = (float)x - CONNECTOR_WIDTH * 0.5f;

        if (corridorHitsOtherRooms(corridorX,
                                   (float)corridorY,
                                   (float)CONNECTOR_WIDTH,
                                   (float)corridorHeight,
                                   topRoom,
                                   bottomRoom,
                                   *ctx.roomsA,
                                   *ctx.roomsB)) {
            return false;
        }

        registerRoomOpening(*ctx.roomOpenings, topRoom, South, ax);
        registerRoomOpening(*ctx.roomOpenings, bottomRoom, North, bx);

        addFloorAndWalls(
            meshes,
            x - CONNECTOR_WIDTH / 2,
            corridorY,
            CONNECTOR_WIDTH,
            corridorHeight,
            ctx.corridorPrefix + "v_",
            ctx.wallPrefix,
            ctx.ceilingPrefix,
            ctx.floorTexture,
            ctx.wallTexture,
            ctx.ceilingTexture,
            /*addNorthWall=*/false,
            /*addEastWall=*/true,
            /*addSouthWall=*/false,
            /*addWestWall=*/true);
        return true;
    }

    static BuildParams makeCorridorContext(const Texture *floorTexture,
                                           const Texture *wallTexture,
                                           const Texture *ceilingTexture,
                                           RoomOpenings &roomOpenings,
                                           const std::vector<BSPLeaf *> &roomsA,
                                           const std::vector<BSPLeaf *> &roomsB) {
        BuildParams ctx;
        ctx.floorTexture = floorTexture;
        ctx.wallTexture = wallTexture;
        ctx.ceilingTexture = ceilingTexture;
        ctx.roomOpenings = &roomOpenings;
        ctx.roomsA = &roomsA;
        ctx.roomsB = &roomsB;
        return ctx;
    }

    // Connects the two subtrees under `parent` with a single corridor, if a
    // safe route exists. Scans outward (center, center+1, center-1, center+2, ...)
    // from the midpoint of the overlapping region between the two sides so
    // that corridors tend to land near the middle of the shared wall rather
    // than jammed against one end.
    static void connectSiblingSubtrees(std::vector<WorldMesh> &meshes,
                                       BSPNode *parent,
                                       const Texture *floorTexture,
                                       const Texture *wallTexture,
                                       const Texture *ceilingTexture,
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

        int overlapMin = 0, overlapMax = 0;
        if (!computeOverlapInterval(roomsA, roomsB, verticalSplit, overlapMin, overlapMax)) {
            // No pair of rooms across the split shares any space to route a
            // straight corridor through - leave this pair of subtrees unconnected.
            return;
        }

        BuildParams ctx = makeCorridorContext(floorTexture, wallTexture, ceilingTexture, roomOpenings, roomsA, roomsB);

        int center = (overlapMin + overlapMax) / 2;
        int maxOffset = (overlapMax - overlapMin) / 2;

        for (int offset = 0; offset <= maxOffset; ++offset) {
            // At offset 0 there's only one candidate coordinate (the center);
            // beyond that, try both sides of the center at this distance.
            std::array<int, 2> candidates = {center + offset, center - offset};
            int candidateCount = (offset == 0) ? 1 : 2;

            for (int i = 0; i < candidateCount; ++i) {
                int coord = candidates[i];
                if (coord < overlapMin || coord > overlapMax)
                    continue;

                bool connected;
                if (verticalSplit) {
                    BSPLeaf *left = rayPickRoomHorizontal(roomsA, coord, /*pickRightMost=*/true);
                    BSPLeaf *right = rayPickRoomHorizontal(roomsB, coord, /*pickRightMost=*/false);
                    connected = left && right && tryBuildHorizontalCorridor(meshes, left, right, coord, ctx);
                } else {
                    BSPLeaf *top = rayPickRoomVertical(roomsA, coord, /*pickBottomMost=*/true);
                    BSPLeaf *bottom = rayPickRoomVertical(roomsB, coord, /*pickBottomMost=*/false);
                    connected = top && bottom && tryBuildVerticalCorridor(meshes, top, bottom, coord, ctx);
                }

                if (connected)
                    return;
            }
        }

        // No safe route was found anywhere in the overlap; skip this connector
        // instead of cutting through another room.
    }

    // =========================================================================
    // 6. Top-level mesh assembly (public API)
    // =========================================================================

    // Walks the tree building floor meshes for every room, then - as each
    // internal node finishes - connects its two children with a corridor.
    // Corridors register doorway openings on the rooms they touch; walls
    // (which need to know about those openings) are built afterwards in a
    // separate pass by addRoomWalls().
    static void meshesFromDungeon(BSPNode *node,
                                  std::vector<WorldMesh> &meshes,
                                  const Texture *floorTexture,
                                  const Texture *wallTexture,
                                  const Texture *ceilingTexture,
                                  RoomOpenings &roomOpenings) {
        if (!node)
            return;

        if (node->room) {
            addFloorMesh(meshes, node->room->x, node->room->y, node->room->width, node->room->height, "room", floorTexture);
            return;
        }

        meshesFromDungeon(node->childA, meshes, floorTexture, wallTexture, ceilingTexture, roomOpenings);
        meshesFromDungeon(node->childB, meshes, floorTexture, wallTexture, ceilingTexture, roomOpenings);
        connectSiblingSubtrees(meshes, node, floorTexture, wallTexture, ceilingTexture, roomOpenings);
    }

    // Public entry point: builds a complete renderable Model from a generated
    // BSP tree (floors, ceilings, corridors, and walls with doorways carved in).
    static Model generateModelFromDungeon(BSPNode *node,
                                          unsigned int shaderId,
                                          const Texture *floorTexture = nullptr,
                                          const Texture *wallTexture = nullptr,
                                          const Texture *ceilingTexture = nullptr) {
        std::vector<WorldMesh> meshes;
        RoomOpenings roomOpenings;

        // Corridors must be built first: they decide where doorways go, and
        // addRoomWalls() needs that opening data to carve the right holes.
        meshesFromDungeon(node, meshes, floorTexture, wallTexture, ceilingTexture, roomOpenings);
        addRoomWalls(node, meshes, wallTexture, roomOpenings);

        return Model{
            .shaderId = shaderId,
            .meshes = meshes,
        };
    }

    // =========================================================================
    // 7. Point light placement (public API)
    // =========================================================================

    static bool meshBoundsXZ(const WorldMesh &mesh, float &minX, float &maxX, float &minZ, float &maxZ) {
        if (mesh.vertices.empty())
            return false;

        minX = std::numeric_limits<float>::max();
        maxX = std::numeric_limits<float>::lowest();
        minZ = std::numeric_limits<float>::max();
        maxZ = std::numeric_limits<float>::lowest();

        for (const auto &v : mesh.vertices) {
            minX = std::min(minX, v.Position.x);
            maxX = std::max(maxX, v.Position.x);
            minZ = std::min(minZ, v.Position.z);
            maxZ = std::max(maxZ, v.Position.z);
        }
        return true;
    }

    // Small rooms get one central light; larger rooms get two lights spread
    // along their longer axis so the far corners aren't left dark.
    static void addRoomPointLights(const WorldMesh &roomMesh, std::vector<PointLightSpawn> &outLights) {
        float minX, maxX, minZ, maxZ;
        if (!meshBoundsXZ(roomMesh, minX, maxX, minZ, maxZ))
            return;

        float width = maxX - minX;
        float depth = maxZ - minZ;
        float centerX = (minX + maxX) * 0.5f;
        float centerZ = (minZ + maxZ) * 0.5f;
        float lightY = WALL_HEIGHT - 0.3f;

        glm::vec3 warmColor(1.0f, 0.93f, 0.80f);

        constexpr float ROOM_MULTI_LIGHT_THRESHOLD = 10.0f;
        if (std::max(width, depth) < ROOM_MULTI_LIGHT_THRESHOLD) {
            outLights.push_back({.position = glm::vec3(centerX, lightY, centerZ), .color = warmColor, .intensity = 1.0f});
            return;
        }

        float lightIntensity = 0.1f;
        if (width >= depth) {
            outLights.push_back({.position = glm::vec3(minX + width * 0.70f, lightY, centerZ), .color = warmColor, .intensity = lightIntensity});
            outLights.push_back({.position = glm::vec3(minX + width * 0.30f, lightY, centerZ), .color = warmColor, .intensity = lightIntensity});
        } else {
            outLights.push_back({.position = glm::vec3(centerX, lightY, minZ + depth * 0.30f), .color = warmColor, .intensity = lightIntensity});
            outLights.push_back({.position = glm::vec3(centerX, lightY, minZ + depth * 0.70f), .color = warmColor, .intensity = lightIntensity});
        }
    }

    // Only long corridors get lights, placed a quarter of the way in from
    // each end - short connectors are assumed to be lit well enough by the
    // rooms they join.
    static void addCorridorPointLights(const WorldMesh &corridorMesh, std::vector<PointLightSpawn> &outLights) {
        float minX, maxX, minZ, maxZ;
        if (!meshBoundsXZ(corridorMesh, minX, maxX, minZ, maxZ))
            return;

        float width = maxX - minX;
        float depth = maxZ - minZ;
        float length = std::max(width, depth);
        constexpr float HALLWAY_LIGHT_MIN_LENGTH = 14.0f;
        if (length < HALLWAY_LIGHT_MIN_LENGTH)
            return;

        float centerX = (minX + maxX) * 0.5f;
        float centerZ = (minZ + maxZ) * 0.5f;
        float lightY = WALL_HEIGHT - 0.3f;
        glm::vec3 coolColor(0.9f, 0.95f, 1.0f);

        if (width >= depth) {
            outLights.push_back({.position = glm::vec3(minX + width * 0.25f, lightY, centerZ), .color = coolColor, .intensity = 0.8f});
            outLights.push_back({.position = glm::vec3(minX + width * 0.75f, lightY, centerZ), .color = coolColor, .intensity = 0.8f});
        } else {
            outLights.push_back({.position = glm::vec3(centerX, lightY, minZ + depth * 0.25f), .color = coolColor, .intensity = 0.8f});
            outLights.push_back({.position = glm::vec3(centerX, lightY, minZ + depth * 0.75f), .color = coolColor, .intensity = 0.8f});
        }
    }

    // Public entry point: scans a finished Model's meshes (identified by the
    // name prefixes assigned during mesh building) and decides where to spawn
    // point lights for rooms and long corridors.
    static std::vector<PointLightSpawn> generatePointLightSpawns(const Model &dungeonModel) {
        std::vector<PointLightSpawn> pointLights;

        for (const auto &mesh : dungeonModel.meshes) {
            bool isRoom = mesh.name.rfind("room", 0) == 0;
            bool isCorridor = mesh.name.rfind("corridor_h_", 0) == 0 || mesh.name.rfind("corridor_v_", 0) == 0;

            if (isRoom) {
                addRoomPointLights(mesh, pointLights);
            } else if (isCorridor) {
                addCorridorPointLights(mesh, pointLights);
            }
        }

        return pointLights;
    }
};