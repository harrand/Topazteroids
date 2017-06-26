#include "engine.hpp"
#include <time.h>
#include <stdlib.h>

void spawnTorpedo(Player& player, Engine& engine)
{
	std::vector<std::pair<std::string, Texture::TextureType>> textures;
	textures.push_back(std::make_pair(engine.getResources().getTag("torpedo.path"), Texture::TextureType::TEXTURE));
	textures.push_back(std::make_pair(engine.getResources().getTag("stone_normalmap.path"), Texture::TextureType::NORMAL_MAP));
	textures.push_back(std::make_pair(engine.getResources().getTag("stone_parallaxmap.path"), Texture::TextureType::PARALLAX_MAP));
	textures.push_back(std::make_pair(engine.getResources().getTag("default_displacementmap.path"), Texture::TextureType::DISPLACEMENT_MAP));
	float torpedo_size = CastUtility::fromString<float>(engine.getProperties().getTag("torpedo_size"));
	EntityObject torpedo(engine.getResources().getTag("sphere.path"), textures, 10, player.getPosition(), Vector3F(), Vector3F(torpedo_size, torpedo_size, torpedo_size));
	torpedo.applyForce("shoot", Force(player.getCamera().getForward() * CastUtility::fromString<float>(engine.getProperties().getTag("torpedo_speed"))));
	engine.getWorldR().addEntityObject(torpedo);
}

bool isTorpedo(const EntityObject& eo, const Engine& engine)
{
	for(auto pair : eo.getTextures())
	{
		if(pair.second == Texture::TextureType::TEXTURE && pair.first == engine.getResources().getTag("torpedo.path"))
			return true;
	}
	return false;
}

void spawnPowerup(Player& player, Engine& engine, int asteroid_dispersion)
{	
	std::vector<std::pair<std::string, Texture::TextureType>> textures;
	textures.push_back(std::make_pair(engine.getResources().getTag("undefined.path"), Texture::TextureType::TEXTURE));
	textures.push_back(std::make_pair(engine.getResources().getTag("default_normalmap.path"), Texture::TextureType::NORMAL_MAP));
	textures.push_back(std::make_pair(engine.getResources().getTag("default_parallaxmap.path"), Texture::TextureType::PARALLAX_MAP));
	textures.push_back(std::make_pair(engine.getResources().getTag("default_displacementmap.path"), Texture::TextureType::DISPLACEMENT_MAP));
	float powerup_size = CastUtility::fromString<float>(engine.getProperties().getTag("powerup_size"));
	Object powerup(engine.getResources().getTag("sphere.path"), textures, player.getPosition() + Vector3F(rand() % (asteroid_dispersion * 2) - asteroid_dispersion, rand() % (asteroid_dispersion * 2) - asteroid_dispersion, rand() % (asteroid_dispersion * 2) - asteroid_dispersion), Vector3F(), Vector3F(powerup_size, powerup_size, powerup_size));
	engine.getWorldR().addObject(powerup);
	
	LogUtility::message("Powerup spawned at location ", StringUtility::format(StringUtility::devectoriseList3<float>(powerup.getPosition())));
}

