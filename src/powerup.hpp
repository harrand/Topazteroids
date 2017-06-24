#ifndef POWERUP_HPP
#define POWERUP_HPP
#include "light.hpp"

class Powerup: public BaseLight
{
public:
	enum class PowerupType : unsigned int
	{
		NOLIGHT = 0,
		PULSE = 1,
		SPEED = 2,
		LIFE = 3,
		NIL = 4,
	};
	Powerup(PowerupType type, Vector3F position, float brightness);
	
	bool reaches(const Vector3F& playerPosition, float leeway);
	
	const PowerupType& getType() const;
	PowerupType& getTypeR();
private:
	PowerupType type;
};

#endif