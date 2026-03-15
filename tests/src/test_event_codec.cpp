#include <catch2/catch_test_macros.hpp>
#include <gamelib/events.h>
#include <string>

TEST_CASE("event deserialization", "[events][codec]")
{
    SECTION("event with object_exists requirement")
    {
        std::string json = R"({
            "id": "test_event",
            "message": "Something happened",
            "requires": [["object_exists", "lamp"]],
            "times": {"low": [8, 0], "high": [20, 30]},
            "triggers": "next_event"
        })";

        gamelib::event ev{};
        auto err = glz::read_json(ev, json);
        REQUIRE(!err);
        CHECK(ev.id == "test_event");
        CHECK(ev.message == "Something happened");
        REQUIRE(ev.requirements.size() == 1);

        auto* req = std::get_if<gamelib::ev_req_object_exists>(&ev.requirements[0]);
        REQUIRE(req != nullptr);
        CHECK(req->id == gamelib::requirement_id::object_exists);
        CHECK(req->object_id == "lamp");

        CHECK(ev.times.low[0] == 8);
        CHECK(ev.times.low[1] == 0);
        CHECK(ev.times.high[0] == 20);
        CHECK(ev.times.high[1] == 30);
        CHECK(ev.triggers == "next_event");
    }

    SECTION("event with empty requirements")
    {
        std::string json = R"({
            "id": "simple",
            "message": "No reqs",
            "requires": [],
            "times": {"low": [0, 0], "high": [23, 59]},
            "triggers": ""
        })";

        gamelib::event ev{};
        auto err = glz::read_json(ev, json);
        REQUIRE(!err);
        CHECK(ev.id == "simple");
        CHECK(ev.requirements.empty());
    }
}

TEST_CASE("event serialization", "[events][codec]")
{
    SECTION("round-trip event")
    {
        gamelib::event ev;
        ev.id = "rt_event";
        ev.message = "Round trip";
        ev.requirements.push_back(gamelib::ev_req_object_exists{
            gamelib::requirement_id::object_exists, "chair"
        });
        ev.times.low = {6, 30};
        ev.times.high = {18, 0};
        ev.triggers = "trigger_x";

        std::string json;
        auto write_err = glz::write_json(ev, json);
        REQUIRE(!write_err);

        gamelib::event ev2{};
        auto read_err = glz::read_json(ev2, json);
        REQUIRE(!read_err);
        CHECK(ev2.id == "rt_event");
        CHECK(ev2.message == "Round trip");
        REQUIRE(ev2.requirements.size() == 1);
        auto* req = std::get_if<gamelib::ev_req_object_exists>(&ev2.requirements[0]);
        REQUIRE(req != nullptr);
        CHECK(req->object_id == "chair");
        CHECK(ev2.times.low[0] == 6);
        CHECK(ev2.times.high[1] == 0);
        CHECK(ev2.triggers == "trigger_x");
    }
}
