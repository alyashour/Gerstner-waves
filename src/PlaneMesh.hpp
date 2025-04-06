#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.hpp"
#include "LoadBMP.hpp"

#include <iostream>
#include <GL/glew.h>

void checkGLError(const char *stmt, const char *fname, int line)
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		std::cerr << "OpenGL error " << err
				  << " at " << fname << ":" << line
				  << " for " << stmt << std::endl;
	}
}

#define GL_CHECK(stmt)                           \
	do                                           \
	{                                            \
		stmt;                                    \
		checkGLError(#stmt, __FILE__, __LINE__); \
	} while (0)

void checkShaderCompile(GLuint shader, const std::string &shaderName)
{
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
		std::cerr << "ERROR::SHADER_COMPILATION_ERROR in " << shaderName << "\n"
				  << infoLog << std::endl;
	}
}

void checkProgramLink(GLuint program)
{
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
		std::cerr << "ERROR::PROGRAM_LINKING_ERROR\n"
				  << infoLog << std::endl;
	}
}

// Example function to load a BMP file and create an OpenGL texture.
GLuint loadTextureFromBMP(const char *imagePath)
{
	// Variables to hold image data and dimensions.
	unsigned char *data = nullptr;
	unsigned int width, height;

	// Load BMP data from the file.
	loadBMP(imagePath, &data, &width, &height);

	if (!data)
	{
		std::cerr << "Failed to load BMP: " << imagePath << std::endl;
		return 0;
	}

	// Generate a texture ID.
	GLuint textureID;
	glGenTextures(1, &textureID);

	// Bind the texture so we can set parameters and upload data.
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Set texture parameters: wrapping mode and filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// BMP files typically store pixel data in BGR format.
	// Upload the texture data to the GPU.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// Generate mipmaps.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Unbind the texture.
	glBindTexture(GL_TEXTURE_2D, 0);

	// Free the BMP data now that it has been uploaded to the GPU.
	delete[] data;

	return textureID;
}

class PlaneMesh
{
	GLfloat min, max;
	glm::vec4 modelColor;

	std::vector<float> verts;
	std::vector<float> normals;
	std::vector<GLuint> indices;

	GLsizei numVerts, numIndices;

	// buffer
	GLuint vao;
	GLuint shaderProgramID;

	// texture
	GLuint distextID, waterTextureID;

	void planeMeshQuads(float min, float max, float stepsize)
	{

		// The following coordinate system works as if (min, 0, min) is the origin
		// And then builds up the mesh from that origin down (in z)
		// and then to the right (in x).
		// So, one "row" of the mesh's vertices have a fixed x and increasing z

		// manually create a first column of vertices
		float x = min;
		float y = 0;
		for (float z = min; z <= max; z += stepsize)
		{
			verts.push_back(x);
			verts.push_back(y);
			verts.push_back(z);
			normals.push_back(0);
			normals.push_back(1);
			normals.push_back(0);
		}

		for (float x = min + stepsize; x <= max; x += stepsize)
		{
			for (float z = min; z <= max; z += stepsize)
			{
				verts.push_back(x);
				verts.push_back(y);
				verts.push_back(z);
				normals.push_back(0);
				normals.push_back(1);
				normals.push_back(0);
			}
		}

		int nCols = (max - min) / stepsize + 1;
		int i = 0, j = 0;
		for (float x = min; x < max; x += stepsize)
		{
			j = 0;
			for (float z = min; z < max; z += stepsize)
			{
				indices.push_back(i * nCols + j);
				indices.push_back(i * nCols + j + 1);
				indices.push_back((i + 1) * nCols + j + 1);
				indices.push_back((i + 1) * nCols + j);
				++j;
			}
			++i;
		}
	}

public:
	PlaneMesh(float min, float max, float stepsize)
	{
		this->min = min;
		this->max = max;
		modelColor = glm::vec4(0.6f, 0.9f, 1.0f, 1.0f);

		planeMeshQuads(min, max, stepsize);
		numVerts = verts.size() / 3;
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
			(void *)0);
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
			(void *)0);

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
			"shaders/fragment.glsl");

		if (shaderProgramID == 0)
		{
			std::cerr << "Couldn't generate shader..." << std::endl;
			exit(1);
		}

		// generate texture
		distextID = loadTextureFromBMP("assets/displacement-map1.bmp");
        waterTextureID = loadTextureFromBMP("assets/water.bmp");

		GL_CHECK(glUseProgram(shaderProgramID));
	}

	void draw(glm::vec3 lightPos, glm::mat4 V, glm::mat4 P)
	{
		// calculate MVP
		glm::mat4 M = glm::mat4(1.0f);
		glm::mat4 MVP = P * V * M;

		// Bind some stuff
		GL_CHECK(glUseProgram(shaderProgramID));
		GL_CHECK(glBindVertexArray(vao));

		// set the mvp
		GLint mvpLocation = glGetUniformLocation(shaderProgramID, "MVP");
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(MVP));

		// set the tess levels
		GLint innerTessLocation = glGetUniformLocation(shaderProgramID, "innerTess");
		GLint outerTessLocation = glGetUniformLocation(shaderProgramID, "outerTess");
		glUniform1f(innerTessLocation, 64);
		glUniform1f(outerTessLocation, 64);

		// set the time
		GLint timeLocation = glGetUniformLocation(shaderProgramID, "time");
		glUniform1f(timeLocation, (float)glfwGetTime());

		// Set up displacement texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, distextID);
        GLint distextSamplerLocation = glGetUniformLocation(shaderProgramID, "distext");
        glUniform1i(distextSamplerLocation, 0);

        // Set up water texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, waterTextureID);
        GLint waterTexLocation = glGetUniformLocation(shaderProgramID, "waterTexture");
        glUniform1i(waterTexLocation, 1);

		// Compute view position from the view matrix.
		glm::mat4 invV = glm::inverse(V);
		glm::vec3 viewPos = glm::vec3(invV[3]);

		// Set light position uniform.
		GLint lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
		glUniform3f(lightPosLocation, lightPos.x, lightPos.y, lightPos.z);

		// Set view position uniform.
		GLint viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos");
		glUniform3f(viewPosLocation, viewPos.x, viewPos.y, viewPos.z);

		// Set object color uniform.
		glm::vec4 objectColor(modelColor.x, modelColor.y, modelColor.z, 1.0f); // using PlaneMesh::modelColor
		GLint objectColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
		glUniform4f(objectColorLocation, objectColor.x, objectColor.y, objectColor.z, objectColor.w);

		// Set texScale uniform.
		GLint texScaleLocation = glGetUniformLocation(shaderProgramID, "texScale");
		glUniform1f(texScaleLocation, 50.0f);
		
		// Set texOffset uniform.
		GLint texOffsetLocation = glGetUniformLocation(shaderProgramID, "texOffset");
		glUniform2f(texOffsetLocation, 0.0f, 0.0f);

		// set the patch and draw
		GL_CHECK(glPatchParameteri(GL_PATCH_VERTICES, 4));
		GL_CHECK(glDrawElements(GL_PATCHES, numIndices, GL_UNSIGNED_INT, (void *)0));
	}
};
