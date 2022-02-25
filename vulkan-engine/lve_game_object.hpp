#pragma once

#include "lve_model.hpp"
#include "memory"

namespace lve {
    struct Transform2DComponent {
        glm::vec2 translation{};
        glm::vec2 scale{1.0f, 1.0f};
        float rotation;

        glm::mat2 mat2() {
            const float s = glm::sin(rotation);
            const float c = glm::cos(rotation);
            glm::mat2 rotMat{{c, s}, {-s, c}};
            glm::mat2 scaleMat{{scale.x, .0f}, {0.0f, scale.y}};
            return rotMat * scaleMat;
        }
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
        Transform2DComponent transform2d{};

       private:
        LVEGameObject(id_t objId) : id{objId} {}

        id_t id;
    };
}  // namespace lve
