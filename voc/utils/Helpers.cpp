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


#include "Helpers.h"

namespace helpers
{
    std::string loadFile(std::string const &file)
    {
        std::string result;

        std::ifstream stream(file.c_str());
        if (!stream.is_open())
            return result;

        stream.seekg(0, std::ios::end);
        result.reserve(stream.tellg());
        stream.seekg(0, std::ios::beg);

        result.assign((std::istreambuf_iterator<char>(stream)),
                std::istreambuf_iterator<char>());

        return result;
    }

    GLuint createShader(GLenum type, std::string const &source)
    {
        GLuint name = 0;

        if (!source.empty())
        {
            std::string content = helpers::loadFile(source);
            char const *ptr = content.c_str();
            name = glCreateShader(type);
            glShaderSource(name, 1, &ptr, NULL);
            glCompileShader(name);
        }

        return name;
    }

    bool checkShader(GLuint shader, std::string const &file)
    {
        if (!shader)
            return false;

        GLint result = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

        fprintf(stdout, "[INF] Compiling shader: %s\n", file.c_str());

        int infoLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);
        if (infoLength > 0)
        {
            std::vector<char> buffer(infoLength);
            glGetShaderInfoLog(shader, infoLength, NULL, &buffer[0]);
            fprintf(stdout, "%s\n", &buffer[0]);
        }

        return result == GL_TRUE;
    }

    bool checkProgram(GLuint program)
    {
        if (!program)
            return false;

        GLint result = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &result);

        fprintf(stdout, "[INF] Linking program...\n");

        int infoLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLength);
        if (infoLength > 0)
        {
            std::vector<char> buffer(std::max(infoLength, int(1)));
            glGetProgramInfoLog(program, infoLength, NULL, &buffer[0]);
            fprintf(stdout, "%s\n", &buffer[0]);
        }

        return result == GL_TRUE;
    }

    bool validateProgram(GLuint program)
    {
        if (!program)
            return false;

        glValidateProgram(program);

        GLint result = GL_FALSE;
        glGetProgramiv(program, GL_VALIDATE_STATUS, &result);

        if (result == GL_FALSE)
        {
            fprintf(stdout, "[INF] Validate program\n");

            int infoLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLength);
            
            if (infoLength > 0)
            {
                std::vector<char> buffer(infoLength);
                glGetProgramInfoLog(program, infoLength, NULL, &buffer[0]);
                fprintf(stdout, "%s\n", &buffer[0]);
            }
        }

        return result == GL_TRUE;
    }

    bool checkGLVersion()
    {
        GLint majorVersion = 0;
        GLint minorVersion = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

        printf("[INF] OpenGL Version %d.%d\n", majorVersion, minorVersion);

        return true;
    }

    bool checkExtension(char const *ext)
    {
        GLint extensionCount = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &extensionCount);
        for (GLint i = 0; i < extensionCount; i++)
            if (std::string((char const *)glGetStringi(GL_EXTENSIONS, i)) == std::string(ext))
                return true;

        printf("[ERR] Failed to find extension: \"%s\"\n", ext);
        return false;
    }

    std::map<std::string, GLint> getActiveUniforms(GLuint program)
    {
        std::map<std::string, GLint> uniforms;

        int total = -1;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &total);
        
        int bufsize;
        glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &bufsize);
        for (int i = 0; i < total; i++)
        {
            int length = -1, size = -1;
            GLenum type = GL_ZERO;

            std::vector<char> name(bufsize);
            glGetActiveUniform(program, i, bufsize, &length, &size, &type, &name[0]);
            uniforms[&name[0]] = glGetUniformLocation(program, &name[0]);
        }

        return uniforms;
    }

    std::map<std::string, GLint> getActiveAttributes(GLuint program)
    {
        std::map<std::string, GLint> attrib;

        int total = -1;
        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &total);
        
        int bufsize;
        glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &bufsize);
        for (int i = 0; i < total; i++)
        {
            int length = -1, size = -1;
            GLenum type = GL_ZERO;

            std::vector<char> name(bufsize);
            glGetActiveAttrib(program, i, bufsize, &length, &size, &type, &name[0]);
            attrib[&name[0]] = glGetAttribLocation(program, &name[0]);
        }

        return attrib;
    }
}

