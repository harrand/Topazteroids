#include "asteroid.hpp"
#include "shot.hpp"
#include "TZ/gui_display.hpp"

void launch();
bool will_collide(const Camera& camera, Vector3F motion, const std::vector<Asteroid>& asteroid_list);
void impact(unsigned int& lives, bool& just_impacted);
void shoot(const Engine& engine, const Camera& camera, std::vector<Shot>& shot_list);
void die();
void cleanup(const Camera& camera, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list);
#ifdef main
#undef main
#endif

namespace asteroids
{
	extern float player_speed;
}

int main()
{
	tz::initialise();
	launch();
	tz::terminate();
	return 0;
}

float asteroids::player_speed;

void launch()
{
	Window game_window(800, 600, "Asteroids");
	Engine engine(&game_window, "../../../res/runtime/properties.mdl");
	AudioMusic bgm("../../../res/runtime/music/asteroids.wav");
	
	unsigned int level = 1, score = 0, lives = 5;
	bool just_impacted = false;
	
	bgm.play();
	KeyListener key_listener;
	MouseListener mouse_listener;
	Random random;
	
	std::vector<Asteroid> asteroid_list;
	std::vector<Shot> shot_list;
	TrivialFunctor render_asteroid_list([&](){for(auto& asteroid : asteroid_list)asteroid.render(engine.camera, &engine.default_shader, game_window.get_width(), game_window.get_height());});
	TrivialFunctor update_asteroid_list([&](){for(auto& asteroid : asteroid_list)asteroid.update_motion(engine.get_fps());});
	TrivialFunctor render_shot_list([&](){for(auto& shot : shot_list)shot.render(engine.camera, &engine.default_shader, game_window.get_width(), game_window.get_height());});
	TrivialFunctor update_shot_list([&](){for(auto& shot : shot_list){shot.update_motion(engine.get_fps());}});
	engine.add_update_command(&render_asteroid_list);
	engine.add_tick_command(&update_asteroid_list);
	engine.add_update_command(&render_shot_list);
	engine.add_tick_command(&update_shot_list);
	
	asteroids::player_speed = tz::util::cast::from_string<float>(MDLF(RawFile(engine.get_properties().get_tag("resources"))).get_tag("speed"));
	engine.register_listener(key_listener);
	engine.register_listener(mouse_listener);
	
	Font example_font("../../../res/runtime/fonts/upheaval.ttf", 25);
	TextLabel lives_label(0.0f, 0.0f, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Lives: x x x x x", engine.default_gui_shader);
	TextLabel level_label(0.0f, game_window.get_height() - 50, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Level 1", engine.default_gui_shader);
	TextLabel score_label(0.0f, game_window.get_height() - 50 - (level_label.get_height() * 2), Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Score: 0", engine.default_gui_shader);
	game_window.add_child(&lives_label);
	game_window.add_child(&level_label);
	game_window.add_child(&score_label);
	
	asteroid_list.emplace_back(engine, Vector3F(), Vector3F(), Vector3F(10, 10, 10));
	
	float rotational_speed = tz::util::cast::from_string<float>(engine.get_resources().get_tag("rotational_speed"));
	Timer seconds;
	while(!engine.get_window().is_close_requested())
	{
		cleanup(engine.camera, asteroid_list, shot_list);
		engine.update(0);
		if(seconds.millis_passed(1000))
		{
			std::string lives_string = "Lives:";
			for(unsigned int i = 0; i < lives; i++)
				lives_string += " x";
			lives_label.set_text(lives_string);
			level_label.set_text("Level " + tz::util::cast::to_string(level));
			score_label.set_text("Score: " + tz::util::cast::to_string(score));
			tz::util::log::message("A second has passed.");
			seconds.reload();
		}
		seconds.update();
		if(engine.is_update_due())
		{
			if(key_listener.is_key_pressed("W"))
			{
				if(will_collide(engine.camera, engine.camera.forward() * asteroids::player_speed, asteroid_list))
					impact(lives, just_impacted);
				else
					engine.camera.position += engine.camera.forward() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("S"))
			{
				if(will_collide(engine.camera, engine.camera.backward() * asteroids::player_speed, asteroid_list))
					impact(lives, just_impacted);
				else
					engine.camera.position += engine.camera.backward() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("A"))
			{
				if(will_collide(engine.camera, engine.camera.left() * asteroids::player_speed, asteroid_list))
					impact(lives, just_impacted);
				else
					engine.camera.position += engine.camera.left() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("D"))
			{
				if(will_collide(engine.camera, engine.camera.right() * asteroids::player_speed, asteroid_list))
					impact(lives, just_impacted);
				else
					engine.camera.position += engine.camera.right() * asteroids::player_speed;
			}
			if(key_listener.catch_key_pressed("F"))
				shoot(engine, engine.camera, shot_list);
			if(mouse_listener.is_left_clicked())
			{
				Vector2F delta = mouse_listener.get_mouse_delta_pos();
				engine.camera.rotation.y += rotational_speed * delta.x;
				engine.camera.rotation.x -= rotational_speed * delta.y;
				mouse_listener.reload_mouse_delta();
			}
		}
	}
}

bool will_collide(const Camera& camera, Vector3F motion, const std::vector<Asteroid>& asteroid_list)
{
	bool ret = false;
	for(const auto& asteroid : asteroid_list)
	{
		Vector3F future = camera.position + motion;
		BoundingSphere future_boundary(future, 0.1f);
		if(future_boundary.intersects(asteroid.boundary()))
			ret = true;
	}
	return ret;
}

void impact(unsigned int& lives, bool& just_impacted)
{
	if(just_impacted)
		return;
	just_impacted = true;
	std::function<void(std::reference_wrapper<bool>)> set_impacted_off = [](std::reference_wrapper<bool> just_impacted)->void{just_impacted.get() = false;};
	tz::time::scheduler::async_delayed_task<void, std::reference_wrapper<bool>>(5000, set_impacted_off, std::ref(just_impacted));
	if(lives <= 1)
	{
		lives = 0;
		die();
		return;
	}
	lives--;
	tz::audio::play_clip_async(AudioClip("../../../res/runtime/music/bang.wav"));
}

void shoot(const Engine& engine, const Camera& camera, std::vector<Shot>& shot_list)
{
	shot_list.emplace_back(engine, camera.position, Vector3F(), Vector3F(2, 2, 2));
	shot_list.back().velocity = camera.forward() * 50000;
	tz::audio::play_clip_async(AudioClip("../../../res/runtime/music/shoot.wav"));
}

void die()
{
	tz::util::log::error("oy vey! looks like you have died, goyim.");
}

void cleanup(const Camera& camera, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list)
{
	for(auto iter = asteroid_list.begin(); iter != asteroid_list.end(); iter++)
	{
		if((camera.position - iter->position).length() > 500)
		{
			asteroid_list.erase(iter);
			return;
		}
	}
	for(auto iter = shot_list.begin(); iter != shot_list.end(); iter++)
	{
		if((camera.position - iter->position).length() > 500)
		{
			shot_list.erase(iter);
			return;
		}
	}
}