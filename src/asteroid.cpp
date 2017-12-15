#include "asteroid.hpp"

Asteroid::Asteroid(const Engine& engine, Vector3F position, Vector3F rotation, Vector3F scale): EntityObject(tz::graphics::find_mesh("../../../res/runtime/models/sphere.obj", engine.get_meshes()), {{tz::graphics::TextureType::TEXTURE, nullptr}, {tz::graphics::TextureType::NORMAL_MAP, nullptr}, {tz::graphics::TextureType::PARALLAX_MAP, nullptr}, {tz::graphics::TextureType::DISPLACEMENT_MAP, nullptr}}, tz::physics::default_mass, position, rotation, scale)
{
	switch(tz::util::random<int>() % 3)
	{
		case 0:
			this->textures[tz::graphics::TextureType::TEXTURE] = Texture::get_from_link<Texture>("../../../res/runtime/textures/lava.jpg", engine.get_textures());
			break;
		case 1:
			this->textures[tz::graphics::TextureType::TEXTURE] = Texture::get_from_link<Texture>("../../../res/runtime/textures/sand.jpg", engine.get_textures());
			break;
		case 2:
			this->textures[tz::graphics::TextureType::TEXTURE] = Texture::get_from_link<Texture>("../../../res/runtime/textures/metal.jpg", engine.get_textures());
			break;
		default:
			this->textures[tz::graphics::TextureType::TEXTURE] = Texture::get_from_link<Texture>("../../../res/runtime/textures/undefined.jpg", engine.get_textures());
			break;
	}
}

BoundingSphere Asteroid::boundary() const
{
	return tz::physics::bound_sphere(*this);
}