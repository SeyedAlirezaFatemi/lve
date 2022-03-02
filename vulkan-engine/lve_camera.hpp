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

        const glm::mat4& getProjection() const { return projectionMatrix; }

       private:
        glm::mat4 projectionMatrix{1.f};
    };

}  // namespace lve
