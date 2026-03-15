#include "game/window.h"
#include <raylib.h>

namespace g
{
    bool window::_window_opened = false;

    void window::cleanup()
    {
        if (_owner && IsWindowReady())
        {
            CloseWindow();
            _owner = false;
        }
    }

    window::window(int w, int h, int target_fps, std::string title) : 
        _width(w), _height(h), _target_fps(target_fps), 
        _title(std::move(title)), _owner(!_window_opened)
    {
        if (_window_opened)
        {
            throw window_already_exists();
        }

        InitWindow(w, h, _title.c_str());
        SetTargetFPS(target_fps);
        _window_opened = true;
        _owner = true;
    }

    window::~window()
    {
        cleanup();
    }

    window::window(window&& other) noexcept : 
        _width(std::exchange(other._width, 0)),
        _height(std::exchange(other._height, 0)),
        _target_fps(std::exchange(other._target_fps, 0)),
        _title(std::move(other._title)),
        _owner(std::exchange(other._owner, false))
    {

    }

    window& window::operator=(window&& other) noexcept
    {
        if (this == &other) return *this;
        cleanup();

        _width = std::exchange(other._width, 0);
        _height = std::exchange(other._height, 0);
        _title = std::move(other._title);
        _owner = std::exchange(other._owner, false);
        return *this;
    }

    const std::string& window::title() const
    {
        return _title;
    }

    std::pair<int, int> window::get_dimensions() const
    {
        return {_width, _height};
    }

    void window::set_width(int w)
    {
        if (!_owner) return;

        SetWindowSize(w, _height);
        _width = w;
    }

    void window::set_height(int h)
    {
        if (!_owner) return;
        SetWindowSize(_width, h);
        _height = h;
    }

    void window::set_dimensions(int w, int h)
    {
        if (!_owner) return;
        SetWindowSize(w, h);
        _width = w;
        _height = h;
    }

    void window::set_title(std::string title)
    {
        if (!_owner) return;
        SetWindowTitle(title.c_str());
        _title = std::move(title);
    }
}