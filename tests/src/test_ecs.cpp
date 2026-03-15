#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <gamelib/ecs/component_pool.hpp>
#include <gamelib/ecs/entity.hpp>
#include <gamelib/ecs/system_scheduler.hpp>
#include <gamelib/ecs/world.hpp>

struct position
{
    float x, y, z;
};

struct health
{
    int value;
};

TEST_CASE("component_pool", "[ecs]")
{
    gamelib::component_pool<position> pool;

    SECTION("add and get")
    {
        pool.add(1u, position{1.0f, 2.0f, 3.0f});

        auto* p = pool.get(1u);
        REQUIRE(p != nullptr);
        CHECK(p->x == 1.0f);
        CHECK(p->y == 2.0f);
        CHECK(p->z == 3.0f);
    }

    SECTION("add duplicate is ignored")
    {
        pool.add(1u, position{1.0f, 2.0f, 3.0f});
        pool.add(1u, position{9.0f, 9.0f, 9.0f});

        auto* p = pool.get(1u);
        REQUIRE(p != nullptr);
        CHECK(p->x == 1.0f);
    }

    SECTION("get returns nullptr for missing entity")
    {
        CHECK(pool.get(42u) == nullptr);
    }

    SECTION("has")
    {
        pool.add(1u, position{0, 0, 0});

        CHECK(pool.has(1u));
        CHECK_FALSE(pool.has(2u));
    }

    SECTION("remove")
    {
        pool.add(1u, position{1, 0, 0});
        pool.add(2u, position{2, 0, 0});
        pool.add(3u, position{3, 0, 0});

        pool.remove(2u);

        CHECK_FALSE(pool.has(2u));
        CHECK(pool.get(2u) == nullptr);
        CHECK(pool.dense.size() == 2);

        // remaining entities still accessible
        REQUIRE(pool.has(1u));
        CHECK(pool.get(1u)->x == 1.0f);
        REQUIRE(pool.has(3u));
        CHECK(pool.get(3u)->x == 3.0f);
    }

    SECTION("remove last element")
    {
        pool.add(1u, position{1, 0, 0});
        pool.remove(1u);

        CHECK_FALSE(pool.has(1u));
        CHECK(pool.dense.empty());
    }

    SECTION("remove nonexistent entity is a no-op")
    {
        pool.add(1u, position{1, 0, 0});
        pool.remove(99u);

        CHECK(pool.dense.size() == 1);
    }

    SECTION("each iterates all components")
    {
        pool.add(1u, position{1, 0, 0});
        pool.add(2u, position{2, 0, 0});
        pool.add(3u, position{3, 0, 0});

        float sum = 0;
        pool.each([&](gamelib::entity_id, position& pos) {
            sum += pos.x;
        });

        CHECK(sum == 6.0f);
    }

    SECTION("each can mutate components")
    {
        pool.add(1u, position{1, 2, 3});

        pool.each([](gamelib::entity_id, position& pos) {
            pos.x *= 10;
        });

        CHECK(pool.get(1u)->x == 10.0f);
    }

    SECTION("each on empty pool")
    {
        int count = 0;
        pool.each([&](gamelib::entity_id, position&) { ++count; });

        CHECK(count == 0);
    }

    SECTION("const get")
    {
        pool.add(1u, position{5, 6, 7});

        const auto& cpool = pool;
        const position* p = cpool.get(1u);
        REQUIRE(p != nullptr);
        CHECK(p->x == 5.0f);
        CHECK(cpool.get(99u) == nullptr);
    }

    SECTION("variadic add constructs in place")
    {
        gamelib::component_pool<health> hp;
        hp.add(1u, 100);

        REQUIRE(hp.has(1u));
        CHECK(hp.get(1u)->value == 100);
    }
}

struct velocity
{
    float dx, dy, dz;
};

struct gravity
{
    float strength;
};

