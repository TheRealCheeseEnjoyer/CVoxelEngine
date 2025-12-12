#include "Engine/Utils.h"

#include <stdio.h>
#include <stdlib.h>

unsigned int utils_read_file(const char* filePath, const char** output)  {
    FILE* file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Error opening shader file %s\n", filePath);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    const unsigned int length = ftell(file);
    rewind(file);

    *output = malloc(length * sizeof(char));
    if (!*output) {
        fprintf(stderr, "Error allocating memory. File %s not read.", filePath);
        return 0;
    }
    fread(*output, length, sizeof(char), file);
    fclose(file);
    return length;
}
