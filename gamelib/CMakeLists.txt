project(gamelib)

set(
    LIB_INC
    include/gamelib/ecs/component_pool.hpp
    include/gamelib/ecs/ecs.hpp
    include/gamelib/ecs/entity.hpp
    include/gamelib/ecs/world.hpp
)

set(
    LIB_SRC
)

add_library(gamelib INTERFACE)
target_sources(gamelib INTERFACE ${LIB_INC})
target_include_directories(gamelib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
    gamelib
    INTERFACE
    absl::flat_hash_map
    absl::flat_hash_set
)