TEST_CASE("world", "[ecs]")
{
    gamelib::world w;

    SECTION("create_entity")
    {
        auto e1 = w.create_entity();
        auto e2 = w.create_entity();

        CHECK(e1 != e2);
        CHECK(w.is_alive(e1));
        CHECK(w.is_alive(e2));
        CHECK(w.entity_count() == 2);
    }

    SECTION("destroy_entity")
    {
        auto e = w.create_entity();
        w.destroy_entity(e);

        CHECK_FALSE(w.is_alive(e));
        CHECK(w.entity_count() == 0);
    }

    SECTION("destroy_entity removes components")
    {
        auto e = w.create_entity();
        w.add<position>(e, {1, 2, 3});
        w.destroy_entity(e);

        CHECK_FALSE(w.is_alive(e));
    }

    SECTION("destroy_entity on dead entity is a no-op")
    {
        auto e = w.create_entity();
        w.destroy_entity(e);
        w.destroy_entity(e);

        CHECK(w.entity_count() == 0);
    }

    SECTION("entity id reuse via free list")
    {
        auto e1 = w.create_entity();
        w.destroy_entity(e1);

        auto e2 = w.create_entity();
        CHECK(e2 == e1);
        CHECK(w.is_alive(e2));
    }

    SECTION("add and get component")
    {
        auto e = w.create_entity();
        w.add<position>(e, {10, 20, 30});

        auto* p = w.get<position>(e);
        REQUIRE(p != nullptr);
        CHECK(p->x == 10.0f);
        CHECK(p->y == 20.0f);
        CHECK(p->z == 30.0f);
    }

    SECTION("add component to dead entity is ignored")
    {
        auto e = w.create_entity();
        w.destroy_entity(e);
        w.add<position>(e, {1, 2, 3});

        CHECK(w.get<position>(e) == nullptr);
    }

    SECTION("has component")
    {
        auto e = w.create_entity();
        CHECK_FALSE(w.has<position>(e));

        w.add<position>(e, {0, 0, 0});
        CHECK(w.has<position>(e));
    }

    SECTION("remove component")
    {
        auto e = w.create_entity();
        w.add<position>(e, {1, 2, 3});
        w.remove<position>(e);

        CHECK_FALSE(w.has<position>(e));
        CHECK(w.get<position>(e) == nullptr);
        CHECK(w.is_alive(e));
    }

    SECTION("multiple component types on same entity")
    {
        auto e = w.create_entity();
        w.add<position>(e, {1, 2, 3});
        w.add<health>(e, {100});

        REQUIRE(w.get<position>(e) != nullptr);
        REQUIRE(w.get<health>(e) != nullptr);
        CHECK(w.get<position>(e)->x == 1.0f);
        CHECK(w.get<health>(e)->value == 100);
    }

    SECTION("each with single component")
    {
        auto e1 = w.create_entity();
        auto e2 = w.create_entity();
        w.add<position>(e1, {1, 0, 0});
        w.add<position>(e2, {2, 0, 0});

        float sum = 0;
        w.each<position>([&](gamelib::entity_id, position& p) {
            sum += p.x;
        });

        CHECK(sum == 3.0f);
    }

    SECTION("each with multiple components filters correctly")
    {
        auto e1 = w.create_entity();
        auto e2 = w.create_entity();
        auto e3 = w.create_entity();

        w.add<position>(e1, {1, 0, 0});
        w.add<velocity>(e1, {10, 0, 0});

        w.add<position>(e2, {2, 0, 0});
        // e2 has no velocity

        w.add<position>(e3, {3, 0, 0});
        w.add<velocity>(e3, {30, 0, 0});

        float sum = 0;
        w.each<position, velocity>([&](gamelib::entity_id, position& p, velocity& v) {
            sum += p.x + v.dx;
        });

        CHECK(sum == 44.0f);
    }

    SECTION("set_resource and get_resource")
    {
        w.set_resource<gravity>({9.81f});

        auto* g = w.get_resource<gravity>();
        REQUIRE(g != nullptr);
        CHECK(g->strength == 9.81f);
    }

    SECTION("get_resource returns nullptr when not set")
    {
        CHECK(w.get_resource<gravity>() == nullptr);
    }

    SECTION("set_resource overwrites previous value")
    {
        w.set_resource<gravity>({9.81f});
        w.set_resource<gravity>({1.62f});

        auto* g = w.get_resource<gravity>();
        REQUIRE(g != nullptr);
        CHECK(g->strength == 1.62f);
    }
}

// --- Test systems for scheduler ---

static std::vector<std::string>& execution_log()
{
    static std::vector<std::string> log;
    return log;
}

struct movement_system : gamelib::isystem
{
    [[nodiscard]] gamelib::system_desc desc() const override
    {
        return {
            .type_id = gamelib::system_type_registry::get<movement_system>(),
            .name = "movement",
            .runs_before = {},
            .runs_after = {},
        };
    }

    void update_physics(gamelib::world& w, float dt) override final
    {
        execution_log().push_back("movement");
        w.each<position, velocity>([&](gamelib::entity_id, position& p, velocity& v) {
            p.x += v.dx * dt;
            p.y += v.dy * dt;
            p.z += v.dz * dt;
        });
    }
    
    void update_game(gamelib::world& w, float dt) override final
    {
    
    }
};

struct gravity_system : gamelib::isystem
{
    [[nodiscard]] gamelib::system_desc desc() const override
    {
        return {
            .type_id = gamelib::system_type_registry::get<gravity_system>(),
            .name = "gravity",
            .runs_before = {gamelib::system_type_registry::get<movement_system>()},
            .runs_after = {},
        };
    }

    void update_physics(gamelib::world& w, float dt) override
    {
        execution_log().push_back("gravity");
        w.each<velocity>([&](gamelib::entity_id, velocity& v) {
            v.dy -= 9.81f * dt;
        });
    }

    void update_game(gamelib::world& w, float dt) override final {}
};