void consumePowerup(unsigned int& level, unsigned int& score, unsigned int& lives, Player& player, Engine& engine)
{
	switch(rand() % 6)
	{
	case 0:
		lives++;
		LogUtility::message("You gained a life!");
	break;
	case 1:
		for(EntityObject& eo : engine.getWorldR().getEntityObjectsR())
			eo.getScaleR() = eo.getScale() / 2;
		LogUtility::message("Halved the size of all asteroids!");
	break;
	case 2:
		player.applyForce("impact", Force());
		LogUtility::message("Immune to damage for 5 seconds!");
		{
		std::function<void(std::reference_wrapper<Player> p)> undoImpact([](Player& p)->void{p.removeForce("impact");});
		Scheduler::asyncDelayedTask<void, std::reference_wrapper<Player>>(5000, undoImpact, std::ref(player));
		}
	break;
	case 3:
		for(EntityObject& eo : engine.getWorldR().getEntityObjectsR())
		{
			for(auto pair : eo.getForces())
				eo.removeForce(pair.first);
			eo.applyForce("pulse", Force((eo.getPosition() - player.getPosition()).normalised() * CastUtility::fromString<float>(engine.getProperties().getTag("pulse_force"))));
		}
		LogUtility::message("Force Pulse! All asteroids pushed away!");
	break;
	case 4:
		{
			float curSpeed = CastUtility::fromString<float>(engine.getResources().getTag("speed"));
			Commands::inputCommand("setspeed " + CastUtility::toString(curSpeed * 2.0f), engine.getWorldR(), player, engine.getDefaultShader());
			Commands::inputCommand("delayedcmd 5000 setspeed " + CastUtility::toString(curSpeed), engine.getWorldR(), player, engine.getDefaultShader());
			LogUtility::message("Speed doubled for 5 seconds!");
		}
	break;
	case 5:
		level++;
		score += 500;
		LogUtility::message("Gained 500 score, moving you to the next level!");
	break;
	}
}

void spawnAsteroid(Player& player, Engine& engine, unsigned int level, int asteroid_dispersion, int asteroid_max_speed, unsigned int asteroid_size)
{
	std::vector<std::pair<std::string, Texture::TextureType>> textures;
	switch(rand() % 3)
	{
	case 0:
		textures.push_back(std::make_pair(engine.getResources().getTag("lava.path"), Texture::TextureType::TEXTURE));
		textures.push_back(std::make_pair(engine.getResources().getTag("lava_normalmap.path"), Texture::TextureType::NORMAL_MAP));
		textures.push_back(std::make_pair(engine.getResources().getTag("lava_parallaxmap.path"), Texture::TextureType::PARALLAX_MAP));
		textures.push_back(std::make_pair(engine.getResources().getTag("default_displacementmap.path"), Texture::TextureType::DISPLACEMENT_MAP));
	break;
	case 1:
		textures.push_back(std::make_pair(engine.getResources().getTag("sand.path"), Texture::TextureType::TEXTURE));
		textures.push_back(std::make_pair(engine.getResources().getTag("sand_normalmap.path"), Texture::TextureType::NORMAL_MAP));
		textures.push_back(std::make_pair(engine.getResources().getTag("sand_parallaxmap.path"), Texture::TextureType::PARALLAX_MAP));
		textures.push_back(std::make_pair(engine.getResources().getTag("default_displacementmap.path"), Texture::TextureType::DISPLACEMENT_MAP));
	break;
	default:
		textures.push_back(std::make_pair(engine.getResources().getTag("metal.path"), Texture::TextureType::TEXTURE));
		textures.push_back(std::make_pair(engine.getResources().getTag("metal_normalmap.path"), Texture::TextureType::NORMAL_MAP));
		textures.push_back(std::make_pair(engine.getResources().getTag("metal_parallaxmap.path"), Texture::TextureType::PARALLAX_MAP));
		textures.push_back(std::make_pair(engine.getResources().getTag("default_displacementmap.path"), Texture::TextureType::DISPLACEMENT_MAP));
	break;
	}
	
	EntityObject asteroid(engine.getResources().getTag("sphere.path"), textures, 10, player.getPosition() + Vector3F(rand() % (asteroid_dispersion * 2) - asteroid_dispersion, rand() % (asteroid_dispersion * 2) - asteroid_dispersion, rand() % (asteroid_dispersion * 2) - asteroid_dispersion), Vector3F(), Vector3F(asteroid_size, asteroid_size, asteroid_size) * (level == 1 ? level : (rand() % (level - 1) + 1)));
	asteroid.applyForce("motion", Force(Vector3F(rand() % (asteroid_max_speed * 2) - asteroid_max_speed, rand() % (asteroid_max_speed * 2) - asteroid_max_speed, rand() % (asteroid_max_speed * 2) - asteroid_max_speed) * level));
	asteroid.applyForce("chase", Force((asteroid.getPosition() - player.getPosition()).normalised() * pow(level, 1.5) * -CastUtility::fromString<float>(engine.getProperties().getTag("asteroid_chase_multiplier"))));
	// Don't spawn unfairly close
	if((asteroid.getPosition() - player.getPosition()).length() > (asteroid.getScale().getX() * 2))
		engine.getWorldR().addEntityObject(asteroid);
	else
		spawnAsteroid(player, engine, level, asteroid_dispersion, asteroid_max_speed, asteroid_size);
}

