#pragma once

#include <raylib.h>
#include <utility>

namespace g
{
    struct draw_raii
    {
        inline draw_raii()
        {
            BeginDrawing();
        }

        draw_raii(const draw_raii&) = delete;

        inline ~draw_raii()
        {
            EndDrawing();
        }
    };

    struct managed_render_texture
    {
    private:
        bool _initialized{true};

        void cleanup()
        {
            if (_initialized)
            {
                _initialized = false;
                UnloadRenderTexture(tex);
            }
        }
    public:
        RenderTexture2D tex;

        managed_render_texture(int w, int h, TextureFilter texflags = TEXTURE_FILTER_POINT) : tex(LoadRenderTexture(w, h))
        {
            SetTextureFilter(tex.texture, texflags);
        }

        managed_render_texture(const managed_render_texture&) = delete;
        managed_render_texture(managed_render_texture&& other) noexcept : 
            _initialized(other._initialized),
            tex(std::exchange(other.tex, RenderTexture2D{}))
        {

        }

        managed_render_texture& operator=(const managed_render_texture&) = delete;
        managed_render_texture& operator=(managed_render_texture&& other) noexcept
        {
            if (this == &other) return *this;
            cleanup();
            _initialized = std::exchange(other._initialized, false);
            tex = std::exchange(other.tex, RenderTexture2D{});
            return *this;
        }

        ~managed_render_texture()
        {
            cleanup();
        }
    };

    struct render_target_raii
    {
        render_target_raii(RenderTexture2D& tex)
        {
            BeginTextureMode(tex);
        }

        render_target_raii(const render_target_raii&) = delete;
        render_target_raii(render_target_raii&&) = delete;
        render_target_raii& operator=(const render_target_raii&) = delete;
        render_target_raii& operator=(render_target_raii&&) = delete;

        ~render_target_raii()
        {
            EndTextureMode();
        }
    };
}