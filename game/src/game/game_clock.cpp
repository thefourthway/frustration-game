#include "game/game_clock.h"
#include "game/game.h"

namespace g
{
    game_clock::game_clock() : previous(GetTime())
    {

    }

    float game_clock::update_game(game& g)
    {
        double currentTime = GetTime();
        double frameTime = currentTime - previous;
        previous = currentTime;

        accum += frameTime;

        while (accum >= fixed_delta) {
            g.update_physics(static_cast<float>(fixed_delta));
            accum -= fixed_delta;
        }

        g.update_game(static_cast<float>(frameTime));

        return static_cast<float>(frameTime);
    }
}