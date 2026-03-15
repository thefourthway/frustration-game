#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <glaze/glaze.hpp>

namespace gamelib
{
    enum class requirement_id : std::uint8_t
    {
        object_exists = 0
    };

    struct ev_req_object_exists
    {
        requirement_id id;
        std::string object_id;
    };

    using event_requirement = std::variant<ev_req_object_exists>;

    struct event_times
    {
        std::array<std::uint32_t, 2> low;
        std::array<std::uint32_t, 2> high;
    };

    struct event
    {
        std::string id;
        std::string message;

        std::vector<event_requirement> requirements;
        event_times times;
        std::string triggers;
    };

    struct events_file
    {
        std::vector<event> events;
    };
}

template <>
struct glz::meta<gamelib::requirement_id>
{
    using enum gamelib::requirement_id;
    static constexpr auto value = enumerate(object_exists);
};

template <>
struct glz::meta<gamelib::event_times>
{
    using T = gamelib::event_times;
    static constexpr auto value = object(
        "low", &T::low,
        "high", &T::high
    );
};

template <>
struct glz::meta<gamelib::ev_req_object_exists>
{
    using T = gamelib::ev_req_object_exists;
    static constexpr auto value = glz::array(&T::id, &T::object_id);
};

template <>
struct glz::meta<gamelib::event>
{
    using T = gamelib::event;
    static constexpr auto value = object(
        "id", &T::id,
        "message", &T::message,
        "requires", &T::requirements,
        "times", &T::times,
        "triggers", &T::triggers
    );
};
