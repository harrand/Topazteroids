#ifndef GUI_DISPLAY_HPP
#define GUI_DISPLAY_HPP
#include "gui.hpp"

/*
	A 2D plane rendered on the screen. Can contain any other gui element in its own region. Can also be given colour to easily show the region it encompasses.
*/
class Panel : public GUI
{
public:
	Panel(float x, float y, float width, float height, Vector4F colour, Shader& shader);
	Panel(const Panel& copy) = default;
	Panel(Panel&& move) = default;
	~Panel() = default;
	Panel& operator=(const Panel& rhs) = default;

	const Vector4F& get_colour() const;
	void set_colour(Vector4F colour);
	const Texture* get_texture() const;
	void set_texture(Texture* texture);
	void disable_texture();
	bool has_texture() const;
	virtual void update() override;
	virtual void destroy() override;
	virtual bool focused() const override{return false;}
	virtual bool is_window() const override{return false;}
	virtual bool is_mouse_sensitive() const override{return false;}
protected:
	Texture* texture;
	Vector4F colour;
	Mesh quad;
};

/*
	Very similar to a Panel, but has additional font-rendering applied. Use this to write text to the screen.
*/
class TextLabel : public Panel
{
public:
	TextLabel(float x, float y, Vector4F colour, std::optional<Vector4F> background_colour, std::optional<Vector3F> text_border_colour, Font font, const std::string& text, Shader& shader);
	TextLabel(const TextLabel& copy) = default;
	TextLabel(TextLabel&& move) = default;
	~TextLabel() = default;
	TextLabel& operator=(const TextLabel& rhs) = default;
	
	virtual void update() override;
	bool has_background_colour() const;
	bool has_text_border_colour() const;
	const Font& get_font() const;
	void set_font(Font font);
	const std::string& get_text() const;
	void set_text(const std::string& new_text);
	const Texture& get_texture() const;
	void set_texture(Texture texture);
private:
	using Panel::get_texture;
	using Panel::set_texture;
	using Panel::disable_texture;
	using Panel::has_texture;
	std::optional<Vector4F> background_colour;
	std::optional<Vector3F> text_border_colour;
	Font font;
	std::string text;
	Texture text_texture;
};


#endif