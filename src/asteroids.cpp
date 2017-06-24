#include "engine.hpp"
#include <time.h>
#include <stdlib.h>

#ifdef main
#undef main
#endif
int main()
{	
	srand(time(NULL));
	LogUtility::message("Initialising camera, player and window...");
	Camera cam;
	Player player(10, cam);
	Window wnd(800, 600, "Asteroids (Level 1)");
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
	for(unsigned int i = 0; i < asteroids; i++)
	{
		std::vector<std::pair<std::string, Texture::TextureType>> textures;
		textures.push_back(std::make_pair(engine.getResources().getTag("lava.path"), Texture::TextureType::TEXTURE));
		textures.push_back(std::make_pair(engine.getResources().getTag("lava_normalmap.path"), Texture::TextureType::NORMAL_MAP));
		textures.push_back(std::make_pair(engine.getResources().getTag("lava_parallaxmap.path"), Texture::TextureType::PARALLAX_MAP));
		textures.push_back(std::make_pair(engine.getResources().getTag("default_displacementmap.path"), Texture::TextureType::DISPLACEMENT_MAP));
		EntityObject asteroid(engine.getResources().getTag("sphere.path"), textures, 10, Vector3F(rand() % (asteroid_dispersion * 2) - asteroid_dispersion, rand() % (asteroid_dispersion * 2) - asteroid_dispersion, rand() % (asteroid_dispersion * 2) - asteroid_dispersion), Vector3F(rand(), rand(), rand()), Vector3F(asteroid_size, asteroid_size, asteroid_size));
		asteroid.applyForce("motion", Force(Vector3F(rand() % (asteroid_max_speed * 2) - asteroid_max_speed, rand() % (asteroid_max_speed * 2) - asteroid_max_speed, rand() % (asteroid_max_speed * 2) - asteroid_max_speed) * level));
		// Don't spawn unfairly close
		if((asteroid.getPosition() - player.getPosition()).length() > (asteroid_size * 2))
			engine.getWorldR().addEntityObject(asteroid);
	}
	LogUtility::message("Asteroids are near you, DON'T CRASH!");
	
	TimeKeeper tk;
	unsigned int score = 0;
	while(!wnd.isCloseRequested())
	{
		tk.update();
		engine.update(shader_id, mc, kc);
		for(std::size_t i = 0; i < engine.getWorld().getEntityObjects().size(); i++)
		{
			EntityObject eo = engine.getWorldR().getEntityObjectsR().at(i);
			if((eo.getPosition() - player.getPosition()).length() < (eo.getScale().getX()) && player.getForces().find("impact") == player.getForces().end())
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
					abort();
				}
			}
			else if((eo.getPosition() - player.getPosition()).length() > 10000)
			{
				engine.getWorldR().getEntityObjectsR().erase(engine.getWorldR().getEntityObjectsR().begin() + i);
				LogUtility::message("Culled an asteroid for being too far away...");
			}
		}
		if(tk.millisPassed((500 * level) - score))
		{
			std::vector<std::pair<std::string, Texture::TextureType>> textures;
			textures.push_back(std::make_pair(engine.getResources().getTag("lava.path"), Texture::TextureType::TEXTURE));
			textures.push_back(std::make_pair(engine.getResources().getTag("lava_normalmap.path"), Texture::TextureType::NORMAL_MAP));
			textures.push_back(std::make_pair(engine.getResources().getTag("lava_parallaxmap.path"), Texture::TextureType::PARALLAX_MAP));
			textures.push_back(std::make_pair(engine.getResources().getTag("default_displacementmap.path"), Texture::TextureType::DISPLACEMENT_MAP));
			EntityObject asteroid(engine.getResources().getTag("sphere.path"), textures, 10, Vector3F(rand() % (asteroid_dispersion * 2) - asteroid_dispersion, rand() % (asteroid_dispersion * 2) - asteroid_dispersion, rand() % (asteroid_dispersion * 2) - asteroid_dispersion), Vector3F(), Vector3F(asteroid_size, asteroid_size, asteroid_size) * level);
			asteroid.applyForce("motion", Force(Vector3F(rand() % (asteroid_max_speed * 2) - asteroid_max_speed, rand() % (asteroid_max_speed * 2) - asteroid_max_speed, rand() % (asteroid_max_speed * 2) - asteroid_max_speed) * level));
			// Don't spawn unfairly close
			if((asteroid.getPosition() - player.getPosition()).length() > (asteroid_size * 2))
				engine.getWorldR().addEntityObject(asteroid);
			tk.reload();
			score += 5;
			if(score >= (500 * level))
			{
				wnd.setTitle("Asteroids (Level "+ CastUtility::toString(++level) + ")");
			}
			LogUtility::message("Score = ", score, " (Level ", level, ")");
		}
	}
	return 0;
}