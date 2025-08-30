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
        include
        ${PROJECT_SOURCE_DIR}/libs
)

add_executable(CVoxelEngine main.c
        libs/glad.c
        include/Settings.h
        src/managers/InputManager.c
        include/managers/InputManager.h
        libs/Vector.h
        libs/Vector.c
        src/World.c
        include/World.h
        src/Chunk.c
        include/Chunk.h
        include/Block.h
        src/managers/TextureManager.c
        include/managers/TextureManager.h
        include/BlockType.h
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
        include/Constants.h
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
        src/managers/SettingsManager.c
        include/managers/SettingsManager.h
)

target_link_libraries(CVoxelEngine -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm -lpthread)