struct render_system : gamelib::isystem
{
    [[nodiscard]] gamelib::system_desc desc() const override
    {
        return {
            .type_id = gamelib::system_type_registry::get<render_system>(),
            .name = "render",
            .runs_before = {},
            .runs_after = {gamelib::system_type_registry::get<movement_system>()},
        };
    }

    void update_physics(gamelib::world&, float) override {}

    void update_game(gamelib::world&, float) override
    {
        execution_log().push_back("render");
    }
};

TEST_CASE("system_scheduler", "[ecs]")
{
    gamelib::system_scheduler scheduler;
    execution_log().clear();

    SECTION("register and update")
    {
        scheduler.register_system<movement_system>();

        gamelib::world w;
        auto e = w.create_entity();
        w.add<position>(e, {0, 0, 0});
        w.add<velocity>(e, {1, 0, 0});

        scheduler.update_physics(w, 1.0f);
        scheduler.update_world(w, 1.0f);

        CHECK(w.get<position>(e)->x == 1.0f);
    }

    SECTION("runs_before ordering")
    {
        scheduler.register_system<movement_system>();
        scheduler.register_system<gravity_system>();

        gamelib::world w;
        scheduler.update_physics(w, 1.0f);
        scheduler.update_world(w, 1.0f);

        REQUIRE(execution_log().size() == 2);
        CHECK(execution_log()[0] == "gravity");
        CHECK(execution_log()[1] == "movement");
    }

    SECTION("runs_after ordering")
    {
        scheduler.register_system<render_system>();
        scheduler.register_system<movement_system>();

        gamelib::world w;
        scheduler.update_physics(w, 1.0f);
        scheduler.update_world(w, 1.0f);

        REQUIRE(execution_log().size() == 2);
        CHECK(execution_log()[0] == "movement");
        CHECK(execution_log()[1] == "render");
    }

    SECTION("full ordering: gravity -> movement -> render")
    {
        scheduler.register_system<render_system>();
        scheduler.register_system<gravity_system>();
        scheduler.register_system<movement_system>();

        gamelib::world w;
        scheduler.update_physics(w, 1.0f);
        scheduler.update_world(w, 1.0f);

        REQUIRE(execution_log().size() == 3);
        CHECK(execution_log()[0] == "gravity");
        CHECK(execution_log()[1] == "movement");
        CHECK(execution_log()[2] == "render");
    }

    SECTION("ordered_system_names")
    {
        scheduler.register_system<render_system>();
        scheduler.register_system<gravity_system>();
        scheduler.register_system<movement_system>();
        scheduler.rebuild();

        auto names = scheduler.ordered_system_names();
        REQUIRE(names.size() == 3);
        CHECK(names[0] == "gravity");
        CHECK(names[1] == "movement");
        CHECK(names[2] == "render");
    }

    SECTION("unregister_system")
    {
        scheduler.register_system<movement_system>();
        scheduler.register_system<render_system>();

        scheduler.unregister_system(gamelib::system_type_registry::get<movement_system>());

        gamelib::world w;
        scheduler.update_world(w, 1.0f);

        REQUIRE(execution_log().size() == 1);
        CHECK(execution_log()[0] == "render");
    }

    SECTION("clear")
    {
        scheduler.register_system<movement_system>();
        scheduler.register_system<render_system>();
        scheduler.clear();

        gamelib::world w;
        scheduler.update_world(w, 1.0f);

        CHECK(execution_log().empty());
        CHECK(scheduler.registered_ids.empty());
    }

    SECTION("update_world triggers rebuild when dirty")
    {
        scheduler.register_system<movement_system>();

        gamelib::world w;
        scheduler.update_physics(w, 1.0f);
        scheduler.update_world(w, 1.0f);
        CHECK_FALSE(scheduler.dirty);

        scheduler.register_system<render_system>();
        CHECK(scheduler.dirty);

        scheduler.update_physics(w, 1.0f);
        scheduler.update_world(w, 1.0f);
        CHECK_FALSE(scheduler.dirty);
    }

    SECTION("systems affect world state across the pipeline")
    {
        scheduler.register_system<gravity_system>();
        scheduler.register_system<movement_system>();

        gamelib::world w;
        auto e = w.create_entity();
        w.add<position>(e, {0, 100, 0});
        w.add<velocity>(e, {5, 0, 0});

        scheduler.update_physics(w, 1.0f);
        scheduler.update_world(w, 1.0f);

        auto* p = w.get<position>(e);
        auto* v = w.get<velocity>(e);
        REQUIRE(p != nullptr);
        REQUIRE(v != nullptr);
        // gravity applied first: vy = 0 - 9.81 = -9.81
        CHECK(v->dy == Catch::Approx(-9.81f));
        // then movement: y = 100 + (-9.81) * 1 = 90.19
        CHECK(p->x == Catch::Approx(5.0f));
        CHECK(p->y == Catch::Approx(90.19f));
    }
}