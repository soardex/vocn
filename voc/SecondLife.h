/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Fitz Abucay, 2014
 */

#ifndef SECONDLIFE_H
#define SECONDLIFE_H

#include "Commons.h"
#include "EmperorSystem.h"

#include "utils/Helpers.h"

class CSecondLife
{
public:
    explicit CSecondLife(CEmperorSystem *ces);
    ~CSecondLife();

    void init();
    void destroy();
    void update();

protected:
    void loadWaveObjFile(char const *file, char const *path, bool visible, float offset = 0.0);

    void push();
    void pop();

    void stagePerspectiveObjects();

private:
    CEmperorSystem *m_psSystem;

    struct SCreationParams
    {
        int width;
        int height;
    } m_sCreationParams;

    struct SModelViewProjection
    {
        glm::mat4 projection;
        glm::mat4 view;
        glm::mat4 model;

        struct
        {
            glm::mat4 perspective;
            glm::mat4 orthographic;
        } constant;

        enum EProjection
        {
            PERSPECTIVE,
            ORTHOGRAPHIC
        };

        void setProjection(GLint location, EProjection e)
        {
            if (e == PERSPECTIVE)
                projection = constant.perspective;
            else
                projection = constant.orthographic;

            glUniformMatrix4fv(location, 1, GL_FALSE, &projection[0][0]);
        }

        glm::mat4 getModelView() const { return view * model; }
        glm::mat4 getModelView(glm::mat4 const &v) { return v * model; }
    } m_sMvp;

    struct SCamera
    {
        glm::vec3 pos;
        glm::vec3 eye;
        glm::vec3 up;

        float angle;
        float look;

        SCamera() : pos(1.0), eye(0.0), up(0.0), angle(0.0), look(0.0)
        {
            pos = glm::vec3(0.0, 0.0, 5.0);
            eye = glm::vec3(0.0, 0.0, -1.0);
            up = glm::vec3(0.0, 1.0, 0.0);
        }
    } m_sCamera;

    struct STimer
    {
        float current;
        float previous;
        float delta;

        STimer()
        {
            current = 0.0;
            previous = 0.0;
            delta = 0.0;
        }

        float getUpdatedTime() const { return SDL_GetTicks(); } 
        float getDeltaTime() const { return (current - previous) / 1000.0; }
    } m_sTimer;

    struct SShader
    {
        std::string name;
        GLuint program;

        std::map<std::string, GLint> attribs;
        std::map<std::string, GLint> uniforms;

        SShader(char const *name = "unnamed")
        {
            this->name = name;
            program = 0;
        }
    };

    struct SMesh
    {
        enum
        {
            VERTEX,
            ELEMENT,

            MAX
        };

        GLuint object;
        std::vector<GLuint> buffers;

        struct
        {
            GLuint texture;
            GLenum type;
            GLint count;
        } properties;


        struct
        {
            glm::vec3 diffuse;
            glm::vec3 ambient;
            glm::vec3 emission;
            glm::vec3 specular;

            std::string diffuse_tex;
            std::string ambient_tex;
            std::string specular_tex;
            std::string normal_tex;

            float shininess;
            float ior;
        } materials;

        struct
        {
            bool texture;
            bool light;
            bool blend;
            bool cull;
        } options;

        SMesh() : buffers(MAX)
        {
            object = 0;
            properties.texture = 0;
            properties.type = 0;
            properties.count = 0;

            options.texture = false;
            options.light = false;
            options.blend = false;
            options.cull = false;
        }
    };

    struct SMeshNode
    {
        std::string name;
        bool visible;
        unsigned int priority;

        std::vector<SMesh> mesh;

        SMeshNode()
        {
            name = "unnamed";
            visible = false;
            priority = 0;
        }
    };

    std::map<std::string, SShader> m_mShader;
    std::vector<SMeshNode> m_vMesh;

    std::stack<glm::mat4, std::vector<glm::mat4> > m_vStack;
    glm::mat4 m_sCurrentMatrix;
};

#endif /* end of include guard: SECONDLIFE_H */

