#ifndef BLOCKTYPE_H
#define BLOCKTYPE_H

typedef enum {
    BLOCK_AIR,
    BLOCK_GRASS,
    BLOCK_ROCK,
    BLOCK_WATER,
    BLOCK_SAND,
    BLOCK_WOOD,
    BLOCK_NUM_BLOCK_TYPES
} BlockType;

static const char* blocktype_to_texture_path(BlockType type) {
    switch (type) {
        case BLOCK_GRASS:
            return "assets/grass.png";
        case BLOCK_ROCK:
            return "assets/rock.png";
        case BLOCK_WATER:
            return "assets/water.png";
        case BLOCK_SAND:
            return "assets/sand.png";
        case BLOCK_WOOD:
            return "assets/wood.png";
    }
    return "assets/missing.png";
}

#endif
