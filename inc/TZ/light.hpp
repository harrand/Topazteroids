#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "vector.hpp"
#include "graphics.hpp"

namespace tz
{
	namespace graphics
	{
		constexpr std::size_t light_number_of_uniforms = 5;
		constexpr std::size_t maximum_lights = 8;
		constexpr Vector3F light_default_colour(std::array<float, 3>({1.0f, 1.0f, 1.0f}));
		constexpr float light_default_power = 1.0f;
		constexpr float light_default_diffuse_component = 1.0f;
		constexpr float light_default_specular_component = 10.0f;
	}
}

/*
	Very barebones implementation of a light. Directional/Spot-Light logic, unlike other game engines, is performed in GLSL, not in C++. Hence why there is no SpotLight class, for example.
*/
class Light
{
public:
	Light(Vector3F position = Vector3F(), Vector3F colour = tz::graphics::light_default_colour, float power = tz::graphics::light_default_power, float diffuse_component = tz::graphics::light_default_diffuse_component, float specular_component = tz::graphics::light_default_specular_component);
	Light(const Light& copy) = default;
	Light(Light&& move) = default;
	~Light() = default;
	Light& operator=(const Light& rhs) = default;
	
	std::array<GLint, tz::graphics::light_number_of_uniforms> get_uniforms(GLuint shader_program_handle, std::size_t light_uniform_index) const;
	bool operator==(const Light& rhs) const;

	Vector3F position;
	Vector3F colour;
	float power;
	float diffuse_component;
	float specular_component;
};

#endif