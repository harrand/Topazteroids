#include "powerup.hpp"

Powerup::Powerup(PowerupType type, Vector3F position, float brightness): BaseLight(position, Vector3F(static_cast<unsigned int>(type), static_cast<unsigned int>(type)/2, 0), brightness), type(type)
{
	
}

bool Powerup::reaches(const Vector3F& playerPosition, float leeway)
{
	return (this->getPosition() - playerPosition).length() <= leeway;
}

const Powerup::PowerupType& Powerup::getType() const
{
	return this->type;
}

Powerup::PowerupType& Powerup::getTypeR()
{
	return this->type;
}