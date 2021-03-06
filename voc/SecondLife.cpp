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

#include "SecondLife.h"
#include "imported/tinyobjloader/tiny_obj_loader.h"

CSecondLife::CSecondLife(CEmperorSystem *ces)
    : m_psSystem(ces),
    m_sCurrentMatrix(1.0)
{
    m_sCreationParams.width = 640;
    m_sCreationParams.height = 480;
}

CSecondLife::~CSecondLife()
{
}

void CSecondLife::init()
{
    m_sTimer.current = m_sTimer.getUpdatedTime(); 
    m_sTimer.previous = 0.0;

    struct SShaderDrop
    {
        std::string name;
        std::string vert;
        std::string frag;
    };

    SShaderDrop drops[] =
    {
        { "perspective", "./build/assets/shaders/flat-depth.vs", "./build/assets/shaders/flat-depth.fs" },
        { "font", "./build/assets/shaders/font.vs", "./build/assets/shaders/font.fs" }
    };

    for (size_t i = 0; i < sizeof(drops) / sizeof(drops[0]); i++)
    {
        bool validated = true;
        if (validated)
        {
            m_mShader[drops[i].name.c_str()] = SShader(drops[i].name.c_str());

            std::string vertShaderSource = drops[i].vert;
            std::string fragShaderSource = drops[i].frag;

            GLuint vertShader = helpers::createShader(GL_VERTEX_SHADER, vertShaderSource);
            GLuint fragShader = helpers::createShader(GL_FRAGMENT_SHADER, fragShaderSource);

            validated = validated && helpers::checkShader(vertShader, vertShaderSource);
            validated = validated && helpers::checkShader(fragShader, fragShaderSource);

            m_mShader[drops[i].name.c_str()].program = glCreateProgram();
            glAttachShader(m_mShader[drops[i].name.c_str()].program, vertShader);
            glAttachShader(m_mShader[drops[i].name.c_str()].program, fragShader);

            glDeleteShader(vertShader);
            glDeleteShader(fragShader);

            glBindAttribLocation(m_mShader[drops[i].name.c_str()].program, helpers::semantic::attr::POSITION, "position");
            glBindAttribLocation(m_mShader[drops[i].name.c_str()].program, helpers::semantic::attr::TEXCOORD, "texcoord");

            glLinkProgram(m_mShader[drops[i].name.c_str()].program);
            validated = helpers::checkProgram(m_mShader[drops[i].name.c_str()].program);

        }

        if (validated)
            m_mShader[drops[i].name.c_str()].uniforms = helpers::getActiveUniforms(m_mShader[drops[i].name.c_str()].program);
    }

    m_psSystem->getTextureManager()->load("./build/assets/textures/nz.jpg", 0, GL_BGR);
    m_psSystem->getTextureManager()->load("./build/assets/textures/nx.jpg", 1, GL_BGR);
    m_psSystem->getTextureManager()->load("./build/assets/textures/pz.jpg", 2, GL_BGR);
    m_psSystem->getTextureManager()->load("./build/assets/textures/px.jpg", 3, GL_BGR);
    m_psSystem->getTextureManager()->load("./build/assets/textures/py.jpg", 4, GL_BGR);
    m_psSystem->getTextureManager()->load("./build/assets/textures/ny.jpg", 5, GL_BGR);

    m_psSystem->getFontManager()->load("serif", "/usr/share/fonts/dejavu/DejaVuSerif.ttf");

    stagePerspectiveObjects();

    glViewport(0, 0, m_sCreationParams.width, m_sCreationParams.height);
    m_sMvp.constant.perspective = glm::perspective(45.0,
            double(m_sCreationParams.width / m_sCreationParams.height),
            0.001, 750.0);
    m_sMvp.constant.orthographic = glm::ortho(0.0, double(m_sCreationParams.width),
            double(m_sCreationParams.height), 0.0, -1.0, 1.0);

    m_sMvp.model = glm::mat4(1.0);
    m_sMvp.view = glm::lookAt( glm::vec3(0.0, 0.0, 3.0),
            glm::vec3(0.0, 0.0, -5.0),
            glm::vec3(0.0, 1.0, 0.0));

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glFrontFace(GL_CCW);

    glUseProgram(m_mShader["perspective"].program);
    glUniform4fv(m_mShader["perspective"].uniforms["diffuse"], 1, &glm::vec4(1.0, 0.5, 0.0, 1.0)[0]);
    glUseProgram(0);
}

