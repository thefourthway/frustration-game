#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <gamelib/ecs/world.hpp>
#include <memory>
#include <vector>

namespace g
{
    namespace layers
    {
        static constexpr JPH::ObjectLayer NON_MOVING = 0;
        static constexpr JPH::ObjectLayer MOVING = 1;
    }

    class bp_layer_interface final : public JPH::BroadPhaseLayerInterface
    {
    public:
        JPH::uint GetNumBroadPhaseLayers() const override { return 2; }
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
        {
            return JPH::BroadPhaseLayer(static_cast<JPH::BroadPhaseLayer::Type>(layer));
        }
        const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
        {
            switch (static_cast<JPH::BroadPhaseLayer::Type>(layer))
            {
                case 0: return "NON_MOVING";
                case 1: return "MOVING";
                default: return "UNKNOWN";
            }
        }
    };

    class object_vs_bp_filter final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const override
        {
            if (layer1 == layers::NON_MOVING)
                return layer2 == JPH::BroadPhaseLayer(1);
            return true;
        }
    };

    class object_layer_pair_filter final : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer layer1, JPH::ObjectLayer layer2) const override
        {
            if (layer1 == layers::NON_MOVING && layer2 == layers::NON_MOVING)
                return false;
            return true;
        }
    };

    struct jolt_body
    {
        JPH::BodyID body_id;
    };

    struct character_controller
    {
        JPH::Character* character{};
    };

    struct jolt_state
    {
        std::unique_ptr<bp_layer_interface> bp_layer_iface;
        std::unique_ptr<object_vs_bp_filter> obj_vs_bp;
        std::unique_ptr<object_layer_pair_filter> obj_pair_filter;
        std::unique_ptr<JPH::TempAllocatorImpl> temp_allocator;
        std::unique_ptr<JPH::JobSystemThreadPool> job_system;
        std::unique_ptr<JPH::PhysicsSystem> physics_system;
        std::vector<JPH::Ref<JPH::Character>> characters;
        absl::flat_hash_map<std::uint32_t, gamelib::entity_id> body_to_entity;

        jolt_state() = default;
        jolt_state(jolt_state&&) noexcept = default;
        jolt_state& operator=(jolt_state&&) noexcept = default;

        ~jolt_state()
        {
            for (auto& c : characters)
                c->RemoveFromPhysicsSystem();
        }
    };

    void init_jolt();
    jolt_state create_jolt_state();
    void load_floor(gamelib::world& w, jolt_state& jolt, const char* map);
}
