#pragma once

#include <string>
#include <string_view>
#include <stdexcept>
#include <raylib.h>

namespace g
{
    class window_already_exists : public std::runtime_error
    {
    public:
        window_already_exists() : std::runtime_error("Window has already been initialized")
        {

        }
    };

    struct window
    {
    private:
        int _width{800};
        int _height{450};
        int _target_fps{60};
        std::string _title{"game window"};
        bool _owner{true};

        static bool _window_opened;

        void cleanup();
    public:
        window(int w = 800, int h = 450, int target_fps = 60, std::string title = "game window");
        ~window();

        window(const window&) = delete;
        window(window&&) noexcept;

        window& operator=(const window&) = delete;
        window& operator=(window&&) noexcept;

        [[nodiscard]] inline int width() const { return _width; };
        [[nodiscard]] inline int height() const { return _height; };
        [[nodiscard]] inline int target_fps() const { return _target_fps; };
        [[nodiscard]] const std::string& title() const;
        [[nodiscard]] std::pair<int, int> get_dimensions() const;

        void set_width(int w);
        void set_height(int h);
        void set_dimensions(int w, int h);
        void set_title(std::string title);

        [[nodiscard]] inline bool is_focused()
        {
            return IsWindowFocused();
        }

        [[nodiscard]] inline bool should_close()
        {
            return WindowShouldClose();
        };
    };
}