#ifdef main
#undef main
#endif
int main()
{	
	srand(time(NULL));
	LogUtility::message("Initialising camera, player and window...");
	Camera cam;
	Player player(10, cam);
	Window wnd(800, 600, "Asteroids");
	
	KeyListener kl;
	wnd.registerListener(kl);
	
	LogUtility::message("Initialising engine...");
	Engine engine(player, wnd, "../../../res/runtime/properties.mdl");
	LogUtility::message("Initialising key and mouse controllers...");
	std::size_t shader_id = 0;
	KeybindController kc(player, engine.getShader(shader_id), engine.getWorldR(), wnd);
	MouseController mc(player, engine.getWorldR(), wnd);

	AudioMusic music(engine.getProperties().getTag("music_path"));
	music.play();
	unsigned int level = 1;
	unsigned int lives = 5;
	LogUtility::message("Beginning asteroid generation...");
	unsigned int asteroids = CastUtility::fromString<unsigned int>(engine.getProperties().getTag("asteroids"));
	int asteroid_dispersion = CastUtility::fromString<unsigned int>(engine.getProperties().getTag("asteroid_dispersion"));
	int asteroid_max_speed = CastUtility::fromString<unsigned int>(engine.getProperties().getTag("asteroid_max_speed"));
	unsigned int asteroid_size = CastUtility::fromString<unsigned int>(engine.getProperties().getTag("asteroid_size"));
	LogUtility::message("Loading ", asteroids, " asteroids with following attributes:");
	LogUtility::message("\tMaximum Dispersion = ", asteroid_dispersion);
	LogUtility::message("\tMaximum Speed = ", asteroid_max_speed);
	LogUtility::message("\tAsteroid Size = ", asteroid_size);
	//SDL_SetRelativeMouseMode(SDL_TRUE);
	for(unsigned int i = 0; i < asteroids; i++)
	{
		//Player& player, Engine& engine, unsigned int level, int asteroid_dispersion, int asteroid_max_speed, unsigned int asteroid_size
		spawnAsteroid(player, engine, level, asteroid_dispersion, asteroid_max_speed, asteroid_size);
	}
	LogUtility::message("Asteroids are near you, DON'T CRASH!");
	
	TimeKeeper tk;
	unsigned int score = 0;
	while(!wnd.isCloseRequested())
	{
		tk.update();
		engine.update(shader_id, mc, kc);
		if(kl.catchKeyPressed(engine.getProperties().getTag("shoot_keybind")))
			spawnTorpedo(player, engine);
		for(std::size_t i = 0; i < engine.getWorld().getEntityObjects().size(); i++)
		{
			EntityObject eo = engine.getWorldR().getEntityObjectsR().at(i);
			for(std::size_t j = 0; j < engine.getWorld().getEntityObjects().size(); j++)
			{
				EntityObject other_eo = engine.getWorldR().getEntityObjectsR().at(j);
				if(i == j || (eo.getPosition() - other_eo.getPosition()).length() > (eo.getScale().getX() + other_eo.getScale().getX()))
					continue;
				if(isTorpedo(other_eo, engine) && !isTorpedo(eo, engine))
				{
					// other_eo needs to despawn and break eo
					LogUtility::message("PEW!");
					score += 10;
					engine.getWorldR().getEntityObjectsR().erase(engine.getWorldR().getEntityObjectsR().begin() + j);
					engine.getWorldR().getEntityObjectsR().erase(engine.getWorldR().getEntityObjectsR().begin() + i);
					Vector3F dir = eo.getVelocity().cross(other_eo.getVelocity()) * 0.005;
					EntityObject daughter1(eo), daughter2(eo);
					daughter1.setVelocity(dir);
					daughter2.setVelocity(dir * -1);
					engine.getWorldR().addEntityObject(daughter1);
					engine.getWorldR().addEntityObject(daughter2);
				}
			}
			if((eo.getPosition() - player.getPosition()).length() < (eo.getScale().getX()) && player.getForces().find("impact") == player.getForces().end() && !isTorpedo(eo, engine))
			{
				Commands::inputCommand("play noise.wav me", engine.getWorldR(), player, engine.getDefaultShader());
				player.applyForce("impact", eo.getPosition() - player.getPosition());
				std::function<void(std::reference_wrapper<Player> p)> undoImpact([](Player& p)->void{p.removeForce("impact");});
				Scheduler::asyncDelayedTask<void, std::reference_wrapper<Player>>(5000, undoImpact, std::ref(player));
				LogUtility::warning("IMPACT! Ship integrity at ", (--lives * 2) * 10, "%!");
				if(lives == 1)
					LogUtility::warning("CRITICAL WARNING: System Shutdown imͯm̽in̰eͯn͈t͉ ̈́upo͝n̍ n̥̹e̘͠x͉̌t͎͗͛ ͬͦͩï̶̙mͦp͏a͈c̶̹t͈!");
				if(lives == 0)
				{
					LogUtility::error("HͮUL̍L̊ I̱NT̜E̳̰͆G̸̬̏R̿I̪ͬT͎̓ͫY̴̳̥ ̈́Ć͂ͩO̦͔̐M͕͉̙͙͙͎̀̔̀̚P͉̗̣̘̀̓̀R̴̊͐ͭ̓ͅÓͬ̏ͅ҉̻̓̈́͒Ḿ̫̆͋̆̔̎̆I͋S̬͍̻̓ͦ̑̆E̋D̡̛̮̲̜͇͍͊ͬ̂̄͗̎ͥ͗̓͗͛̕͜,̶̢̤̞̀̎̚ ̢̛̹̟̩͈̩̅ͩ̀͂̈́̉ͩͧ̔̈́͜ABORṰ̶̝͍͎͌̿́͐̉̌̆-̨̱̲̍̊");
					wnd.setTitle("Asteroids | YOU CRASHED! GAME OVER | Level " + CastUtility::toString(level));
					abort();
				}
			}
			else if((eo.getPosition() - player.getPosition()).length() > 10000)
				engine.getWorldR().getEntityObjectsR().erase(engine.getWorldR().getEntityObjectsR().begin() + i);
		}
		for(std::size_t i = 0; i < engine.getWorld().getObjects().size(); i++)
		{
			Object& obj = engine.getWorldR().getObjectsR().at(i);
			if((obj.getPosition() - player.getPosition()).length() < (obj.getScale().getX()))
			{
				engine.getWorldR().getObjectsR().erase(engine.getWorldR().getObjectsR().begin() + i);
				consumePowerup(level, score, lives, player, engine);
				Commands::inputCommand("play powerup.wav me", engine.getWorldR(), player, engine.getDefaultShader());
			}
		}
		if(tk.millisPassed((500 * level) - score))
		{
			if(rand() % 5 == 1)
				spawnPowerup(player, engine, asteroid_dispersion);
			spawnAsteroid(player, engine, level, asteroid_dispersion, asteroid_max_speed, asteroid_size);
			tk.reload();
			score += 5;
			if(score >= (500 * level))
			{
				level++;
			}
			//LogUtility::message("Score = ", score, " (Level ", level, ")");
			std::string lifeBar = "";
			for(unsigned int i = 0; i < lives; i++)
				lifeBar += "[] ";
			wnd.setTitle("Asteroids | Level " + CastUtility::toString(level) + " | Ship Integrity: " + lifeBar);
		}
	}
	return 0;
}