project(game VERSION 0.0.1 LANGUAGES CXX)

set(GAME_INC
    include/game/components.h
    include/game/draw_raii.hpp

    include/game/game_clock.h
    include/game/game_loop.h
    include/game/game.h
    include/game/items.h
    include/game/ivec.hpp
    include/game/jolt_setup.h

    include/game/seed_rng.h
    include/game/systems.h
    include/game/vec.hpp
    include/game/window.h
)

set(GAME_SRC
    src/game/game_clock.cpp
    src/game/game_loop.cpp
    src/game/game.cpp
    src/game/seed_rng.cpp
    src/game/systems.cpp
    src/game/window.cpp
)

add_executable(frustration ${GAME_INC} ${GAME_SRC} src/main.cpp)
target_include_directories(frustration PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    frustration
    PRIVATE
    raylib
    Jolt
    gamelib
    glaze::glaze
)
