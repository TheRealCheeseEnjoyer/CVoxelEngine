#include "VoxelEngine.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <json-c/json.h>

#include "Block.h"
#include "BlockId.h"
#include "managers/TextureManager.h"

typedef struct {
    const char name[32];
    BlockId id;
} NameIdPair;

static NameIdPair pairs[] = {
    {"air", BLOCK_AIR},
    {"grass", BLOCK_GRASS},
    {"sand", BLOCK_SAND},
    {"rock", BLOCK_ROCK},
    {"wood", BLOCK_WOOD},
    {"dirt", BLOCK_DIRT},
    {"water", BLOCK_WATER}
};

BlockData g_blockData[BLOCK_NUM_BLOCK_TYPES] = {};
static unsigned int blocksAtlas = 0;
static unsigned int blocksAtlasTextureNum = 0;

BlockId get_block_id(const char* name) {
    for (size_t i = 0; i < sizeof(pairs)/sizeof(NameIdPair); i++) {
        if (strcmp(name, pairs[i].name) == 0) {
            return pairs[i].id;
        }
    }
    return BLOCK_INVALID_ID;
}

void initialize_block_data() {
    json_object* root = json_object_from_file("assets/blocks.json");
    if (!root) {
        fprintf(stderr, "Could not load assets/blocks.json\n");
        return;
    }

    json_object *blocks = json_object_object_get(root, "blocks");
    size_t num_blocks = json_object_array_length(blocks);
    static_assert(sizeof(pairs)/sizeof(NameIdPair) == BLOCK_NUM_BLOCK_TYPES && "HEY!!! You forgot to update the name to block id map!!!");
    assert(num_blocks == BLOCK_NUM_BLOCK_TYPES && "OI LAD!!! You forgot to update the block id enum or blocks.json!!");

    tm_begin_atlas();
    for (size_t i = 0; i < num_blocks; i++) {
        json_object *block = json_object_array_get_idx(blocks, i);
        json_object *name = json_object_object_get(block, "name");
        json_object* internal_name = json_object_object_get(block, "internal_name");
        BlockId id = get_block_id(json_object_get_string(internal_name));
        if (id == BLOCK_INVALID_ID) {
            fprintf(stderr, "Block id for %s not found\n", json_object_get_string(internal_name));
            continue;
        }

        strncpy(g_blockData[id].name, json_object_get_string(name), MaxNameLen);
        json_object* textureArray = json_object_object_get(block, "textures");

        for (int face = 0; face < json_object_array_length(textureArray); face++) {
            json_object* textureFile = json_object_array_get_idx(textureArray, face);
            unsigned int atlasTextureIndex = tm_add_texture_to_atlas(json_object_get_string(textureFile));
            g_blockData[id].sideTextures[face] = atlasTextureIndex;
            if (atlasTextureIndex >= blocksAtlasTextureNum)
                blocksAtlasTextureNum = atlasTextureIndex + 1; // the last atlas index is valid so the size is that + 1
        }
    }
    blocksAtlas = tm_end_atlas();

    json_object_put(root);
}

void VoxelEngine_init() {
    initialize_block_data();
}

unsigned int VoxelEngine_get_atlas_id() {
    return blocksAtlas;
}

unsigned int VoxelEngine_get_atlas_num_textures() {
    return blocksAtlasTextureNum;
}