void CSecondLife::destroy()
{
    std::vector<SMeshNode>::iterator it = m_vMesh.begin();
    for (; it != m_vMesh.end(); it++)
    {
        std::vector<SMesh>::iterator n = it->mesh.begin();
        for (; n != it->mesh.end(); n++)
        {
            glDeleteBuffers(SMesh::MAX, &n->buffers[0]);
            glDeleteVertexArrays(1, &n->object);
        }

        it->mesh.clear();
    }

    m_vMesh.clear();
    glDeleteProgram(m_mShader["perspective"].program);
    glDeleteProgram(m_mShader["font"].program);
}

void CSecondLife::update()
{
    m_sTimer.previous = m_sTimer.current;
    m_sTimer.current = m_sTimer.getUpdatedTime();
    m_sTimer.delta = m_sTimer.getDeltaTime();

    glm::mat4 view = glm::lookAt(m_sCamera.pos, (m_sCamera.pos + m_sCamera.eye), m_sCamera.up);

    if (m_psSystem->getEventHandler()->getSpectatorKey(CEventHandler::ESpecKey::E_SK_UP))
    {
        m_sCamera.pos.x += m_sCamera.eye.x * 3.1415 * m_sTimer.delta;
        m_sCamera.pos.z += m_sCamera.eye.z * 3.1415 * m_sTimer.delta;
        m_sCamera.pos.y += m_sCamera.eye.y * 3.1415 * m_sTimer.delta;
    }
    else if (m_psSystem->getEventHandler()->getSpectatorKey(CEventHandler::ESpecKey::E_SK_DN))
    {
        m_sCamera.pos.x -= m_sCamera.eye.x * 3.1415 * m_sTimer.delta;
        m_sCamera.pos.z -= m_sCamera.eye.z * 3.1415 * m_sTimer.delta;
        m_sCamera.pos.y -= m_sCamera.eye.y * 3.1415 * m_sTimer.delta;
    }
    else if (m_psSystem->getEventHandler()->getSpectatorKey(CEventHandler::ESpecKey::E_SK_LT))
    {
        m_sCamera.angle -= 3.1415 * m_sTimer.delta;
        m_sCamera.eye.x = sin(m_sCamera.angle);
        m_sCamera.eye.z = -cos(m_sCamera.angle);
    }
    else if (m_psSystem->getEventHandler()->getSpectatorKey(CEventHandler::ESpecKey::E_SK_RT))
    {
        m_sCamera.angle += 3.1415 * m_sTimer.delta;
        m_sCamera.eye.x = sin(m_sCamera.angle);
        m_sCamera.eye.z = -cos(m_sCamera.angle);
    }

    if (m_psSystem->getEventHandler()->getSpectatorKey(CEventHandler::ESpecKey::E_SK_PGUP))
    {
        m_sCamera.look += 3.1415 * m_sTimer.delta;
        m_sCamera.eye.y = sin(m_sCamera.look);
    }
    else if (m_psSystem->getEventHandler()->getSpectatorKey(CEventHandler::ESpecKey::E_SK_PGDN))
    {
        m_sCamera.look -= 3.1415 * m_sTimer.delta;
        m_sCamera.eye.y = sin(m_sCamera.look);
    }

    m_sCurrentMatrix = m_sMvp.getModelView(view);

    glClearColor(0.0, 0.7, 0.7, 1.0);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_mShader["perspective"].program);
    m_sMvp.setProjection(m_mShader["perspective"].uniforms["projection"], SModelViewProjection::PERSPECTIVE);
    glUniformMatrix4fv(m_mShader["perspective"].uniforms["modelview"], 1, GL_FALSE, &m_sMvp.getModelView(view)[0][0]);
    push();
        //! recursive rendering
        glUniformMatrix4fv(m_mShader["perspective"].uniforms["modelview"], 1, GL_FALSE, &m_sCurrentMatrix[0][0]);
        std::vector<SMeshNode>::iterator it = m_vMesh.begin();
        for (; it != m_vMesh.end(); it++)
        {
            if (it->visible)
            {
                push();
                    glUniformMatrix4fv(m_mShader["perspective"].uniforms["modelview"], 1, GL_FALSE, &m_sCurrentMatrix[0][0]);
                    std::vector<SMesh>::iterator n = it->mesh.begin();
                    for (; n != it->mesh.end(); n++)
                    {
                        if (n->options.texture)
                        {
                            m_psSystem->getTextureManager()->bindTexture(n->properties.texture);
                            glUniform1i(m_mShader["perspective"].uniforms["textured"], GL_TRUE);
                        }

                        glBindVertexArray(n->object);
                            glDrawElements(n->properties.type, n->properties.count, GL_UNSIGNED_INT, 0);
                        glBindVertexArray(0);

                        if (n->options.texture)
                            glUniform1i(m_mShader["perspective"].uniforms["textured"], GL_FALSE);
                    }
                pop();
            }
        }
    pop();
    glUseProgram(0);

    glUseProgram(m_mShader["font"].program);
    m_sMvp.setProjection(m_mShader["font"].uniforms["projection"], SModelViewProjection::ORTHOGRAPHIC);
    push();
        m_psSystem->getFontManager()->setFontType("serif");
        m_psSystem->getFontManager()->setPixelSize(m_mShader["font"].uniforms["offset"], 48);
        m_psSystem->getFontManager()->write("Hello, World!", glm::vec2(10.0));
    pop();
    glUseProgram(0);
}

