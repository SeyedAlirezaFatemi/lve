#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "lve_model.hpp"
#include "memory"

namespace lve {
    struct TransformComponent {
        glm::vec3 translation{};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
        glm::vec3 rotation{};

        glm::mat4 modelToWorldMatrix();
        glm::mat3 normalToWorldMatrix();
    };

    class LVEGameObject {
       public:
        using id_t = unsigned int;

        static LVEGameObject createGameObject() {
            static id_t currentId = 0;
            return LVEGameObject{currentId++};
        }

        LVEGameObject(const LVEGameObject &) = delete;
        LVEGameObject &operator=(const LVEGameObject &) = delete;

        LVEGameObject(LVEGameObject &&) = default;
        LVEGameObject &operator=(LVEGameObject &&) = default;

        id_t getId() const { return id; }

        std::shared_ptr<LVEModel> model{};
        glm::vec3 color{};
        TransformComponent transform{};

       private:
        LVEGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}  // namespace lve
