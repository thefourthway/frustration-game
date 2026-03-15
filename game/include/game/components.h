#pragma once

#include <game/vec.hpp>
#include <raylib.h>
#include <raymath.h>

namespace g
{
    struct transform
    {
        vec3 position{};
    };

    struct box_renderer
    {
        vec3 size{1, 1, 1};
        Color color{GRAY};
    };

    struct player
    {
        float yaw{};
        float pitch{};
    };

    struct camera_state
    {
        Camera3D camera{};
    };

    struct inventory
    {
        bool holds_item{false};
    };

    struct is_trash {};
    struct is_bottle {};
}
