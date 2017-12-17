#include "asteroid.hpp"
#include "shot.hpp"
#include "powerup.hpp"
#include "TZ/gui_display.hpp"
#include <chrono>

void launch();
bool will_collide(const Camera& camera, Vector3F motion, const std::vector<Asteroid>& asteroid_list);
void impact(unsigned int recovery_period, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list, TextLabel& game_over_label);
void spawn_powerup(const Engine& engine, Random<>& random, const Camera& camera, std::vector<Powerup>& powerup_list);
void shoot(const Engine& engine, const Camera& camera, bool& just_shot, std::vector<Shot>& shot_list);
void spawn_asteroid_within_range(const Engine& engine, Random<>& random, const Camera& camera, std::vector<Asteroid>& asteroid_list);
void die(std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list, TextLabel& game_over_label);
std::string consume_powerup(const Engine& engine, Random<>& random, Camera& camera, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list);
void cleanup(const Camera& camera, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list);
#ifdef main
#undef main
#endif

namespace asteroids
{
	extern float player_speed, initial_player_speed;
	extern unsigned int chase_multiplier;
	extern bool game_over;
	extern bool just_impacted;
	extern unsigned int shot_streak;
	extern unsigned int level;
	extern unsigned int score;
	extern unsigned int lives;
	extern unsigned int shot_period, initial_shot_period;
	extern unsigned int asteroid_dispersion;
	constexpr unsigned int initial_level = 1;
	constexpr unsigned int initial_score = 0;
	constexpr unsigned int initial_lives = 5;
	constexpr float player_boundary_radius = 0.1f;
	constexpr unsigned int aimbot_delay = 250;
	
