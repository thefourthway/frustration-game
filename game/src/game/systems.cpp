#include "game/systems.h"
#include "game/components.h"
#include "game/jolt_setup.h"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <print>

namespace g
{
    // --- physics_system ---

    gamelib::system_desc physics_system::desc() const
    {
        return {
            .type_id = gamelib::system_type_registry::get<physics_system>(),
            .name = "physics",
            .runs_before = {},
            .runs_after = { gamelib::system_type_registry::get<player_system>() },
        };
    }

    void physics_system::update_physics(gamelib::world& w, float dt)
    {
        auto* jolt = w.get_resource<jolt_state>();
        if (!jolt) return;

        jolt->physics_system->Update(dt, 1, jolt->temp_allocator.get(), jolt->job_system.get());

        // Sync transforms for regular physics bodies
        auto& body_iface = jolt->physics_system->GetBodyInterface();
        w.each<jolt_body, transform>([&](gamelib::entity_id, jolt_body& body, transform& t) {
            JPH::RVec3 pos = body_iface.GetPosition(body.body_id);
            t.position = vec3(
                static_cast<float>(pos.GetX()),
                static_cast<float>(pos.GetY()),
                static_cast<float>(pos.GetZ())
            );
        });

        // Sync transforms for character controllers
        w.each<character_controller, transform>([&](gamelib::entity_id, character_controller& cc, transform& t) {
            cc.character->PostSimulation(0.05f);
            JPH::RVec3 pos = cc.character->GetPosition();
            t.position = vec3(
                static_cast<float>(pos.GetX()),
                static_cast<float>(pos.GetY()),
                static_cast<float>(pos.GetZ())
            );
        });
    }

    // --- player_system ---

    gamelib::system_desc player_system::desc() const
    {
        return {
            .type_id = gamelib::system_type_registry::get<player_system>(),
            .name = "player",
            .runs_before = { gamelib::system_type_registry::get<physics_system>() },
            .runs_after = {},
        };
    }

    void player_system::update_physics(gamelib::world& w, float dt)
    {
        auto* jolt = w.get_resource<jolt_state>();
        if (!jolt) return;

        w.each<player, character_controller, transform>([&](gamelib::entity_id, player& p, character_controller& cc, transform& t) {
            // Mouse look — accumulate yaw/pitch as floats, clamp pitch
            constexpr float sensitivity = 0.003f;
            constexpr float max_pitch = 1.4f; // ~80 degrees
            Vector2 delta = GetMouseDelta();

            p.yaw   -= delta.x * sensitivity;
            p.pitch -= delta.y * sensitivity;
            p.pitch  = std::clamp(p.pitch, -max_pitch, max_pitch);

            // Build orientation quaternion: yaw first, then pitch
            Quaternion orientation = QuaternionMultiply(
                QuaternionFromAxisAngle({0, 1, 0}, p.yaw),
                QuaternionFromAxisAngle({1, 0, 0}, p.pitch)
            );

            Vector3 full_forward = Vector3RotateByQuaternion({0, 0, -1}, orientation);

            // Forward/right on XZ plane for movement (ignore pitch)
            Vector3 flat_forward = Vector3Normalize({full_forward.x, 0, full_forward.z});
            Vector3 right = Vector3CrossProduct({0, 1, 0}, flat_forward);

            // WASD relative to facing direction
            Vector3 move_dir = {0, 0, 0};
            if (IsKeyDown(KEY_W)) move_dir = Vector3Add(move_dir, flat_forward);
            if (IsKeyDown(KEY_S)) move_dir = Vector3Subtract(move_dir, flat_forward);
            if (IsKeyDown(KEY_A)) move_dir = Vector3Add(move_dir, right);
            if (IsKeyDown(KEY_D)) move_dir = Vector3Subtract(move_dir, right);

            constexpr float speed = 5.0f;
            if (Vector3Length(move_dir) > 0.001f)
                move_dir = Vector3Scale(Vector3Normalize(move_dir), speed);

            JPH::Vec3 current_vel = cc.character->GetLinearVelocity();
            float y_vel = current_vel.GetY();

            // Ground check via Jolt Character
            bool grounded = cc.character->GetGroundState() == JPH::Character::EGroundState::OnGround;

            constexpr float jump_force = 6.0f;
            if (IsKeyPressed(KEY_SPACE) && grounded)
                y_vel = jump_force;

            cc.character->SetLinearVelocity(JPH::Vec3(move_dir.x, y_vel, move_dir.z));

            // Update camera to follow player
            auto* cam = w.get_resource<camera_state>();
            if (cam)
            {
                constexpr float eye_height = 0.6f;
                Vector3 eye = {t.position.x, t.position.y + eye_height, t.position.z};

                cam->camera.position = eye;
                cam->camera.target = Vector3Add(eye, full_forward);
            }
        });
    }

