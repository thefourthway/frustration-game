#include "game/game.h"
#include "game/jolt_setup.h"
#include <raylib.h>

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

#include <algorithm>
#include <thread>

static const char WALL = '#';
static const char SPAWN = '@';
static const char TRASH = '^';
static const char BOTTLE = '&';

static const char* FLOOR = R"(#######
#######
#######
##@####
#######
#######
#######)";

static const char* OBJECTS = R"(#######
#  ^  #
#     #
#     #
#  &  #
#     #
#######)";

namespace g
{
    void input_state::update()
    {
        Vector2 mouse = GetMousePosition();
        mouse_pos = vec2(mouse.x, mouse.y);
    }

    void init_jolt()
    {
        static bool initialized = false;
        if (initialized) return;

        JPH::RegisterDefaultAllocator();
        JPH::Factory::sInstance = new JPH::Factory;
        JPH::RegisterTypes();

        initialized = true;
    }

    jolt_state create_jolt_state()
    {
        jolt_state s;
        s.bp_layer_iface   = std::make_unique<bp_layer_interface>();
        s.obj_vs_bp        = std::make_unique<object_vs_bp_filter>();
        s.obj_pair_filter  = std::make_unique<object_layer_pair_filter>();
        s.temp_allocator   = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
        s.job_system       = std::make_unique<JPH::JobSystemThreadPool>(
            JPH::cMaxPhysicsJobs,
            JPH::cMaxPhysicsBarriers,
            std::max(1u, std::thread::hardware_concurrency() - 1)
        );
        s.physics_system = std::make_unique<JPH::PhysicsSystem>();
        s.physics_system->Init(
            1024, 0, 1024, 1024,
            *s.bp_layer_iface, *s.obj_vs_bp, *s.obj_pair_filter
        );
        s.physics_system->SetGravity(JPH::Vec3(0, -9.81f, 0));
        return s;
    }

    void load_floor(gamelib::world& w, jolt_state& jolt, const char* map)
    {
        auto& body_iface = jolt.physics_system->GetBodyInterface();

        int row = 0, col = 0;
        for (const char* c = map; *c; ++c)
        {
            if (*c == '\n') { row++; col = 0; continue; }

            if (*c == WALL || *c == SPAWN)
            {
                float x = static_cast<float>(col);
                float z = static_cast<float>(row);

                // Floor tile: 1x1x1 box centered at (x, 0, z), top surface at y=0.5
                JPH::BoxShapeSettings floor_shape(JPH::Vec3(0.5f, 0.5f, 0.5f));
                JPH::BodyCreationSettings floor_settings(
                    floor_shape.Create().Get(),
                    JPH::RVec3(x, 0, z),
                    JPH::Quat::sIdentity(),
                    JPH::EMotionType::Static,
                    layers::NON_MOVING
                );
                JPH::BodyID floor_id = body_iface.CreateAndAddBody(floor_settings, JPH::EActivation::DontActivate);

                auto trash_ent = w.create_entity();
                w.add(trash_ent, transform{{x, 0, z}});
                w.add(trash_ent, jolt_body{floor_id});
                w.add(trash_ent, box_renderer{{1, 1, 1}, DARKGRAY});

                if (*c == '@')
                {
                    // Player: Jolt Character with capsule shape
                    constexpr float char_radius = 0.3f;
                    constexpr float char_half_height = 0.4f;

                    JPH::CharacterSettings char_settings;
                    char_settings.mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
                    char_settings.mLayer = layers::MOVING;
                    char_settings.mShape = new JPH::CapsuleShape(char_half_height, char_radius);
                    char_settings.mFriction = 0.5f;
                    char_settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -char_radius);

                    JPH::Ref<JPH::Character> character = new JPH::Character(
                        &char_settings,
                        JPH::RVec3(x, 2.0, z),
                        JPH::Quat::sIdentity(),
                        0,
                        jolt.physics_system.get()
                    );
                    character->AddToPhysicsSystem(JPH::EActivation::Activate);

                    auto player_ent = w.create_entity();
                    w.add(player_ent, transform{{x, 2.0f, z}});
                    w.add(player_ent, character_controller{character.GetPtr()});
                    w.add(player_ent, player{});
                    w.add(player_ent, inventory{.holds_item = false});

                    jolt.characters.push_back(std::move(character));
                }
            }
            col++;
        }

