#include "constants.h"

namespace vka
{
const std::vector<glm::vec3> cCubeVertecies =
{
        glm::vec3(0.0, 0.0, 1.0),
        glm::vec3(1.0, 0.0, 1.0),
        glm::vec3(1.0, 1.0, 1.0),
        glm::vec3(0.0, 1.0, 1.0),

        glm::vec3(0.0, 0.0, 0.0),
        glm::vec3(1.0, 0.0, 0.0),
        glm::vec3(1.0, 1.0, 0.0),
        glm::vec3(0.0, 1.0, 0.0),
};

const std::vector<uint32_t> cCubeTriangleIndices =
    {
        // front
        0, 1, 2,
        2, 3, 0,
        // right
        1, 5, 6,
        6, 2, 1,
        // back
        7, 6, 5,
        5, 4, 7,
        // left
        4, 0, 3,
        3, 7, 4,
        // bottom
        4, 5, 1,
        1, 0, 4,
        // top
        3, 2, 6,
        6, 7, 3};
const std::vector<uint32_t> cCubeLineIndices =
    {

        0, 1,
        1, 2,
        2, 3,
        3, 0,

        4, 5, 
        5, 6,
        6, 7,
        7, 4,

        0, 4,
        1, 5,
        2, 6,
        3, 7
};

}