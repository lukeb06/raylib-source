#pragma once

#include <cstddef>
#include <memory>
#include <typeindex>
#include <unordered_map>

using Entity = std::size_t;

struct IComponentPool {
    virtual ~IComponentPool() = default;
    virtual void Remove(Entity entity) = 0;
};

template <typename T> class ComponentPool : public IComponentPool {
  public:
    std::unordered_map<Entity, T> data;

    void Remove(Entity entity) override { data.erase(entity); }
};

class Registry {
  private:
    Entity nextEntity = 1;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> pools;

    template <typename T> ComponentPool<T> *GetPool() {
        auto type = std::type_index(typeid(T));
        if (pools.find(type) == pools.end()) {
            pools[type] = std::make_unique<ComponentPool<T>>();
        }
        return static_cast<ComponentPool<T> *>(pools[type].get());
    }

  public:
    Registry() = default;

    Entity CreateEntity();
    Entity CreateLocalPlayer();
    void Clear();

    template <typename T> T &AddComponent(Entity entity, T component = T{}) {
        auto pool = GetPool<T>();
        pool->data[entity] = component;
        return pool->data[entity];
    }

    template <typename T> T *GetComponent(Entity entity) {
        auto pool = GetPool<T>();
        auto it = pool->data.find(entity);
        return (it != pool->data.end()) ? &it->second : nullptr;
    }

    template <typename T> bool HasComponent(Entity entity) {
        return GetComponent<T>(entity) != nullptr;
    }

    template <typename T> ComponentPool<T> *View() { return GetPool<T>(); }
};
