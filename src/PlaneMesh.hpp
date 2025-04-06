#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"

#include <iostream>
#include <GL/glew.h>

void checkGLError(const char* stmt, const char* fname, int line) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error " << err 
                  << " at " << fname << ":" << line 
                  << " for " << stmt << std::endl;
    }
}

#define GL_CHECK(stmt) do { \
    stmt;                   \
    checkGLError(#stmt, __FILE__, __LINE__); \
} while (0)

void checkShaderCompile(GLuint shader, const std::string& shaderName) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "ERROR::SHADER_COMPILATION_ERROR in " << shaderName << "\n" << infoLog << std::endl;
    }
}

void checkProgramLink(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "ERROR::PROGRAM_LINKING_ERROR\n" << infoLog << std::endl;
    }
}

class PlaneMesh {
	GLfloat min, max;
	glm::vec4 modelColor;

	std::vector<float> verts;
	std::vector<float> normals;
	std::vector<GLuint> indices;

	GLsizei numVerts, numIndices;

	GLuint vao;
	GLuint shaderProgramID;

	void planeMeshQuads(float min, float max, float stepsize) {

		// The following coordinate system works as if (min, 0, min) is the origin
		// And then builds up the mesh from that origin down (in z)
		// and then to the right (in x).
		// So, one "row" of the mesh's vertices have a fixed x and increasing z

		//manually create a first column of vertices
		float x = min;
		float y = 0;
		for (float z = min; z <= max; z += stepsize) {
			verts.push_back(x);
			verts.push_back(y);
			verts.push_back(z);
			normals.push_back(0);
			normals.push_back(1);
			normals.push_back(0);
		}

		for (float x = min+stepsize; x <= max; x += stepsize) {
			for (float z = min; z <= max; z += stepsize) {
				verts.push_back(x);
				verts.push_back(y);
				verts.push_back(z);
				normals.push_back(0);
				normals.push_back(1);
				normals.push_back(0);
			}
		}

		int nCols = (max-min)/stepsize + 1;
		int i = 0, j = 0;
		for (float x = min; x < max; x += stepsize) {
			j = 0;
			for (float z = min; z < max; z += stepsize) {
				indices.push_back(i*nCols + j);
				indices.push_back(i*nCols + j + 1);
				indices.push_back((i+1)*nCols + j + 1);
				indices.push_back((i+1)*nCols + j);
				++j;
			}
			++i;
		}
	}

public:

	PlaneMesh(float min, float max, float stepsize) {
		this->min = min;
		this->max = max;
		modelColor = glm::vec4(0, 1.0f, 1.0f, 1.0f);

		planeMeshQuads(min, max, stepsize);
		numVerts = verts.size()/3;
		numIndices = indices.size();
		
		// gen and fill buffers
		// create vao
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		// create vertex buffer
		GLuint vboVerts;
		glGenBuffers(1, &vboVerts);
		glBindBuffer(GL_ARRAY_BUFFER, vboVerts);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		glEnableVertexAttribArray(0);

		// create normal buffer
		GLuint vboNormals;
		glGenBuffers(1, &vboNormals);
		glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
		glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(
			1,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);

		// index buffer
		GLuint ebo;
		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

		// shaders and uniforms
		shaderProgramID = createShaderProgram(
			"shaders/vertex.glsl",
			"shaders/tess_control.glsl",
			"shaders/tess_eval.glsl",
			"shaders/geo.glsl",
			"shaders/fragment.glsl"
		);
		
		if (shaderProgramID == 0) {
			std::cerr << "Couldn't generate shader..." << std::endl;
			exit(1);
		}

		GL_CHECK(glUseProgram(shaderProgramID));
	}

	void draw(glm::vec3 lightPos, glm::mat4 V, glm::mat4 P) {
		// calculate MVP
		glm::mat4 M = glm::mat4(1.0f);  
		glm::mat4 MVP = P * V * M;
	
		//Bind some stuff
		GL_CHECK(glUseProgram(shaderProgramID));
		GL_CHECK(glBindVertexArray(vao));

		// set the mvp
		GLint mvpLocation = glGetUniformLocation(shaderProgramID, "MVP");
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(MVP));
		
		// set the tess levels
		GLint innerTessLocation = glGetUniformLocation(shaderProgramID, "innerTess");
		GLint outerTessLocation = glGetUniformLocation(shaderProgramID, "outerTess");
		glUniform1f(innerTessLocation, 16);
		glUniform1f(outerTessLocation, 16);

		// set the patch and draw
		GL_CHECK(glPatchParameteri(GL_PATCH_VERTICES, 4));
		GL_CHECK(glDrawElements(GL_PATCHES, numIndices, GL_UNSIGNED_INT, (void*)0));
	}
};
