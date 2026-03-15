#pragma once

#include <string>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <array>
#include <variant>

#include <glaze/glaze.hpp>

namespace gamelib
{
    enum class objective_type : std::uint8_t
    {
        interact = 0,
        collect = 1,
        location = 2,
        item = 3,
        deposit = 4
    };

    enum class objective_requirement_type : std::uint8_t
    {
        location = 0,
        held_item = 1,
        wait = 2,
        wait_at = 3
    };

    enum class quest_drop_distribution_type : std::uint8_t
    {
        one_of = 0,
        all_of = 1
    };

    struct qobj_req_location
    {
        objective_requirement_type type{objective_requirement_type::location};
        std::string location;
    };

    struct qobj_req_held_item
    {
        objective_requirement_type type{objective_requirement_type::held_item};
        std::string item_type;
    };

    struct qobj_req_wait
    {
        objective_requirement_type type{objective_requirement_type::wait};
        float duration;
    };

    struct qobj_req_wait_at
    {
        objective_requirement_type type{objective_requirement_type::wait_at};
        float duration;
        std::string location;
    };

    using quest_objective_requirement = std::variant<
        qobj_req_location,
        qobj_req_held_item,
        qobj_req_wait,
        qobj_req_wait_at
    >;

    struct qobj_interact
    {
        objective_type type{objective_type::interact};
        std::string target;
        std::string description;
        std::vector<quest_objective_requirement> requirements;
    };

    struct qobj_collect
    {
        objective_type type{objective_type::collect};
        std::string target;
        std::string description;
        std::vector<quest_objective_requirement> requirements;
    };

    struct qobj_location
    {
        objective_type type{objective_type::location};
        std::string target;
        std::string description;
        std::vector<quest_objective_requirement> requirements;
    };

    struct qobj_item
    {
        objective_type type{objective_type::item};
        std::string target;
        std::string description;
        std::vector<quest_objective_requirement> requirements;
    };

    struct qobj_deposit
    {
        objective_type type{objective_type::deposit};
        std::string target;
        std::string description;
        std::vector<quest_objective_requirement> requirements;
    };

    using quest_objective = std::variant<
        qobj_interact,
        qobj_collect,
        qobj_location,
        qobj_item,
        qobj_deposit
    >;

    struct qdrop_dist_one_of
    {
        struct item
        {
            std::string type;
            std::string id;
            std::uint32_t probability;
        };

        quest_drop_distribution_type dist_type{quest_drop_distribution_type::one_of};
        std::vector<item> items;
    };

    struct qdrop_dist_all_of
    {
        struct item
        {
            std::string type;
            std::string id;
        };

        quest_drop_distribution_type dist_type{quest_drop_distribution_type::all_of};
        std::vector<item> items;
    };

    using quest_drops = std::variant<qdrop_dist_one_of, qdrop_dist_all_of>;

    struct quest
    {
        std::string id;
        std::string name;
        std::vector<quest_objective> objectives;
        std::vector<quest_drops> drops;
        std::string next;
    };

    struct quests_file
    {
        std::vector<quest> quests;
    };
}

// --- Enum metadata ---

template <>
struct glz::meta<gamelib::objective_type>
{
    using enum gamelib::objective_type;
    static constexpr auto value = enumerate(interact, collect, location, item, deposit);
};

template <>
struct glz::meta<gamelib::objective_requirement_type>
{
    using enum gamelib::objective_requirement_type;
    static constexpr auto value = enumerate(location, held_item, wait, wait_at);
};

template <>
struct glz::meta<gamelib::quest_drop_distribution_type>
{
    using enum gamelib::quest_drop_distribution_type;
    static constexpr auto value = enumerate(one_of, all_of);
};

// --- Objective requirement structs ---

template <>
struct glz::meta<gamelib::qobj_req_location>
{
    using T = gamelib::qobj_req_location;
    static constexpr auto value = object(
        "type", &T::type,
        "location", &T::location
    );
};

template <>
struct glz::meta<gamelib::qobj_req_held_item>
{
    using T = gamelib::qobj_req_held_item;
    static constexpr auto value = object(
        "type", &T::type,
        "item", &T::item_type
    );
};

