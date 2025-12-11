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

static unsigned int blocktype_to_atlas_index(BlockType type) {
    switch (type) {
        case BLOCK_GRASS:
            return 0;
        case BLOCK_ROCK:
            return 2;
        case BLOCK_WATER:
            return 3;
        case BLOCK_SAND:
            return 1;
        case BLOCK_WOOD:
            return 4;
    }
    return -1;
}

#endif
