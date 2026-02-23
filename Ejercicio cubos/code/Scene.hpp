// Este código es de dominio público
// angel.rodriguez@udit.es

#pragma once

#include "Cube.hpp"
#include <vector>
#include <string>
#include <glm.hpp>

namespace udit
{

    class Scene
    {
    private:

        // ==========================
        // SHADERS
        // ==========================

        static const std::string vertex_shader_code;
        static const std::string fragment_shader_code;

        GLint model_view_matrix_id;
        GLint projection_matrix_id;
        GLuint square_vao;
        GLuint square_vbo;


        Cube cube;

        // ==========================
        // BOX STRUCT
        // ==========================

        struct Box
        {
            enum Direction
            {
                LEFT,
                RIGHT,
                UP,
                DOWN
            };

            glm::vec3 position;
            Direction direction;
            float speed;

            Box(const glm::vec3& start_position,
                Direction start_direction);

            void update(float length);
            glm::mat4 get_transform();

        private:
            void check_boundaries(float length);
        };

        std::vector<Box> boxes;
        float square_path_length = 6.f;

    public:

        Scene(unsigned width, unsigned height);

        void update();
        void render();
        void resize(unsigned width, unsigned height);

    private:

        void create_boxes();

        GLuint compile_shaders();
        void show_compilation_error(GLuint shader_id);
        void show_linkage_error(GLuint program_id);
    };

}
