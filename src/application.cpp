#include "asteroid.hpp"
#include "TZ/gui_display.hpp"

void launch();
#ifdef main
#undef main
#endif

namespace asteroids
{
	extern float speed;
}

int main()
{
	tz::initialise();
	launch();
	tz::terminate();
	return 0;
}

float asteroids::speed;

void launch()
{
	Window game_window(800, 600, "Asteroids");
	Engine engine(&game_window, "../../../res/runtime/properties.mdl");
	KeyListener key_listener;
	MouseListener mouse_listener;
	Random random;
	
	std::vector<Asteroid> asteroid_list;
	TrivialFunctor render_asteroid_list([&](){for(auto& asteroid : asteroid_list)asteroid.render(engine.camera, &engine.default_shader, game_window.get_width(), game_window.get_height());});
	engine.add_update_command(&render_asteroid_list);
	
	asteroids::speed = tz::util::cast::from_string<float>(MDLF(RawFile(engine.get_properties().get_tag("resources"))).get_tag("speed"));
	engine.register_listener(key_listener);
	engine.register_listener(mouse_listener);
	
	Font example_font("../../../res/runtime/fonts/upheaval.ttf", 25);
	TextLabel text(0.0f, 0.0f, Vector4F(1, 1, 1, 1), {}, Vector3F(0, 0, 0), example_font, "Lives: {} {} {} {} {}", engine.default_gui_shader);
	game_window.add_child(&text);
	
	asteroid_list.push_back({engine, Vector3F(), Vector3F(), Vector3F(10, 10, 10)});
	
	float rotational_speed = tz::util::cast::from_string<float>(engine.get_resources().get_tag("rotational_speed"));
	while(!engine.get_window().is_close_requested())
	{
		engine.update(0);
		if(engine.is_update_due())
		{
			if(key_listener.is_key_pressed("W"))
				engine.camera.position += engine.camera.forward() * asteroids::speed;
			if(key_listener.is_key_pressed("S"))
				engine.camera.position += engine.camera.backward() * asteroids::speed;
			if(key_listener.is_key_pressed("A"))
				engine.camera.position += engine.camera.left() * asteroids::speed;
			if(key_listener.is_key_pressed("D"))
				engine.camera.position += engine.camera.right() * asteroids::speed;
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