#ifndef SHOT_HPP
#define SHOT_HPP
#include "TZ/engine.hpp"

class Shot : public EntityObject
{
public:
	Shot(const Engine& engine, Vector3F position, Vector3F rotation, Vector3F scale);
	BoundingSphere boundary() const;
};

#endif