void CSecondLife::push()
{
    m_vStack.push(m_sCurrentMatrix);
}

void CSecondLife::pop()
{
    m_sCurrentMatrix = m_vStack.top();
    m_vStack.pop();
}

void CSecondLife::loadWaveObjFile(char const *file, char const *path, bool visible, float offset)
{
    std::vector<tinyobj::shape_t> shapes;
    std::string err = tinyobj::LoadObj(shapes, file, path);

    if (!err.empty())
        fprintf(stderr, "[ERR] Scene Error: Unable to load obj file.");

    SMeshNode meshNode;
    std::vector<SMesh> holder;
    for (int i = 0; i < shapes.size(); i++)
    {
        SMesh mesh;

        GLsizei elementCount;
        GLsizeiptr elementSize;
        std::vector<glm::uint32> elementData;

        GLsizei vertexCount;
        GLsizeiptr vertexSize;
        std::vector<glm::vec3> vertexData;

        assert((shapes[i].mesh.positions.size() % 3) == 0);
        for (int v = 0; v < shapes[i].mesh.positions.size() / 3; v++)
        {
            vertexData.push_back(glm::vec3(
                shapes[i].mesh.positions[3 * v + 0] - offset,
                shapes[i].mesh.positions[3 * v + 1] - offset,
                shapes[i].mesh.positions[3 * v + 2] - offset));
        }

        vertexCount = vertexData.size();
        vertexSize = vertexCount * sizeof(glm::vec3);

        for (int f = 0; f < shapes[i].mesh.indices.size(); f++)
            elementData.push_back(shapes[i].mesh.indices[f]);

        elementCount = elementData.size();
        elementSize = elementCount * sizeof(glm::uint32);
        mesh.properties.count = elementCount;

        glGenVertexArrays(1, &mesh.object);

        glGenBuffers(SMesh::MAX, &mesh.buffers[0]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.buffers[SMesh::VERTEX]);
        glBufferData(GL_ARRAY_BUFFER, vertexSize, &vertexData[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.buffers[SMesh::ELEMENT]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementSize, &elementData[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(mesh.object);
            glBindBuffer(GL_ARRAY_BUFFER, mesh.buffers[SMesh::VERTEX]);
            glVertexAttribPointer(helpers::semantic::attr::POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.buffers[SMesh::ELEMENT]);
            glEnableVertexAttribArray(helpers::semantic::attr::POSITION);
        glBindVertexArray(0);

        mesh.properties.type = GL_TRIANGLES;
        holder.push_back(mesh);
    }

    meshNode.name = file;
    meshNode.mesh = holder;
    meshNode.visible = visible;
    m_vMesh.push_back(meshNode);
}

void CSecondLife::stagePerspectiveObjects()
{
    //! populate grid object
    {
        SMeshNode meshNode;
        SMesh mesh;
        std::vector<SMesh> holder;

        GLsizei elementCount;
        GLsizeiptr elementSize;
        std::vector<glm::uint32> elementData;

        GLsizei vertexCount;
        GLsizeiptr vertexSize;
        std::vector<glm::vec3> vertexData;

        float extent = 30.0, step = 1.0, h = -0.6;
        for (int line = -extent; line <= extent; line += step)
        {
            vertexData.push_back(glm::vec3(line, h, extent));
            vertexData.push_back(glm::vec3(line, h, -extent));

            vertexData.push_back(glm::vec3(extent, h, line));
            vertexData.push_back(glm::vec3(-extent, h, line));
        }

        vertexCount = vertexData.size();
        vertexSize = vertexCount * sizeof(glm::vec3);

        elementCount = vertexCount;
        elementSize = elementCount * sizeof(glm::uint32);
        for (int i = 0; i < elementCount; i++)
            elementData.push_back(i);

        mesh.properties.count = elementCount;

        glGenVertexArrays(1, &mesh.object);

        glGenBuffers(SMesh::MAX, &mesh.buffers[0]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.buffers[SMesh::VERTEX]);
        glBufferData(GL_ARRAY_BUFFER, vertexSize, &vertexData[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.buffers[SMesh::ELEMENT]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementSize, &elementData[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(mesh.object);
            glBindBuffer(GL_ARRAY_BUFFER, mesh.buffers[SMesh::VERTEX]);
            glVertexAttribPointer(helpers::semantic::attr::POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.buffers[SMesh::ELEMENT]);
            glEnableVertexAttribArray(helpers::semantic::attr::POSITION);
        glBindVertexArray(0);

        mesh.properties.type = GL_LINES;
        holder.push_back(mesh);

        meshNode.name = "grid";
        meshNode.mesh = holder;
        meshNode.visible = true;
        m_vMesh.push_back(meshNode);
    }

    //! populate skybox object
    {
        float scale = 250.0;
        SMeshNode meshNode;
        helpers::SVertv3v2 vertexData[6][4] =
        {
            {
                //! front
                helpers::SVertv3v2(glm::vec3(-scale, -scale, -scale), glm::vec2(0, 0)),
                helpers::SVertv3v2(glm::vec3( scale, -scale, -scale), glm::vec2(1, 0)),
                helpers::SVertv3v2(glm::vec3( scale,  scale, -scale), glm::vec2(1, 1)),
                helpers::SVertv3v2(glm::vec3(-scale,  scale, -scale), glm::vec2(0, 1))
            },
            {
                //! left
                helpers::SVertv3v2(glm::vec3( scale, -scale, -scale), glm::vec2(0, 0)),
                helpers::SVertv3v2(glm::vec3( scale, -scale,  scale), glm::vec2(1, 0)),
                helpers::SVertv3v2(glm::vec3( scale,  scale,  scale), glm::vec2(1, 1)),
                helpers::SVertv3v2(glm::vec3( scale,  scale, -scale), glm::vec2(0, 1))
            },
            {
                //! back
                helpers::SVertv3v2(glm::vec3( scale, -scale,  scale), glm::vec2(0, 0)),
                helpers::SVertv3v2(glm::vec3(-scale, -scale,  scale), glm::vec2(1, 0)),
                helpers::SVertv3v2(glm::vec3(-scale,  scale,  scale), glm::vec2(1, 1)),
                helpers::SVertv3v2(glm::vec3( scale,  scale,  scale), glm::vec2(0, 1))
            },
            {
                //! right
                helpers::SVertv3v2(glm::vec3(-scale, -scale,  scale), glm::vec2(0, 0)),
                helpers::SVertv3v2(glm::vec3(-scale, -scale, -scale), glm::vec2(1, 0)),
                helpers::SVertv3v2(glm::vec3(-scale,  scale, -scale), glm::vec2(1, 1)),
                helpers::SVertv3v2(glm::vec3(-scale,  scale,  scale), glm::vec2(0, 1))
            },
            {
                //! top
                helpers::SVertv3v2(glm::vec3( scale,  scale, -scale), glm::vec2(0, 1)),
                helpers::SVertv3v2(glm::vec3( scale,  scale,  scale), glm::vec2(0, 0)),
                helpers::SVertv3v2(glm::vec3(-scale,  scale,  scale), glm::vec2(1, 0)),
                helpers::SVertv3v2(glm::vec3(-scale,  scale, -scale), glm::vec2(1, 1))
            },
            {
                //! bottom
                helpers::SVertv3v2(glm::vec3( scale, -scale,  scale), glm::vec2(0, 1)),
                helpers::SVertv3v2(glm::vec3( scale, -scale, -scale), glm::vec2(0, 0)),
                helpers::SVertv3v2(glm::vec3(-scale, -scale, -scale), glm::vec2(1, 0)),
                helpers::SVertv3v2(glm::vec3(-scale, -scale,  scale), glm::vec2(1, 1))
            }
        };

        std::vector<SMesh> holder;
        for (int i = 0;  i < 6; i++)
        {
            SMesh mesh;

            GLsizei const vertexCount = 4;
            GLsizeiptr const vertexSize = vertexCount * sizeof(helpers::SVertv3v2);
                    
            GLsizei const elementCount = 4;
            GLsizeiptr const elementSize = elementCount * sizeof(glm::uint32);
            glm::uint32 elementData[] = { 0, 1, 2, 3 };

            mesh.properties.count = elementCount;

            glGenVertexArrays(1, &mesh.object);

            glGenBuffers(SMesh::MAX, &mesh.buffers[0]);
            glBindBuffer(GL_ARRAY_BUFFER, mesh.buffers[SMesh::VERTEX]);
            glBufferData(GL_ARRAY_BUFFER, vertexSize, &vertexData[i][0], GL_STATIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.buffers[SMesh::ELEMENT]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementSize, elementData, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            glBindVertexArray(mesh.object);
                glBindBuffer(GL_ARRAY_BUFFER, mesh.buffers[SMesh::VERTEX]);
                glVertexAttribPointer(helpers::semantic::attr::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(helpers::SVertv3v2), BUFFER_OFFSET(0));
                glVertexAttribPointer(helpers::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(helpers::SVertv3v2), BUFFER_OFFSET(sizeof(glm::vec3)));
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.buffers[SMesh::ELEMENT]);
                glEnableVertexAttribArray(helpers::semantic::attr::POSITION);
                glEnableVertexAttribArray(helpers::semantic::attr::TEXCOORD);
            glBindVertexArray(0);

            mesh.options.texture = true;
            mesh.properties.texture = i;
            mesh.properties.type = GL_TRIANGLE_FAN;
            holder.push_back(mesh);
        }

        meshNode.name = "skybox";
        meshNode.mesh = holder;
        meshNode.visible = true;
        m_vMesh.push_back(meshNode);
    }

    //! populate cube object
    {
        SMeshNode meshNode;
        SMesh mesh;
        std::vector<SMesh> holder;

        float scale = 1.0 * 0.5;
        GLsizei const vertexCount = 12;
        GLsizeiptr const vertexSize = vertexCount * sizeof(helpers::SVertv3v2);
        helpers::SVertv3v2 vertexData[12] = 
        {
            helpers::SVertv3v2(glm::vec3(-scale, -scale, -scale), glm::vec2(0, 1)), 
            helpers::SVertv3v2(glm::vec3( scale, -scale, -scale), glm::vec2(1, 1)),
            helpers::SVertv3v2(glm::vec3( scale,  scale, -scale), glm::vec2(1, 0)),
            helpers::SVertv3v2(glm::vec3(-scale,  scale, -scale), glm::vec2(0, 0)),
            helpers::SVertv3v2(glm::vec3( scale, -scale,  scale), glm::vec2(0, 1)),
            helpers::SVertv3v2(glm::vec3( scale,  scale,  scale), glm::vec2(0, 0)),
            helpers::SVertv3v2(glm::vec3(-scale,  scale,  scale), glm::vec2(1, 0)),
            helpers::SVertv3v2(glm::vec3(-scale, -scale,  scale), glm::vec2(1, 1)),
            helpers::SVertv3v2(glm::vec3(-scale,  scale,  scale), glm::vec2(0, 1)),
            helpers::SVertv3v2(glm::vec3(-scale,  scale, -scale), glm::vec2(1, 1)),
            helpers::SVertv3v2(glm::vec3( scale, -scale,  scale), glm::vec2(1, 0)),
            helpers::SVertv3v2(glm::vec3( scale, -scale, -scale), glm::vec2(0, 0))
        };

        GLsizei const elementCount = 36;
        GLsizeiptr const elementSize = elementCount * sizeof(glm::uint32);
        mesh.properties.count = elementCount;

        glm::uint32 elementData[36] =
        {
            0, 2, 1,
            0, 3, 2,
            1, 5, 4,
            1, 2, 5,
            4, 6, 7,
            4, 5, 6,
            7, 3, 0,
            7, 6, 3,
            9, 5, 2,
            9, 8, 5,
            0, 11, 10,
            0, 10, 7
        };

        glGenVertexArrays(1, &mesh.object);

        glGenBuffers(SMesh::MAX, &mesh.buffers[0]);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.buffers[SMesh::VERTEX]);
        glBufferData(GL_ARRAY_BUFFER, vertexSize, vertexData, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.buffers[SMesh::ELEMENT]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementSize, elementData, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindVertexArray(mesh.object);
            glBindBuffer(GL_ARRAY_BUFFER, mesh.buffers[SMesh::VERTEX]);
            glVertexAttribPointer(helpers::semantic::attr::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(helpers::SVertv3v2), BUFFER_OFFSET(0));
            glVertexAttribPointer(helpers::semantic::attr::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(helpers::SVertv3v2), BUFFER_OFFSET(sizeof(glm::vec3)));
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.buffers[SMesh::ELEMENT]);
            glEnableVertexAttribArray(helpers::semantic::attr::POSITION);
            glEnableVertexAttribArray(helpers::semantic::attr::TEXCOORD);
        glBindVertexArray(0);

        mesh.properties.type = GL_TRIANGLES;
        holder.push_back(mesh);

        meshNode.name = "cube";
        meshNode.mesh = holder;
        meshNode.visible = true;
        m_vMesh.push_back(meshNode);
    }
}

