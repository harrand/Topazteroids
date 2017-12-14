#include "asteroid.hpp"
#include "shot.hpp"
#include "TZ/gui_display.hpp"

void launch();
bool will_collide(const Camera& camera, Vector3F motion, const std::vector<Asteroid>& asteroid_list);
void impact(unsigned int recovery_period, unsigned int& lives, bool& just_impacted, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list, TextLabel& game_over_label);
void shoot(const Engine& engine, const Camera& camera, std::vector<Shot>& shot_list);
void spawn_asteroid_within_range(const Engine& engine, Random<>& random, const Camera& camera, std::vector<Asteroid>& asteroid_list);
void die(std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list, TextLabel& game_over_label);
void cleanup(const Camera& camera, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list);
#ifdef main
#undef main
#endif

namespace asteroids
{
	extern float player_speed;
	extern unsigned int chase_multiplier;
	extern bool game_over;
	constexpr unsigned int culling_distance = 500;
	constexpr unsigned int initial_level = 1;
	constexpr unsigned int initial_score = 0;
	constexpr unsigned int initial_lives = 5;
	constexpr float player_boundary_radius = 0.1f;
	
	constexpr float initial_asteroid_radius = 10;
	constexpr Vector3F initial_asteroid_scale(std::array<float, 3>{initial_asteroid_radius, initial_asteroid_radius, initial_asteroid_radius});
	constexpr float initial_shot_radius = 2;
	constexpr Vector3F initial_shot_scale(std::array<float, 3>{initial_shot_radius, initial_shot_radius, initial_shot_radius * 5});
}

int main()
{
	tz::initialise();
	launch();
	tz::terminate();
	return 0;
}

float asteroids::player_speed;
unsigned int asteroids::chase_multiplier;
bool asteroids::game_over;

