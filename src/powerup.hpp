#ifndef POWERUP_HPP
#define POWERUP_HPP

#include "TZ/engine.hpp"

class Powerup : public EntityObject
{
public:
	Powerup(const Engine& engine, Vector3F position, Vector3F rotation, Vector3F scale);
	BoundingSphere boundary() const;
};


#endif