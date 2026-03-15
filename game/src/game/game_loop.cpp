#include "game/game_loop.h"
#include "game/draw_raii.hpp"
#include "game/game_clock.h"
#include "game/window.h"
#include "game/game.h"

namespace g
{
    void run_game_loop(window& w)
    {
        game g(w);
        game_clock gc{};

        while (!w.should_close())
        {
            g.input.update();

            float ft = gc.update_game(g);

            if (!w.is_focused())
            {
                draw_raii dr;
                continue;
            };

            draw_raii dr;
            ClearBackground(WHITE);
            g.draw(ft);
        }
    }
}