    // --- render_system ---

    gamelib::system_desc render_system::desc() const
    {
        return {
            .type_id = gamelib::system_type_registry::get<render_system>(),
            .name = "render",
            .runs_before = {},
            .runs_after = {},
        };
    }

    void render_system::draw(gamelib::world& w, float)
    {
        auto* cam = w.get_resource<camera_state>();
        if (!cam) return;

        BeginMode3D(cam->camera);

        DrawGrid(20, 1.0f);

        w.each<transform, box_renderer>([&](gamelib::entity_id e, transform& t, box_renderer& r) {
            if (w.has<player>(e)) return;
            DrawCube(t.position, r.size.x, r.size.y, r.size.z, r.color);
            DrawCubeWires(t.position, r.size.x, r.size.y, r.size.z, BLACK);
        });

        w.each<player, character_controller, inventory>([&](
            gamelib::entity_id entity,
            player& p,
            character_controller& cc,
            inventory& i
        ) {
            if (!i.holds_item) return;

            Quaternion orientation = QuaternionMultiply(
                QuaternionFromAxisAngle({0, 1, 0}, p.yaw),
                QuaternionFromAxisAngle({1, 0, 0}, p.pitch)
            );

            Vector3 offset = Vector3RotateByQuaternion({0.3f, -0.3f, -0.6f}, orientation);
            Vector3 sphere_pos = Vector3Add(cam->camera.position, offset);

            DrawSphere(sphere_pos, 0.1f, YELLOW);
        });

        EndMode3D();
    }

    gamelib::system_desc inventory_system::desc() const
    {
        return {
            .type_id = gamelib::system_type_registry::get<inventory_system>(),
            .name = "inventory",
            .runs_before = {},
            .runs_after = {}
        };
    }

    void inventory_system::update_game(gamelib::world& w, float dt)
    {
        auto* cam = w.get_resource<camera_state>();
        auto* jolt = w.get_resource<jolt_state>();

        if (!cam || !jolt) return;
        
        w.each<player, character_controller, inventory>([&](
            gamelib::entity_id entity,
            player& p,
            character_controller& cc,
            inventory& i
        ) {
            if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return;

            Vector3 origin = cam->camera.position;
            Vector3 target = cam->camera.target;
            Vector3 dir = Vector3Subtract(target, origin);    
            constexpr const float max_dist = 2.0f;

            dir = Vector3Scale(Vector3Normalize(dir), max_dist);

            JPH::RRayCast ray(
                JPH::RVec3(origin.x, origin.y, origin.z),
                JPH::Vec3(dir.x, dir.y, dir.z)
            );

            JPH::RayCastResult hit;
            JPH::IgnoreSingleBodyFilter body_filter(cc.character->GetBodyID());

            const bool did_hit = jolt->physics_system->GetNarrowPhaseQuery().CastRay(
                ray,
                hit,
                JPH::BroadPhaseLayerFilter{},
                JPH::ObjectLayerFilter{},
                body_filter
            );

            if (!did_hit) return;

            auto hit_body_id = hit.mBodyID.GetIndexAndSequenceNumber();
                
            if (!jolt->body_to_entity.contains(hit_body_id)) return;

            auto hit_entity_id = jolt->body_to_entity.find(hit_body_id)->second;

            if (w.has<is_trash>(hit_entity_id))
            {
                if (!i.holds_item) return;

                i.holds_item = false;

                w.get<box_renderer>(hit_entity_id)->color = GREEN;
            } else if (w.has<is_bottle>(hit_entity_id))
            {
                i.holds_item = true;

                auto& body_iface = jolt->physics_system->GetBodyInterface();
                body_iface.RemoveBodies(&hit.mBodyID, 1);

                w.remove_entity(hit_entity_id);
                jolt->body_to_entity.erase(hit_entity_id);
            }
        });
    }
}
