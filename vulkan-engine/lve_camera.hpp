#pragma once

// Signal GLM to expect angles to be specified in radians
#define GLM_FORCE_RADIANS
// Signal GLM to expect the depth buffer values to range from 0 to 1. OpenGL is -1 to 1.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace lve {
    class LVECamera {
       public:
        void setOrthographicProjection(
            float left, float right, float top, float bottom, float near, float far);

        // fovy: Vertical field of view
        void setPerspectiveProjection(float fovy, float aspect, float near, float far);

        void setViewDirection(glm::vec3 position,
                              glm::vec3 direction,
                              glm::vec3 up = glm::vec3{.0f, -1.f, .0f});

        void setViewTarget(glm::vec3 position,
                           glm::vec3 target,
                           glm::vec3 up = glm::vec3{.0f, -1.f, .0f});

        void setViewYXZ(glm::vec3 position, glm::vec3 rotation);  // Euler angles

        const glm::mat4& getProjection() const { return projectionMatrix; }
        const glm::mat4& getView() const { return viewMatrix; }

       private:
        glm::mat4 projectionMatrix{1.f};
        glm::mat4 viewMatrix{1.f};  // Stores the camera transform
    };

}  // namespace lve
