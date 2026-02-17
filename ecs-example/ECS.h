#pragma once
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <cassert>

using EntityID = int;

class ECS {
public:
    EntityID createEntity() {
        EntityID id = nextID++;
        entities.push_back(id);
        return id;
    }

    const std::vector<EntityID>& getEntities() const {
        return entities;
    }

    template<typename Comp>
    void addComponent(EntityID id, Comp comp) {
        auto type = std::type_index(typeid(Comp));
        auto& map = getComponentMap<Comp>();
        map[id] = std::make_shared<Comp>(comp);
    }

    template<typename Comp>
    std::shared_ptr<Comp> getComponent(EntityID id) {
        auto& map = getComponentMap<Comp>();
        auto it = map.find(id);
        if (it != map.end()) {
            return std::static_pointer_cast<Comp>(it->second);
        }
        return nullptr;
    }

private:
    EntityID nextID = 0;
    std::vector<EntityID> entities;

    template<typename Comp>
    std::unordered_map<EntityID, std::shared_ptr<void>>& getComponentMap() {
        static std::unordered_map<EntityID, std::shared_ptr<void>> map;
        return map;
    }
};
