#pragma once

#include <game/window.h>
#include <game/vec.hpp>
#include <game/components.h>
#include <game/systems.h>
#include <gamelib/ecs/world.hpp>
#include <gamelib/ecs/system_scheduler.hpp>

namespace g
{
    struct input_state
    {
        vec2 mouse_pos;

        void update();
    };

    struct game
    {
        window& window;
        input_state input{};

        gamelib::system_scheduler scheduler{};
        gamelib::world world{};

        game(::g::window& w);

        void update_physics(float dt);
        void update_game(float dt);
        void draw(float alpha);
    };
}
