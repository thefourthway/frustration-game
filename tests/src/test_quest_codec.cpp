#include <catch2/catch_test_macros.hpp>
#include <gamelib/quests.h>
#include <string>

TEST_CASE("quest deserialization", "[quests][codec]")
{
    SECTION("quest with interact objective and all_of drops")
    {
        std::string json = R"({
            "id": "q1",
            "name": "Test Quest",
            "objectives": [
                {
                    "type": "interact",
                    "target": "npc_bob",
                    "description": "Talk to Bob"
                }
            ],
            "drops": [
                {
                    "dist_type": "all_of",
                    "items": [{"type": "weapon", "id": "sword"}]
                }
            ],
            "next": "q2"
        })";

        gamelib::quest q{};
        auto err = glz::read_json(q, json);
        REQUIRE(!err);
        CHECK(q.id == "q1");
        CHECK(q.name == "Test Quest");
        REQUIRE(q.objectives.size() == 1);

        auto* obj = std::get_if<gamelib::qobj_interact>(&q.objectives[0]);
        REQUIRE(obj != nullptr);
        CHECK(obj->target == "npc_bob");
        CHECK(obj->description == "Talk to Bob");
        CHECK(obj->requirements.empty());

        REQUIRE(q.drops.size() == 1);
        auto* drop = std::get_if<gamelib::qdrop_dist_all_of>(&q.drops[0]);
        REQUIRE(drop != nullptr);
        REQUIRE(drop->items.size() == 1);
        CHECK(drop->items[0].type == "weapon");
        CHECK(drop->items[0].id == "sword");

        CHECK(q.next == "q2");
    }

    SECTION("quest with objective requirements")
    {
        std::string json = R"({
            "id": "q2",
            "name": "Fetch Quest",
            "objectives": [
                {
                    "type": "collect",
                    "target": "gem",
                    "description": "Get the gem",
                    "requires": [
                        {"type": "location", "location": "cave"},
                        {"type": "held_item", "item": "pickaxe"}
                    ]
                }
            ],
            "drops": [],
            "next": ""
        })";

        gamelib::quest q{};
        auto err = glz::read_json(q, json);
        REQUIRE(!err);
        REQUIRE(q.objectives.size() == 1);

        auto* obj = std::get_if<gamelib::qobj_collect>(&q.objectives[0]);
        REQUIRE(obj != nullptr);
        CHECK(obj->target == "gem");
        REQUIRE(obj->requirements.size() == 2);

        auto* loc = std::get_if<gamelib::qobj_req_location>(&obj->requirements[0]);
        REQUIRE(loc != nullptr);
        CHECK(loc->location == "cave");

        auto* held = std::get_if<gamelib::qobj_req_held_item>(&obj->requirements[1]);
        REQUIRE(held != nullptr);
        CHECK(held->item_type == "pickaxe");
    }

    SECTION("quest with one_of drops")
    {
        std::string json = R"({
            "id": "q3",
            "name": "Loot Quest",
            "objectives": [],
            "drops": [
                {
                    "dist_type": "one_of",
                    "items": [
                        {"type": "potion", "id": "hp_pot", "probability": 80},
                        {"type": "potion", "id": "mp_pot", "probability": 20}
                    ]
                }
            ],
            "next": ""
        })";

        gamelib::quest q{};
        auto err = glz::read_json(q, json);
        REQUIRE(!err);
        REQUIRE(q.drops.size() == 1);

        auto* drop = std::get_if<gamelib::qdrop_dist_one_of>(&q.drops[0]);
        REQUIRE(drop != nullptr);
        REQUIRE(drop->items.size() == 2);
        CHECK(drop->items[0].type == "potion");
        CHECK(drop->items[0].id == "hp_pot");
        CHECK(drop->items[0].probability == 80);
        CHECK(drop->items[1].id == "mp_pot");
        CHECK(drop->items[1].probability == 20);
    }

    SECTION("quest with wait_at requirement")
    {
        std::string json = R"({
            "id": "q4",
            "name": "Wait Quest",
            "objectives": [
                {
                    "type": "interact",
                    "target": "bench",
                    "description": "Sit and wait",
                    "requires": [
                        {"type": "wait_at", "location": "bench_spot", "duration": 5.0}
                    ]
                }
            ],
            "drops": [],
            "next": ""
        })";

        gamelib::quest q{};
        auto err = glz::read_json(q, json);
        REQUIRE(!err);

        auto* obj = std::get_if<gamelib::qobj_interact>(&q.objectives[0]);
        REQUIRE(obj != nullptr);
        REQUIRE(obj->requirements.size() == 1);

        auto* wa = std::get_if<gamelib::qobj_req_wait_at>(&obj->requirements[0]);
        REQUIRE(wa != nullptr);
        CHECK(wa->duration == 5.0f);
        CHECK(wa->location == "bench_spot");
    }
}

TEST_CASE("quest serialization", "[quests][codec]")
{
    SECTION("round-trip quest with location objective")
    {
        gamelib::quest q;
        q.id = "rt_q";
        q.name = "Roundtrip";
        q.objectives.push_back(gamelib::qobj_location{
            gamelib::objective_type::location, "park", "Go to the park", {}
        });
        q.drops.push_back(gamelib::qdrop_dist_all_of{
            gamelib::quest_drop_distribution_type::all_of,
            {{"coin", "gold_coin"}}
        });
        q.next = "rt_q2";

        std::string json;
        auto write_err = glz::write_json(q, json);
        REQUIRE(!write_err);

        gamelib::quest q2{};
        auto read_err = glz::read_json(q2, json);
        REQUIRE(!read_err);
        CHECK(q2.id == "rt_q");
        CHECK(q2.name == "Roundtrip");
        REQUIRE(q2.objectives.size() == 1);

        auto* obj = std::get_if<gamelib::qobj_location>(&q2.objectives[0]);
        REQUIRE(obj != nullptr);
        CHECK(obj->target == "park");

        REQUIRE(q2.drops.size() == 1);
        auto* drop = std::get_if<gamelib::qdrop_dist_all_of>(&q2.drops[0]);
        REQUIRE(drop != nullptr);
        CHECK(drop->items[0].type == "coin");

        CHECK(q2.next == "rt_q2");
    }

    SECTION("round-trip quest with nested requirements")
    {
        gamelib::quest q;
        q.id = "nested_q";
        q.name = "Nested";
        gamelib::qobj_item obj;
        obj.target = "key";
        obj.description = "Use the key";
        obj.requirements.push_back(gamelib::qobj_req_held_item{
            gamelib::objective_requirement_type::held_item, "key"
        });
        obj.requirements.push_back(gamelib::qobj_req_location{
            gamelib::objective_requirement_type::location, "door"
        });
        q.objectives.push_back(obj);
        q.next = "";

        std::string json;
        auto write_err = glz::write_json(q, json);
        REQUIRE(!write_err);

        gamelib::quest q2{};
        auto read_err = glz::read_json(q2, json);
        REQUIRE(!read_err);

        auto* o = std::get_if<gamelib::qobj_item>(&q2.objectives[0]);
        REQUIRE(o != nullptr);
        REQUIRE(o->requirements.size() == 2);

        auto* held = std::get_if<gamelib::qobj_req_held_item>(&o->requirements[0]);
        REQUIRE(held != nullptr);
        CHECK(held->item_type == "key");

        auto* loc = std::get_if<gamelib::qobj_req_location>(&o->requirements[1]);
        REQUIRE(loc != nullptr);
        CHECK(loc->location == "door");
    }
}
