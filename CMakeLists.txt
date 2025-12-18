cmake_minimum_required(VERSION 3.29)
project(CVoxelEngine C)

set(CMAKE_C_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

include_directories(
        ${PROJECT_SOURCE_DIR}
        include
        ${PROJECT_SOURCE_DIR}/libs
        /usr/include/freetype2
)

add_executable(CVoxelEngine main.c
        libs/glad.c
        include/Settings.h
        src/managers/InputManager.c
        include/managers/InputManager.h
        src/ChunkManager.c
        include/ChunkManager.h
        src/Chunk.c
        include/Chunk.h
        include/VoxelEngine/Block.h
        src/managers/TextureManager.c
        include/managers/TextureManager.h
        include/VoxelEngine/BlockId.h
        src/Shader.c
        include/Shader.h
        src/Player.c
        include/Player.h
        src/Skybox.c
        include/Skybox.h
        include/AABB.h
        src/Collisions.c
        include/Collisions.h
        include/FaceOrientation.h
        src/Block.c
        src/Rigidbody.c
        include/Rigidbody.h
        src/PerlinNoise.c
        include/PerlinNoise.h
        src/managers/ShaderManager.c
        include/managers/ShaderManager.h
        include/ShaderType.h
        libs/thpool.c
        libs/thpool.h
        include/ui/UIHotbar.h
        include/ui/UISprite.h
        include/ui/UIManager.h
        libs/hashmap.h
        libs/hashmap.c
        src/ui/UIHotbar.c
        src/ui/UISprite.c
        src/ui/UIManager.c
        src/managers/WindowManager.c
        include/managers/WindowManager.h
        src/Inventory.c
        include/Inventory.h
        src/ui/UIInventory.c
        include/ui/UIInventory.h
        include/ui/BlockStack.h
        src/ui/UIText.c
        include/ui/UIText.h
        include/ui/CharGlyph.h
        src/managers/GlyphManager.c
        src/managers/GlyphManager.c
        include/managers/GlyphManager.h
        src/ui/UIStack.c
        include/ui/UIStack.h
        include/Engine/Time.h
        src/Engine/Engine.c
        include/Engine/Engine.h
        src/Engine/Settings.c
        src/ui/UIItem.c
        src/ui/UIItem.h
        include/CommonVertices.h
        include/VoxelEngine/VoxelEngine.c
        include/VoxelEngine/VoxelEngine.h
        src/Engine/Utils.c
        include/Engine/Utils.h
        libs/CVector/Vector.c
        include/Structure.h
        src/Structure.c
        include/VoxelEngine/BlockProperty.h
        include/Geometry.h
)

target_link_libraries(CVoxelEngine -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -lpthread -lfreetype -I/usr/include/freetype2 -I/usr/include/libpng16 -ljson-c)