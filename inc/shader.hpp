#ifndef SHADER_HPP
#define SHADER_HPP
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <string>
#include "utility.hpp"
#include "glew.h"
#define MAX_SHADERS 5

class Shader
{
public:
	Shader(std::string filename);
	Shader(const Shader& copy);
	Shader(Shader&& move);
	Shader& operator=(const Shader& rhs) = delete;
	~Shader();
	
	bool hasVertexShader() const;
	bool hasTessellationControlShader() const;
	bool hasTessellationEvaluationShader() const;
	bool hasGeometryShader() const;
	bool hasFragmentShader() const;
	
	GLuint getProgramHandle() const;
	void bind() const;
	void update(const std::array<float, 16>& modelData, const std::array<float, 16>& viewData, const std::array<float, 16>& projectionData, float parallaxMapScale = 0.04f, float parallaxMapOffset = -0.5f) const;
private:
	static std::string loadShader(const std::string& filename);
	static void checkShaderError(GLuint shader, GLuint flag, bool isProgram, std::string errorMessage);
	static GLuint createShader(std::string source, GLenum shaderType);
	enum class UniformTypes : unsigned int
	{
		MODEL = 0,
		VIEW = 1,
		PROJECTION = 2,
		PARALLAX_MAP_SCALE = 3,
		PARALLAX_MAP_BIAS = 4,
		NUM_UNIFORMS = 5
	};
	std::string filename;
	GLuint programHandle;
	GLuint shaders[MAX_SHADERS];
	GLuint uniforms[static_cast<unsigned int>(UniformTypes::NUM_UNIFORMS)];
};

#endif