	constexpr float initial_asteroid_radius = 10;
	constexpr Vector3F initial_asteroid_scale(std::array<float, 3>{initial_asteroid_radius, initial_asteroid_radius, initial_asteroid_radius});
	constexpr float initial_shot_radius = 4;
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
float asteroids::initial_player_speed;
unsigned int asteroids::chase_multiplier;
bool asteroids::game_over;
bool asteroids::just_impacted;
unsigned int asteroids::shot_streak;
unsigned int asteroids::level;
unsigned int asteroids::score;
unsigned int asteroids::lives;
unsigned int asteroids::shot_period, asteroids::initial_shot_period;
unsigned int asteroids::asteroid_dispersion;

void launch()
{
	Window game_window(800, 600, "Asteroids 3D");
	Engine engine(&game_window, "../../../res/runtime/properties.mdl");
	AudioMusic bgm("../../../res/runtime/music/asteroids.wav");
	
	asteroids::level = asteroids::initial_level, asteroids::score = asteroids::initial_score, asteroids::lives = asteroids::initial_lives;
	asteroids::just_impacted = false;
	bool just_shot = false;
	asteroids::player_speed = asteroids::initial_player_speed = tz::util::cast::from_string<float>(engine.get_properties().get_tag("initial_speed"));
	const unsigned int recovery_period = tz::util::cast::from_string<unsigned int>(engine.get_properties().get_tag("recovery_period"));
	asteroids::initial_shot_period = asteroids::shot_period = tz::util::cast::from_string<unsigned int>(engine.get_properties().get_tag("shot_period"));
	asteroids::chase_multiplier = tz::util::cast::from_string<unsigned int>(engine.get_properties().get_tag("asteroid_chase_multiplier"));
	asteroids::asteroid_dispersion = tz::util::cast::from_string<unsigned int>(engine.get_properties().get_tag("asteroid_dispersion"));
	asteroids::game_over = false;
	asteroids::shot_streak = 0;
	const unsigned int asteroid_probability_tick = tz::util::cast::from_string<unsigned int>(engine.get_properties().get_tag("asteroid_probability_tick"));
	const unsigned int powerup_probability_tick = tz::util::cast::from_string<unsigned int>(engine.get_properties().get_tag("powerup_probability_tick"));
	
	bgm.play();
	KeyListener key_listener;
	MouseListener mouse_listener;
	Random random;
	
	CubeMap skybox_texture("../../../res/runtime/textures/skybox/", "cwd", ".jpg");
	Shader skybox_shader("../../../res/runtime/shaders/skybox");
	Skybox skybox("../../../res/runtime/models/skybox.obj", skybox_texture);
	TrivialFunctor render_skybox([&](){skybox.render(engine.camera, skybox_shader, engine.get_meshes(), game_window.get_width(), game_window.get_height());});
	engine.add_update_command(&render_skybox);
	
	std::vector<Asteroid> asteroid_list;
	std::vector<Shot> shot_list;
	std::vector<Powerup> powerup_list;
	TrivialFunctor render_asteroid_list([&](){for(auto& asteroid : asteroid_list)asteroid.render(engine.camera, &engine.default_shader, game_window.get_width(), game_window.get_height());});
	TrivialFunctor update_asteroid_list([&](){for(auto& asteroid : asteroid_list)asteroid.update_motion(engine.get_tps());});
	TrivialFunctor render_shot_list([&](){for(auto& shot : shot_list)shot.render(engine.camera, &engine.default_shader, game_window.get_width(), game_window.get_height());});
	TrivialFunctor update_shot_list([&](){for(auto& shot : shot_list){shot.update_motion(engine.get_tps());}});
	TrivialFunctor render_powerup_list([&](){for(auto& powerup : powerup_list){powerup.rotation.y += (5.0f / engine.get_fps());powerup.render(engine.camera, &engine.default_shader, game_window.get_width(), game_window.get_height());}});
	TrivialFunctor update_powerup_list([&](){for(auto& powerup : powerup_list){powerup.update_motion(engine.get_tps());}});
	engine.add_update_command(&render_asteroid_list);
	engine.add_tick_command(&update_asteroid_list);
	engine.add_update_command(&render_shot_list);
	engine.add_tick_command(&update_shot_list);
	engine.add_update_command(&render_powerup_list);
	engine.add_tick_command(&update_powerup_list);

	engine.register_listener(key_listener);
	engine.register_listener(mouse_listener);
	
	Font example_font("../../../res/runtime/fonts/upheaval.ttf", 25);
	TextLabel lives_label(0.0f, 0.0f, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Lives: x x x x x", engine.default_gui_shader);
	TextLabel powerup_label(0.0f, lives_label.get_height() * 2, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, " ", engine.default_gui_shader);
	TextLabel level_label(0.0f, game_window.get_height() - 50, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Level 1", engine.default_gui_shader);
	TextLabel score_label(0.0f, game_window.get_height() - 50 - (level_label.get_height() * 2), Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Score: 0", engine.default_gui_shader);
	TextLabel streak_label(0.0f, game_window.get_height() - 50 - (level_label.get_height() * 2) - (score_label.get_height() * 2), Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "0", engine.default_gui_shader);
	TextLabel game_over_text(game_window.get_width() / 2, game_window.get_height() / 2, Vector4F(1, 0, 0, 1), {}, Vector3F(0, 0, 0), example_font, "Game Over!", engine.default_gui_shader);
	game_over_text.set_hidden(true);
	game_window.add_child(&lives_label);
	game_window.add_child(&powerup_label);
	game_window.add_child(&level_label);
	game_window.add_child(&score_label);
	game_window.add_child(&streak_label);
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
			asteroids::score += 1;
			std::string lives_string = "Lives:";
			for(unsigned int i = 0; i < asteroids::lives; i++)
				lives_string += " x";
			lives_label.set_text(lives_string);
			level_label.set_text("Level " + tz::util::cast::to_string(asteroids::level));
			score_label.set_text("Score: " + tz::util::cast::to_string(asteroids::score));
			streak_label.set_text(tz::util::cast::to_string(asteroids::shot_streak));
			if(asteroids::shot_streak >= 9)
				streak_label.set_colour(Vector4F(1, 0, 0, 1));
			else if(asteroids::shot_streak >= 5)
				streak_label.set_colour(Vector4F(0.5, 0.5, 0, 1));
			else if(asteroids::shot_streak >= 3)
				streak_label.set_colour(Vector4F(0, 1, 0, 1));
			else if(asteroids::shot_streak < 2)
				streak_label.set_colour(Vector4F(1, 1, 1, 1));
			seconds.reload();
		}
		seconds.update();
		if(engine.is_update_due())
		{
			if(static_cast<unsigned int>(random.next_int(0, 100)) < asteroid_probability_tick)
				spawn_asteroid_within_range(engine, random, engine.camera, asteroid_list);
			if(static_cast<unsigned int>(random.next_int(0, 100)) < powerup_probability_tick)
				spawn_powerup(engine, random, engine.camera, powerup_list);
			BoundingSphere player_boundary(engine.camera.position, asteroids::player_boundary_radius);
			for(auto iter = powerup_list.begin(); iter != powerup_list.end(); iter++)
			{
				if(player_boundary.intersects(iter->boundary()))
				{
					//const Engine& engine, Random<>& random, Camera& camera, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list
					powerup_label.set_text(consume_powerup(engine, random, engine.camera, asteroid_list, shot_list));
					iter = powerup_list.erase(iter);
					if(iter == powerup_list.end())
						break;
				}
			}
			if(key_listener.is_key_pressed("W"))
			{
				if(will_collide(engine.camera, engine.camera.forward() * asteroids::player_speed, asteroid_list) && !asteroids::just_impacted)
					impact(recovery_period, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.forward() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("S"))
			{
				if(will_collide(engine.camera, engine.camera.backward() * asteroids::player_speed, asteroid_list))
					impact(recovery_period, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.backward() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("A"))
			{
				if(will_collide(engine.camera, engine.camera.left() * asteroids::player_speed, asteroid_list))
					impact(recovery_period, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.left() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("D"))
			{
				if(will_collide(engine.camera, engine.camera.right() * asteroids::player_speed, asteroid_list))
					impact(recovery_period, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.right() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("Space"))
			{
				if(will_collide(engine.camera, engine.camera.up() * asteroids::player_speed, asteroid_list))
					impact(recovery_period, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.up() * asteroids::player_speed;
			}
			if(key_listener.is_key_pressed("Z"))
			{
				if(will_collide(engine.camera, engine.camera.down() * asteroids::player_speed, asteroid_list))
					impact(recovery_period, asteroid_list, shot_list, game_over_text);
				else
					engine.camera.position += engine.camera.down() * asteroids::player_speed;
			}
			if(key_listener.catch_key_pressed(tz::util::cast::to_string(engine.get_properties().get_tag("shoot_keybind"))))
				shoot(engine, engine.camera, just_shot, shot_list);
			if(key_listener.catch_key_pressed("M"))
				bgm.set_paused(!bgm.is_paused());
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

void impact(unsigned int recovery_period, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list, TextLabel& game_over_label)
{
	if(asteroids::just_impacted)
		return;
	asteroids::just_impacted = true;
	std::function<void(std::reference_wrapper<bool>)> set_impacted_off = [](std::reference_wrapper<bool> just_impacted)->void{just_impacted.get() = false;};
	tz::time::scheduler::async_delayed_task<void, std::reference_wrapper<bool>>(recovery_period, set_impacted_off, std::ref(asteroids::just_impacted));
	if(asteroids::lives <= 1)
	{
		asteroids::lives = 0;
		die(asteroid_list, shot_list, game_over_label);
		return;
	}
	asteroids::lives--;
	tz::audio::play_clip_async(AudioClip("../../../res/runtime/music/bang.wav"));
}

void spawn_powerup(const Engine& engine, Random<>& random, const Camera& camera, std::vector<Powerup>& powerup_list)
{
	auto value_ranged = [&]()->float{return random.next_float(asteroids::asteroid_dispersion / -2.0f, asteroids::asteroid_dispersion / 2.0f);};
	Vector3F position_offset = Vector3F(value_ranged(), value_ranged(), value_ranged()) + camera.position;
	powerup_list.emplace_back(engine, position_offset, Vector3F(), asteroids::initial_asteroid_scale);
}

void shoot(const Engine& engine, const Camera& camera, bool& just_shot, std::vector<Shot>& shot_list)
{
	if(just_shot)
		return;
	just_shot = true;
	std::function<void(std::reference_wrapper<bool>)> set_shot_off = [](std::reference_wrapper<bool> just_shot)->void{just_shot.get() = false;};
	tz::time::scheduler::async_delayed_task<void, std::reference_wrapper<bool>>(asteroids::shot_period, set_shot_off, std::ref(just_shot));
	
	shot_list.emplace_back(engine, camera.position, camera.rotation, asteroids::initial_shot_scale);
	shot_list.back().velocity = camera.forward() * tz::util::cast::from_string<unsigned int>(engine.get_properties().get_tag("shot_speed"));
	tz::audio::play_clip_async(AudioClip("../../../res/runtime/music/shoot.wav"));
}

void spawn_asteroid_within_range(const Engine& engine, Random<>& random, const Camera& camera, std::vector<Asteroid>& asteroid_list)
{
	const float asteroid_max_speed = tz::util::cast::from_string<float>(engine.get_properties().get_tag("asteroid_max_speed"));
	auto value_ranged = [&]()->float{return random.next_float(asteroids::asteroid_dispersion / -2.0f, asteroids::asteroid_dispersion / 2.0f);};
	Vector3F position_offset = Vector3F(value_ranged(), value_ranged(), value_ranged()) + camera.position;
	// ensure position_offset is at least a certain distance away from the player
	position_offset += (position_offset - camera.position).normalised() * (asteroids::asteroid_dispersion);
	// position offset always has at least half the max distance away from the camera.
	asteroid_list.emplace_back(engine, position_offset, Vector3F(), asteroids::initial_asteroid_scale * random.next_float(0.25, 1.0) * asteroids::level * asteroids::level);
	asteroid_list.back().velocity = Vector3F(random.next_float(-1, 1), random.next_float(-1, 1), random.next_float(-1, 1));
	asteroid_list.back().velocity += (camera.position - asteroid_list.back().position) * asteroids::chase_multiplier * std::sqrt(std::sqrt(asteroids::level));
	asteroid_list.back().velocity = asteroid_list.back().velocity.normalised() * asteroid_max_speed * std::sqrt(asteroids::level);
}

void die(std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list, TextLabel& game_over_label)
{
	asteroid_list.clear();
	shot_list.clear();
	asteroids::game_over = true;
	game_over_label.set_hidden(false);
}

std::string consume_powerup(const Engine& engine, Random<>& random, Camera& camera, std::vector<Asteroid>& asteroid_list, std::vector<Shot>& shot_list)
{
	std::string message;
	tz::audio::play_clip_async(AudioClip("../../../res/runtime/music/powerup.wav"));
	switch(random.next_int(0, 5))
	{
		case 0:
			asteroids::player_speed += (asteroids::initial_player_speed * 0.75);
			message = "Speed";
			break;
		case 1:
			for(auto& asteroid : asteroid_list)
				asteroid.scale /= 2;
			message = "Size";
			break;
		case 2:
			asteroids::lives++;
			message = "Life";
			break;
		case 3:
		{
			message = "Gattling Gun";
			asteroids::shot_period = asteroids::initial_shot_period / 10.0f;
			TrivialFunctor shot_period_reset([&](){using namespace std::chrono_literals;std::this_thread::sleep_for(5000ms);asteroids::shot_period = asteroids::initial_shot_period;});
			std::thread(shot_period_reset).detach();
			break;
		}
		case 4:
			message = "Force";
			for(auto& asteroid : asteroid_list)
				asteroid.velocity = (asteroid.position - camera.position).normalised() * asteroids::initial_player_speed * 1000;
			break;
		case 5:
			message = "Level";
			asteroids::level++;
			break;
	}
	return message;
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
				asteroids::shot_streak++;
				asteroids::score += asteroids::shot_streak;
				if(iter_asteroid == asteroid_list.end() || iter_shot == shot_list.end())
					return;
			}
		}
	}
	for(auto iter = asteroid_list.begin(); iter != asteroid_list.end(); iter++)
	{
		if((camera.position - iter->position).length() > (asteroids::asteroid_dispersion + (asteroids::player_speed * 200)))
		{
			iter = asteroid_list.erase(iter);
			if(iter == asteroid_list.end())
				return;
		}
	}
	for(auto iter = shot_list.begin(); iter != shot_list.end(); iter++)
	{
		if((camera.position - iter->position).length() > (asteroids::asteroid_dispersion + (asteroids::player_speed * 200)))
		{
			asteroids::shot_streak = 0;
			iter = shot_list.erase(iter);
			if(iter == shot_list.end())
				return;
		}
	}
}