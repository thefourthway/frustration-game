#pragma once
#include <absl/container/flat_hash_map.h>
#include <vector>
#include <functional>
#include <gamelib/ecs/entity.hpp>

namespace gamelib
{
    using component_type_id = uint32_t;
 
    struct component_type_registry {
        template<typename T>
        static component_type_id get() {
            static component_type_id id = next_id();
            return id;
        }
    private:
        static component_type_id next_id() {
            static component_type_id counter = 0;
            return counter++;
        }
    };

    struct icomponent_pool
    {
        virtual ~icomponent_pool() = default;
        virtual void remove(entity_id) = 0;
        virtual bool has(entity_id) const = 0;
    };

    template<typename T>
    struct component_pool : public icomponent_pool
    {
        void add(entity_id entity, T component)
        {
            if (sparse.contains(entity)) return;

            dense.emplace_back(std::move(component));
            owners.push_back(entity);
            sparse.try_emplace(entity, static_cast<std::uint32_t>(dense.size() - 1));
        }

        template<typename... Args>
        void add(entity_id entity, Args... a)
        {
            if (sparse.contains(entity)) return;

            dense.emplace_back(std::forward<Args>(a)...);
            owners.push_back(entity);
            sparse.try_emplace(entity, static_cast<std::uint32_t>(dense.size() - 1));
        }

        void remove(entity_id entity) override final
        {
            auto it = sparse.find(entity);
            if (it == sparse.end()) return;
            
            auto& idx = it->second;

            std::uint32_t last = static_cast<std::uint32_t>(dense.size()) - 1;

            if (idx != last)
            {
                dense[idx] = std::move(dense[last]);
                owners[idx] = owners[last];
                sparse[owners[idx]] = idx;
            }

            dense.pop_back();
            owners.pop_back();
            sparse.erase(entity);
        }

        T* get(entity_id entity)
        {
            auto it = sparse.find(entity);
            if (it == sparse.end()) return nullptr;
            return &dense[it->second];
        }

        [[nodiscard]] const T* get(entity_id entity) const
        {
            auto it = sparse.find(entity);
            if (it == sparse.end()) return nullptr;
            return &dense[it->second];
        }

        [[nodiscard]] bool has(entity_id entity) const final
        {
            return sparse.contains(entity);
        }

        void each(std::function<void(entity_id, T&)> fn)
        {
            for (auto i{0uz}; i < dense.size(); ++i)
            {
                fn(owners[i], dense[i]);
            }
        }

        std::vector<T> dense;
        std::vector<entity_id> owners;
        absl::flat_hash_map<entity_id, std::uint32_t> sparse;
    };
}