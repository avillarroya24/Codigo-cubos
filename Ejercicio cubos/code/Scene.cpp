// Este código es de dominio público
// angel.rodriguez@udit.es

#include "Scene.hpp"

#include <iostream>
#include <cassert>

#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

namespace udit
{

    using namespace std;

    // ============================================================
    // SHADERS
    // ============================================================

    const string Scene::vertex_shader_code =
        "#version 330\n"
        "uniform mat4 model_view_matrix;"
        "uniform mat4 projection_matrix;"
        "layout (location = 0) in vec3 vertex_coordinates;"
        "layout (location = 1) in vec3 vertex_color;"
        "out vec3 front_color;"
        "void main()"
        "{"
        "   gl_Position = projection_matrix * model_view_matrix * vec4(vertex_coordinates, 1.0);"
        "   front_color = vertex_color;"
        "}";

    const string Scene::fragment_shader_code =
        "#version 330\n"
        "in vec3 front_color;"
        "out vec4 fragment_color;"
        "void main()"
        "{"
        "   fragment_color = vec4(front_color, 1.0);"
        "}";

    // ============================================================
    // BOX
    // ============================================================

    Scene::Box::Box(const glm::vec3& start_position, Direction start_direction)
    {
        position = start_position;
        direction = start_direction;
        speed = 0.05f;
    }

    void Scene::Box::update(float length)
    {
        glm::vec3 displacement(0.f);

        switch (direction)
        {
        case LEFT:  displacement = glm::vec3(-1.f, 0.f, 0.f); break;
        case RIGHT: displacement = glm::vec3(1.f, 0.f, 0.f); break;
        case UP:    displacement = glm::vec3(0.f, 1.f, 0.f); break;
        case DOWN:  displacement = glm::vec3(0.f, -1.f, 0.f); break;
        }

        position += displacement * speed;
        check_boundaries(length);
    }

    glm::mat4 Scene::Box::get_transform()
    {
        glm::mat4 transform(1.f);
        transform = glm::translate(transform, position);
        transform = glm::scale(transform, glm::vec3(0.2f, 0.2f, 0.2f));
        return transform;
    }

    void Scene::Box::check_boundaries(float length)
    {
        float half = length / 2.f;

        switch (direction)
        {
        case LEFT:
            if (position.x < -half) { position.x = -half; direction = DOWN; }
            break;

        case DOWN:
            if (position.y < -half) { position.y = -half; direction = RIGHT; }
            break;

        case RIGHT:
            if (position.x > half) { position.x = half; direction = UP; }
            break;

        case UP:
            if (position.y > half) { position.y = half; direction = LEFT; }
            break;
        }
    }

    // ============================================================
    // SCENE
    // ============================================================

    Scene::Scene(unsigned width, unsigned height)
    {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glClearColor(.2f, .2f, .2f, 1.f);

        GLuint program_id = compile_shaders();
        glUseProgram(program_id);

        model_view_matrix_id = glGetUniformLocation(program_id, "model_view_matrix");
        projection_matrix_id = glGetUniformLocation(program_id, "projection_matrix");

        resize(width, height);

        create_boxes();

        // ============================================================
        // CREAR CONTORNO DEL CUADRADO (solo líneas)
        // ============================================================

        float half = square_path_length * 0.5f;

        float square_vertices[] =
        {
            -half, -half, 0.f,
             half, -half, 0.f,

             half, -half, 0.f,
             half,  half, 0.f,

             half,  half, 0.f,
            -half,  half, 0.f,

            -half,  half, 0.f,
            -half, -half, 0.f
        };

        glGenVertexArrays(1, &square_vao);
        glBindVertexArray(square_vao);

        glGenBuffers(1, &square_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, square_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices), square_vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }

