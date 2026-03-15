#pragma once

namespace g
{
    struct game;
    
    struct game_clock
    {
        double fixed_delta{1.0/60.0};
        double accum{0.0};
        double previous;
        
        game_clock();

        float update_game(game& g);
    };
}