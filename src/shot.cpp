#include "shot.hpp"

Shot::Shot(const Engine& engine, Vector3F position, Vector3F rotation, Vector3F scale): EntityObject(tz::graphics::find_mesh("../../../res/runtime/models/sphere.obj", engine.get_meshes()), {{tz::graphics::TextureType::TEXTURE, Texture::get_from_link<Texture>("../../../res/runtime/textures/torpedo.png", engine.get_textures())}, {tz::graphics::TextureType::NORMAL_MAP, nullptr}, {tz::graphics::TextureType::PARALLAX_MAP, nullptr}, {tz::graphics::TextureType::DISPLACEMENT_MAP, nullptr}}, tz::physics::default_mass, position, rotation, scale){}

BoundingSphere Shot::boundary() const
{
	return tz::physics::bound_sphere(*this);
}