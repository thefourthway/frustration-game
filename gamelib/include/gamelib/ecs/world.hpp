#pragma once
#include <gamelib/ecs/entity.hpp>
#include <gamelib/ecs/component_pool.hpp>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <typeindex>
#include <vector>

namespace gamelib
{
    class world
    {
    public:
        entity_id create_entity() {
            entity_id id;
            if (!_free_list.empty())
            {
                id = _free_list.back();
                _free_list.pop_back();
            } else {
                id = _next_id++;
            }
            _alive.emplace(id);
            return id;
        }

        void destroy_entity(entity_id entity)
        {
            if (!_alive.contains(entity)) return;
            for (auto& [_, pool] : _pools) pool->remove(entity);
            _alive.erase(entity);
            _free_list.push_back(entity);
        }

        [[nodiscard]] inline bool is_alive(entity_id entity) const { return _alive.contains(entity); }
        [[nodiscard]] inline std::size_t entity_count() const { return _alive.size(); }

        template<typename T>
        void add(entity_id entity, T component = T{})
        {
            if (!is_alive(entity)) return;
            pool<T>().add(entity, std::move(component));
        }

        template<typename T> T* get(entity_id entity) { return pool<T>().get(entity); }
        template<typename T> void remove(entity_id entity) { pool<T>().remove(entity); }

        void remove_entity(entity_id entity) {
            for (auto& [_, pool] : _pools)
                pool->remove(entity); 
        }

        template<typename T>
        [[nodiscard]] bool has(entity_id entity) const
        {
            auto it = _pools.find(component_type_registry::get<T>());
            return it != _pools.end() && it->second->has(entity);
        }

        template<typename T, typename ... Rest, typename Fn>
        void each(Fn&& fn)
        {
            pool<T>().each([&](entity_id entity, T& t) {
                auto ptrs = std::tuple<Rest*...>{ get<Rest>(entity)... };

                bool all_present = std::apply(
                    [](auto*... p) {
                        return (... && (p != nullptr));
                    },
                    ptrs
                );

                if (all_present) std::apply([&](auto*... p) { fn(entity, t, *p...); }, ptrs);
            });
        }

        template<typename T>
        void set_resource(T value)
        {
            _resources[std::type_index(typeid(T))] = std::make_unique<holder<T>>(std::move(value));
        }

        template<typename T>
        T* get_resource()
        {
            auto it = _resources.find(std::type_index(typeid(T)));
            if (it == _resources.end()) return nullptr;
            return &static_cast<holder<T>*>(it->second.get())->value;
        }
    private:
        template<typename T>
        component_pool<T>& pool()
        {
            component_type_id id = component_type_registry::get<T>();
            auto [it, inserted] = _pools.emplace(id, nullptr);
            if (inserted) it->second = std::make_unique<component_pool<T>>();
            return *static_cast<component_pool<T>*>(it->second.get());
        }

        struct iholder { virtual ~iholder() = default; };

        template<typename T>
        struct holder : iholder {
            T value;
            explicit holder(T v) : value(std::move(v)) {}
        };

        entity_id _next_id{0};
        
        absl::flat_hash_set<entity_id> _alive;
        std::vector<entity_id> _free_list;
        absl::flat_hash_map<component_type_id, std::unique_ptr<icomponent_pool>> _pools;
        absl::flat_hash_map<std::type_index, std::unique_ptr<iholder>> _resources;
    };
}