    void Scene::create_boxes()
    {
        float half = square_path_length / 2.f;
        int num_cubos = 5;

        // Centrar los cubos dentro del cuadrado
        float start_x = -half;
        float step = square_path_length / (num_cubos - 1);
        float current_x = start_x;

        for (int i = 0; i < num_cubos; ++i)
        {
            boxes.emplace_back(glm::vec3(current_x, half, 0.f), Box::LEFT);
            boxes.emplace_back(glm::vec3(current_x, -half, 0.f), Box::RIGHT);
            current_x += step;
        }

        float current_y = -half + step; //Nos saltamos una de las esquinas

        for (int i = 0; i < num_cubos - 2; ++i)
        {
            boxes.emplace_back(glm::vec3(-half, current_y, 0.f), Box::DOWN);
            boxes.emplace_back(glm::vec3(half, current_y, 0.f), Box::UP);
            current_y += step;
        }
    }


    void Scene::update()
    {
        for (auto& box : boxes)
            box.update(square_path_length);
    }

    void Scene::render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 cam_pos(0.f, 0.f, 12.f);
        glm::vec3 cam_target(0.f, 0.f, 0.f);
        glm::vec3 cam_up(0.f, 1.f, 0.f);

        glm::mat4 view_matrix = glm::lookAt(cam_pos, cam_target, cam_up);

        // DIBUJAR CONTORNO DEL CUADRADO
        glm::mat4 model(1.f);
        glm::mat4 mv = view_matrix * model;

        glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(mv));

        glBindVertexArray(square_vao);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glVertexAttrib3f(1, 1.f, 1.f, 1.f); // líneas blancas
        glDrawArrays(GL_LINES, 0, 8);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glBindVertexArray(0);

        // DIBUJAR CUBOS
        for (auto& box : boxes)
        {
            glm::mat4 cube_mv = view_matrix * box.get_transform();
            glUniformMatrix4fv(model_view_matrix_id, 1, GL_FALSE, glm::value_ptr(cube_mv));
            cube.render();
        }
    }

    void Scene::resize(unsigned width, unsigned height)
    {
        glm::mat4 projection_matrix =
            glm::perspective(glm::radians(59.f),
                GLfloat(width) / height,
                1.f,
                5000.f);

        glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, glm::value_ptr(projection_matrix));
        glViewport(0, 0, width, height);
    }

    GLuint Scene::compile_shaders()
    {
        GLint succeeded = GL_FALSE;

        GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

        const char* vcode[] = { vertex_shader_code.c_str() };
        const char* fcode[] = { fragment_shader_code.c_str() };

        const GLint vsize[] = { (GLint)vertex_shader_code.size() };
        const GLint fsize[] = { (GLint)fragment_shader_code.size() };

        glShaderSource(vertex_shader_id, 1, vcode, vsize);
        glShaderSource(fragment_shader_id, 1, fcode, fsize);

        glCompileShader(vertex_shader_id);
        glCompileShader(fragment_shader_id);

        glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &succeeded);
        if (!succeeded) show_compilation_error(vertex_shader_id);

        glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &succeeded);
        if (!succeeded) show_compilation_error(fragment_shader_id);

        GLuint program_id = glCreateProgram();

        glAttachShader(program_id, vertex_shader_id);
        glAttachShader(program_id, fragment_shader_id);

        glLinkProgram(program_id);

        glGetProgramiv(program_id, GL_LINK_STATUS, &succeeded);
        if (!succeeded) show_linkage_error(program_id);

        glDeleteShader(vertex_shader_id);
        glDeleteShader(fragment_shader_id);

        return program_id;
    }

    void Scene::show_compilation_error(GLuint shader_id)
    {
        string info_log;
        GLint length;

        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);
        info_log.resize(length);

        glGetShaderInfoLog(shader_id, length, NULL, &info_log[0]);

        cerr << info_log << endl;
        assert(false);
    }

    void Scene::show_linkage_error(GLuint program_id)
    {
        string info_log;
        GLint length;

        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length);
        info_log.resize(length);

        glGetProgramInfoLog(program_id, length, NULL, &info_log[0]);

        cerr << info_log << endl;
        assert(false);
    }

}