        jolt.physics_system->OptimizeBroadPhase();
    }

    void load_objects(gamelib::world& w, jolt_state& jolt, const char* map)
    {
        auto& body_iface = jolt.physics_system->GetBodyInterface();

        int row = 0, col = 0;
        for (const char* c = map; *c; ++c)
        {
            if (*c == '\n') { row++; col = 0; continue; }

            if (*c == WALL)
            {
                float x = static_cast<float>(col);
                float z = static_cast<float>(row);

                JPH::BoxShapeSettings floor_shape(JPH::Vec3(0.5f, 1.5f, 0.5f));
                JPH::BodyCreationSettings floor_settings(
                    floor_shape.Create().Get(),
                    JPH::RVec3(x, 1, z),
                    JPH::Quat::sIdentity(),
                    JPH::EMotionType::Static,
                    layers::NON_MOVING
                );
                JPH::BodyID floor_id = body_iface.CreateAndAddBody(floor_settings, JPH::EActivation::DontActivate);

                auto floor_ent = w.create_entity();
                w.add(floor_ent, transform{{x, 1, z}});
                w.add(floor_ent, jolt_body{floor_id});
                w.add(floor_ent, box_renderer{{1, 3, 1}, DARKGRAY});
            } else if (*c == TRASH)
            {
                float x = static_cast<float>(col);
                float z = static_cast<float>(row);

                JPH::BoxShapeSettings floor_shape(JPH::Vec3(0.5f, 0.5f, 0.5f));
                JPH::BodyCreationSettings floor_settings(
                    floor_shape.Create().Get(),
                    JPH::RVec3(x, 1, z),
                    JPH::Quat::sIdentity(),
                    JPH::EMotionType::Static,
                    layers::NON_MOVING
                );
                JPH::BodyID trash_id = body_iface.CreateAndAddBody(floor_settings, JPH::EActivation::DontActivate);

                auto trash_ent = w.create_entity();
                w.add(trash_ent, transform{{x, 1, z}});
                w.add(trash_ent, jolt_body{trash_id});
                w.add(trash_ent, box_renderer{{1, 1, 1}, ORANGE});
                w.add(trash_ent, is_trash{});

                jolt.body_to_entity[trash_id.GetIndexAndSequenceNumber()] = trash_ent;
            } else if (*c == BOTTLE)
            {
                float x = static_cast<float>(col);
                float z = static_cast<float>(row);

                JPH::BoxShapeSettings floor_shape(JPH::Vec3(0.5f, 0.5f, 0.5f));
                JPH::BodyCreationSettings floor_settings(
                    floor_shape.Create().Get(),
                    JPH::RVec3(x, 1, z),
                    JPH::Quat::sIdentity(),
                    JPH::EMotionType::Static,
                    layers::NON_MOVING
                );
                JPH::BodyID bottle_id = body_iface.CreateAndAddBody(floor_settings, JPH::EActivation::DontActivate);

                auto bottle_ent = w.create_entity();
                w.add(bottle_ent, transform{{x, 1, z}});
                w.add(bottle_ent, jolt_body{bottle_id});
                w.add(bottle_ent, box_renderer{{1, 1, 1}, PURPLE});
                w.add(bottle_ent, is_bottle{});

                jolt.body_to_entity[bottle_id.GetIndexAndSequenceNumber()] = bottle_ent;
            }

            col++;
        }

        jolt.physics_system->OptimizeBroadPhase();
    }

    game::game(::g::window& w) : window(w)
    {
        init_jolt();

        auto jolt = create_jolt_state();
        load_floor(world, jolt, FLOOR);
        load_objects(world, jolt, OBJECTS);
        world.set_resource(std::move(jolt));

        DisableCursor();

        camera_state cam;
        cam.camera.position = {2.0f, 2.0f, 2.0f};
        cam.camera.target   = {2.0f, 2.0f, 1.0f};
        cam.camera.up       = {0.0f, 1.0f, 0.0f};
        cam.camera.fovy     = 70.0f;
        cam.camera.projection = CAMERA_PERSPECTIVE;
        world.set_resource(std::move(cam));

        scheduler.register_system<player_system>();
        scheduler.register_system<inventory_system>();
        scheduler.register_system<physics_system>();
        scheduler.register_system<render_system>();
    }

    void game::update_physics(const float dt)
    {
        scheduler.update_physics(world, dt);
    }

    void game::update_game(const float dt)
    {
        scheduler.update_world(world, dt);
    }

    void game::draw(const float alpha)
    {
        scheduler.draw(world, alpha);
    }
}