template <>
struct glz::meta<gamelib::qobj_req_wait>
{
    using T = gamelib::qobj_req_wait;
    static constexpr auto value = object(
        "type", &T::type,
        "duration", &T::duration
    );
};

template <>
struct glz::meta<gamelib::qobj_req_wait_at>
{
    using T = gamelib::qobj_req_wait_at;
    static constexpr auto value = object(
        "type", &T::type,
        "duration", &T::duration,
        "location", &T::location
    );
};

// --- Tagged variant: quest_objective_requirement (discriminated by "type") ---

template <>
struct glz::meta<gamelib::quest_objective_requirement>
{
    static constexpr std::string_view tag = "type";
    static constexpr auto ids = std::array{"location", "held_item", "wait", "wait_at"};
};

// --- Objective structs ---

template <>
struct glz::meta<gamelib::qobj_interact>
{
    using T = gamelib::qobj_interact;
    static constexpr auto value = object(
        "type", &T::type,
        "target", &T::target,
        "description", &T::description,
        "requires", &T::requirements
    );
};

template <>
struct glz::meta<gamelib::qobj_collect>
{
    using T = gamelib::qobj_collect;
    static constexpr auto value = object(
        "type", &T::type,
        "target", &T::target,
        "description", &T::description,
        "requires", &T::requirements
    );
};

template <>
struct glz::meta<gamelib::qobj_location>
{
    using T = gamelib::qobj_location;
    static constexpr auto value = object(
        "type", &T::type,
        "target", &T::target,
        "description", &T::description,
        "requires", &T::requirements
    );
};

template <>
struct glz::meta<gamelib::qobj_item>
{
    using T = gamelib::qobj_item;
    static constexpr auto value = object(
        "type", &T::type,
        "target", &T::target,
        "description", &T::description,
        "requires", &T::requirements
    );
};

template <>
struct glz::meta<gamelib::qobj_deposit>
{
    using T = gamelib::qobj_deposit;
    static constexpr auto value = object(
        "type", &T::type,
        "target", &T::target,
        "description", &T::description,
        "requires", &T::requirements
    );
};

// --- Tagged variant: quest_objective (discriminated by "type") ---

template <>
struct glz::meta<gamelib::quest_objective>
{
    static constexpr std::string_view tag = "type";
    static constexpr auto ids = std::array{"interact", "collect", "location", "item", "deposit"};
};

// --- Drop distribution structs ---

template <>
struct glz::meta<gamelib::qdrop_dist_one_of::item>
{
    using T = gamelib::qdrop_dist_one_of::item;
    static constexpr auto value = object(
        "type", &T::type,
        "id", &T::id,
        "probability", &T::probability
    );
};

template <>
struct glz::meta<gamelib::qdrop_dist_one_of>
{
    using T = gamelib::qdrop_dist_one_of;
    static constexpr auto value = object(
        "dist_type", &T::dist_type,
        "items", &T::items
    );
};

template <>
struct glz::meta<gamelib::qdrop_dist_all_of::item>
{
    using T = gamelib::qdrop_dist_all_of::item;
    static constexpr auto value = object(
        "type", &T::type,
        "id", &T::id
    );
};

template <>
struct glz::meta<gamelib::qdrop_dist_all_of>
{
    using T = gamelib::qdrop_dist_all_of;
    static constexpr auto value = object(
        "dist_type", &T::dist_type,
        "items", &T::items
    );
};

// --- Tagged variant: quest_drops (discriminated by "dist_type") ---

template <>
struct glz::meta<gamelib::quest_drops>
{
    static constexpr std::string_view tag = "dist_type";
    static constexpr auto ids = std::array{"one_of", "all_of"};
};

// --- Quest and top-level ---

template <>
struct glz::meta<gamelib::quest>
{
    using T = gamelib::quest;
    static constexpr auto value = object(
        "id", &T::id,
        "name", &T::name,
        "objectives", &T::objectives,
        "drops", &T::drops,
        "next", &T::next
    );
};
