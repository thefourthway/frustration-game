#include <raylib.h>
#include <game/window.h>
#include <game/game_loop.h>

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 448;

    g::window window(screenWidth, screenHeight);

    g::run_game_loop(window);

    return 0;
}