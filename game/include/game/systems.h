#pragma once

#include <gamelib/ecs/system_scheduler.hpp>

namespace g
{
    struct physics_system : gamelib::isystem
    {
        [[nodiscard]] gamelib::system_desc desc() const override final;
        void update_physics(gamelib::world& w, float dt) override;
    };

    struct player_system : gamelib::isystem
    {
        [[nodiscard]] gamelib::system_desc desc() const override final;
        void update_physics(gamelib::world& w, float dt) override;
    };

    struct render_system : gamelib::isystem
    {
        [[nodiscard]] gamelib::system_desc desc() const override final;
        void draw(gamelib::world& w, float alpha) override;
    };

    struct inventory_system : gamelib::isystem
    {
        [[nodiscard]] gamelib::system_desc desc() const override final;
        void update_game(gamelib::world& w, float alpha) override final;
    };

}