void launch()
{
	Window game_window(800, 600, "Asteroids");
	Engine engine(&game_window, "../../../res/runtime/properties.mdl");
	AudioMusic bgm("../../../res/runtime/music/asteroids.wav");
	
	unsigned int level = asteroids::initial_level, score = asteroids::initial_score, lives = asteroids::initial_lives;
	bool just_impacted = false;
	asteroids::player_speed = tz::util::cast::from_string<float>(MDLF(RawFile(engine.get_properties().get_tag("resources"))).get_tag("speed"));
	const unsigned int recovery_period = tz::util::cast::from_string<unsigned int>(engine.get_properties().get_tag("recovery_period"));
	asteroids::chase_multiplier = tz::util::cast::from_string<unsigned int>(engine.get_properties().get_tag("asteroid_chase_multiplier"));
	asteroids::game_over = false;
	
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

	engine.register_listener(key_listener);
	engine.register_listener(mouse_listener);
	
	Font example_font("../../../res/runtime/fonts/upheaval.ttf", 25);
	TextLabel lives_label(0.0f, 0.0f, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Lives: x x x x x", engine.default_gui_shader);
	TextLabel level_label(0.0f, game_window.get_height() - 50, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Level 1", engine.default_gui_shader);
	TextLabel score_label(0.0f, game_window.get_height() - 50 - (level_label.get_height() * 2), Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Score: 0", engine.default_gui_shader);
	TextLabel game_over_text(game_window.get_width() / 2, game_window.get_height() / 2, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Game Over!", engine.default_gui_shader);
	game_over_text.set_hidden(true);
	game_window.add_child(&lives_label);
	game_window.add_child(&level_label);
	game_window.add_child(&score_label);
	game_window.add_child(&game_over_text);
	
	float rotational_speed = tz::util::cast::from_string<float>(engine.get_resources().get_tag("rotational_speed"));
	Timer seconds;
	while(!engine.get_window().is_close_requested())
	{
		engine.update(0);
		if(asteroids::game_over)
			continue;
		cleanup(engine.camera, asteroid_list, shot_list);
		if(seconds.millis_passed(1000))
		{
			score += 1;
			std::string lives_string = "Lives:";
			for(unsigned int i = 0; i < lives; i++)
				lives_string += " x";
			lives_label.set_text(lives_string);
			level_label.set_text("Level " + tz::util::cast::to_string(level));
			score_label.set_text("Score: " + tz::util::cast::to_string(score));
			seconds.reload();
		}
		seconds.update();
		if(engine.is_update_due())
		{
			if(random.next_int(0, 2000) < 200)
				spawn_asteroid_within_range(engine, random, engine.camera, asteroid_list);
			if(key_listener.is_key_pressed("W"))
			{
				if(will_collide(engine.camera, engine.camera.forward() * asteroids::player_speed, asteroid_list))
					impact(recovery_period, lives, just_impacted, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.forward() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("S"))
			{
				if(will_collide(engine.camera, engine.camera.backward() * asteroids::player_speed, asteroid_list))
					impact(recovery_period, lives, just_impacted, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.backward() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("A"))
			{
				if(will_collide(engine.camera, engine.camera.left() * asteroids::player_speed, asteroid_list))
					impact(recovery_period, lives, just_impacted, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.left() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("D"))
			{
				if(will_collide(engine.camera, engine.camera.right() * asteroids::player_speed, asteroid_list))
					impact(recovery_period, lives, just_impacted, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.right() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("Space"))
			{
				if(will_collide(engine.camera, engine.camera.up() * asteroids::player_speed, asteroid_list))
					impact(recovery_period, lives, just_impacted, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.up() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("Z"))
			{
				if(will_collide(engine.camera, engine.camera.down() * asteroids::player_speed, asteroid_list))
					impact(recovery_period, lives, just_impacted, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.down() * asteroids::player_speed;
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
		BoundingSphere future_boundary(future, asteroids::player_boundary_radius);
		if(future_boundary.intersects(asteroid.boundary()))
			ret = true;
	}
	return ret;
}

void impact(unsigned int recovery_period, unsigned int& lives, bool& just_impacted, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list, TextLabel& game_over_label)
{
	if(just_impacted)
		return;
	just_impacted = true;
	std::function<void(std::reference_wrapper<bool>)> set_impacted_off = [](std::reference_wrapper<bool> just_impacted)->void{just_impacted.get() = false;};
	tz::time::scheduler::async_delayed_task<void, std::reference_wrapper<bool>>(recovery_period, set_impacted_off, std::ref(just_impacted));
	if(lives <= 1)
	{
		lives = 0;
		die(asteroid_list, shot_list, game_over_label);
		return;
	}
	lives--;
	tz::audio::play_clip_async(AudioClip("../../../res/runtime/music/bang.wav"));
}

void shoot(const Engine& engine, const Camera& camera, std::vector<Shot>& shot_list)
{
	shot_list.emplace_back(engine, camera.position, camera.rotation, asteroids::initial_shot_scale);
	shot_list.back().velocity = camera.forward() * tz::util::cast::from_string<unsigned int>(engine.get_properties().get_tag("shot_speed"));
	tz::audio::play_clip_async(AudioClip("../../../res/runtime/music/shoot.wav"));
}

void spawn_asteroid_within_range(const Engine& engine, Random<>& random, const Camera& camera, std::vector<Asteroid>& asteroid_list)
{
	const float spread_radius = tz::util::cast::from_string<float>(engine.get_properties().get_tag("asteroid_dispersion"));
	const float asteroid_max_speed = tz::util::cast::from_string<float>(engine.get_properties().get_tag("asteroid_max_speed"));
	Vector3F position_offset = Vector3F(random.next_float(-spread_radius / 2, spread_radius / 2), random.next_float(-spread_radius / 2, spread_radius / 2), random.next_float(-spread_radius / 2, spread_radius / 2)) + camera.position;
	asteroid_list.emplace_back(engine, position_offset, Vector3F(), asteroids::initial_asteroid_scale);
	asteroid_list.back().velocity = Vector3F(random.next_float(-1, 1), random.next_float(-1, 1), random.next_float(-1, 1));
	asteroid_list.back().velocity += (camera.position - asteroid_list.back().position) * asteroids::chase_multiplier;
	asteroid_list.back().velocity = asteroid_list.back().velocity.normalised() * asteroid_max_speed;
}

void die(std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list, TextLabel& game_over_label)
{
	asteroid_list.clear();
	shot_list.clear();
	asteroids::game_over = true;
	game_over_label.set_hidden(false);
}

void cleanup(const Camera& camera, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list)
{
	for(auto iter_asteroid = asteroid_list.begin(); iter_asteroid != asteroid_list.end(); iter_asteroid++)
	{
		for(auto iter_shot = shot_list.begin(); iter_shot != shot_list.end(); iter_shot++)
		{
			if(iter_shot->boundary().intersects(iter_asteroid->boundary()))
			{
				iter_asteroid = asteroid_list.erase(iter_asteroid);
				iter_shot = shot_list.erase(iter_shot);
				tz::audio::play_clip_async(AudioClip("../../../res/runtime/music/shot_impact.wav"));
				if(iter_asteroid == asteroid_list.end() || iter_shot == shot_list.end())
					return;
			}
		}
	}
	for(auto iter = asteroid_list.begin(); iter != asteroid_list.end(); iter++)
	{
		if((camera.position - iter->position).length() > asteroids::culling_distance)
		{
			iter = asteroid_list.erase(iter);
			if(iter == asteroid_list.end())
				return;
		}
	}
	for(auto iter = shot_list.begin(); iter != shot_list.end(); iter++)
	{
		if((camera.position - iter->position).length() > asteroids::culling_distance)
		{
			iter = shot_list.erase(iter);
			if(iter == shot_list.end())
				return;
		}
	}
}