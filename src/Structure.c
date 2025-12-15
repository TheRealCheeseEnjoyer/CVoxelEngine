#include "Structure.h"

#include <stdio.h>
#include <string.h>
#include <json-c/json_util.h>

#include "VoxelEngine/VoxelEngine.h"

StructureData structures[STRUCTURE_NUM] = {};

typedef struct {
    const char name[32];
    StructureId id;
} NameIdPair;

NameIdPair pairs[] = {
    {"tree", STRUCTURE_TREE}
};

StructureId get_structure_id(const char *name) {
    for (size_t i = 0; i < sizeof(pairs) / sizeof(NameIdPair); i++) {
        if (strcmp(name, pairs[i].name) == 0) {
            return pairs[i].id;
        }
    }
    return STRUCTURE_INVALID_ID;
}

void structure_init() {
    json_object *root = json_object_from_file("assets/structures.json");
    json_object *structureArray = json_object_object_get(root, "structures");
    for (size_t i = 0; i < json_object_array_length(structureArray); i++) {
        json_object *structure = json_object_array_get_idx(structureArray, i);
        json_object *structureName = json_object_object_get(structure, "name");

        StructureId structureId = get_structure_id(json_object_get_string(structureName));
        if (structureId == STRUCTURE_INVALID_ID) {
            fprintf(stderr, "Structure \"%s\" not found\n", json_object_get_string(structureName));
            continue;
        }

        json_object *blockArray = json_object_object_get(structure, "blocks");
        size_t numBlocks = json_object_array_length(blockArray);
        structures[structureId].numBlocks = numBlocks;
        structures[structureId].blocks = malloc(sizeof(BlockId) * numBlocks);
        structures[structureId].positions = malloc(sizeof(ivec3) * numBlocks);
        for (size_t j = 0; j < numBlocks; j++) {
            json_object *block = json_object_array_get_idx(blockArray, j);
            json_object* position = json_object_object_get(block, "position");
            for (int i = 0; i < 3; i++) {
                json_object* coord = json_object_array_get_idx(position, i);
                structures[structureId].positions[j][i] = json_object_get_int(coord);
            }

            json_object* blockName = json_object_object_get(block, "internal_block_name");
            structures[i].blocks[j] = VoxelEngine_get_block_id(json_object_get_string(blockName));
        }
    }

    json_object_put(root);
}

StructureData structure_get_data(StructureId id) {
    return structures[id];
}
