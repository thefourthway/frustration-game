#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <gamelib/ecs/world.hpp>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

namespace gamelib
{
    using system_type_id = std::uint32_t;
 
    struct system_type_registry {
        template<typename T>
        static system_type_id get() {
            static system_type_id id = next_id();
            return id;
        }
    private:
        static system_type_id next_id() {
            static system_type_id counter = 0;
            return counter++;
        }
    };

    struct system_desc
    {
        system_type_id type_id;
        std::string name;
        std::vector<system_type_id> runs_before;
        std::vector<system_type_id> runs_after;
    };

    struct isystem
    {
    public:
        virtual ~isystem() = default;
        [[nodiscard]] virtual system_desc desc() const = 0;
        virtual void update_physics(world&, float dt) {};
        virtual void update_game(world&, float dt) {};
        virtual void draw(world&, float alpha) {};
    };

    struct system_scheduler
    {
        template<typename T, typename... Args>
        void register_system(Args&&... a)
        {
            register_system(std::make_unique<T>(std::forward<Args>(a)...));
        }

        void register_system(std::unique_ptr<isystem> system)
        {
            system_desc d = system->desc();
            system_type_id id = d.type_id;
            registered_ids.push_back(id);
            descs[id] = std::move(d);
            systems[id] = std::move(system);
            dirty = true;
        }

        void unregister_system(system_type_id id)
        {
            registered_ids.erase(
                std::remove(registered_ids.begin(), registered_ids.end(), id),
                registered_ids.end()
            );
            descs.erase(id);
            systems.erase(id);
            dirty = true;
        }

        void clear()
        {
            registered_ids.clear();
            descs.clear();
            systems.clear();
            sorted_order.clear();
            dirty = false;
        }

        void rebuild() {
            sorted_order.clear();
            if (registered_ids.empty()) { dirty = false; return; }
    
            absl::flat_hash_map<system_type_id, absl::flat_hash_set<system_type_id>> succs;
            absl::flat_hash_map<system_type_id, size_t> indegree;
    
            for (auto id : registered_ids) {
                succs[id];
                indegree[id] = 0;
            }
    
            auto add_edge = [&](system_type_id before, system_type_id after) {
                if (!descs.count(before) || !descs.count(after)) return;
                if (succs[before].insert(after).second)
                    ++indegree[after];
            };
    
            for (auto id : registered_ids) {
                for (system_type_id dep : descs[id].runs_after)  add_edge(dep, id);
                for (system_type_id dep : descs[id].runs_before) add_edge(id, dep);
            }
    
            // Kahn's — process zero-indegree nodes, break ties by registration order
            std::vector<system_type_id> ready;
            for (auto id : registered_ids)
                if (indegree[id] == 0) ready.push_back(id);
    
            while (!ready.empty()) {
                std::sort(ready.begin(), ready.end(), [&](system_type_id a, system_type_id b) {
                    return std::find(registered_ids.begin(), registered_ids.end(), a)
                        < std::find(registered_ids.begin(), registered_ids.end(), b);
                });
    
                system_type_id cur = ready.front();
                ready.erase(ready.begin());
                sorted_order.push_back(cur);
    
                for (system_type_id succ : succs[cur])
                    if (--indegree[succ] == 0) ready.push_back(succ);
            }
    
            if (sorted_order.size() != registered_ids.size())
                throw std::runtime_error("system_scheduler: cycle detected in system ordering");
    
            dirty = false;
        }

        void update_physics(world& w, float dt)
        {
            if (dirty) rebuild();
            for (auto id : sorted_order)
            {
                systems[id]->update_physics(w, dt);
            }
        }

        void update_world(world& w, float dt)
        {
            if (dirty) rebuild();
            for (auto id : sorted_order)
            {
                systems[id]->update_game(w, dt);
            }
        }

        void draw(world& w, float alpha)
        {
            if (dirty) rebuild();
            for (auto id : sorted_order)
            {
                systems[id]->draw(w, alpha);
            }
        }

        std::vector<std::string_view> ordered_system_names() const {
            std::vector<std::string_view> names;
            names.reserve(sorted_order.size());

            for (auto id : sorted_order)
            {
                auto it = descs.find(id);
                if (it == descs.end()) continue;

                names.emplace_back(std::string_view { it->second.name });
            }

            return names;
        }

        std::vector<system_type_id> registered_ids;
        absl::flat_hash_map<system_type_id, system_desc> descs;
        absl::flat_hash_map<system_type_id, std::unique_ptr<isystem>> systems;
        std::vector<system_type_id> sorted_order;
        bool dirty{false};
    };
}