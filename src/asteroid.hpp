#ifndef ASTEROID_HPP
#define ASTEROID_HPP
#include "TZ/engine.hpp"

class Asteroid : public EntityObject
{
public:
	Asteroid(const Engine& engine, Vector3F position, Vector3F rotation, Vector3F scale);
	BoundingSphere boundary() const;
};

#endif