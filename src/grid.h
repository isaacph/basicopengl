#ifndef SRC_GRID_H_INCLUDED
#define SRC_GRID_H_INCLUDED
#include <map>
#include <vector>
#include <glm/glm.hpp>
#include "events.h"
#include "physics.h"

const int GRID_SIZE = 16;
using BlockType = char;
const BlockType air=0;
const int TILE_SHEET_WIDTH = 8;
const int TILE_SHEET_HEIGHT = 8;

class Grid {
public:
    BlockType blocks[GRID_SIZE * GRID_SIZE];
    inline Grid() {
        std::fill(blocks, blocks + GRID_SIZE * GRID_SIZE, air);
    }
};

class GridPos {
public:
    int x, y;
    inline bool operator<(const GridPos& other) const {
        return x == other.x ? y < other.y : x < other.x;
    }
};

class GridManager {
public:
    BlockType set(BlockType type, int worldx, int worldy);
    void setGrid(Grid grid, int gridx, int gridy);
    BlockType check(int worldx, int worldy) const;
    std::map<GridPos, Grid> grids;
    Event<std::pair<GridPos, Grid>> gridChanges;
};

std::vector<float> makeTexturedBuffer(const Grid& grid);
std::pair<glm::vec2, glm::vec2> getSpriteSheetCoordinates(int sheetTilesX, int sheetTilesY, int index);
Grid randomGrid();
std::vector<GridPos> overlappingTiles(const Convex& convex);
Box tileBox(int tileX, int tileY);

#endif
