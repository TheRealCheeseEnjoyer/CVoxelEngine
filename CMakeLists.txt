cmake_minimum_required(VERSION 3.29)
project(CVoxelEngine C)

set(CMAKE_C_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_executable(CVoxelEngine main.c
        src/glad.c
        src/Settings.c
        include/Settings.h
        src/InputManager.c
        include/InputManager.h
        include/Vector.h
        src/Vector.c
        src/World.c
        include/World.h
        src/Chunk.c
        include/Chunk.h
        include/Block.h
        src/TextureManager.c
        include/TextureManager.h
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
        src/PerlinNoise.h
)

target_link_libraries(CVoxelEngine -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lm)