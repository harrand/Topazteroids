#ifndef LIGHT_HPP
#define LIGHT_HPP
#include "vector.hpp"

class BaseLight
{
public:
	BaseLight(const Vector3F& pos = Vector3F(), const Vector3F& colour = Vector3F(1, 1, 1), const float power = 1.0f);
	BaseLight(const BaseLight& copy) = default;
	BaseLight(BaseLight&& move) = default;
	BaseLight& operator=(const BaseLight& rhs) = default;
	
	const Vector3F& getPos() const;
	const Vector3F& getColour() const;
	float getPower() const;
private:
	Vector3F pos;
	Vector3F colour;
	float power;
};

#endif