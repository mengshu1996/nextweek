#pragma once
#include "rtweekend.h"

class texture {
public:
	virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color : public texture
{
public:
	solid_color() {}
	solid_color(color c) : color_value(c) {}

	solid_color(double red, double green, double blue) :solid_color(color(red, green, blue)) {}

	virtual color value(double u, double v, const point3& p) const {
		return color_value;
	}
private:
	color color_value;
};

class checker_texture : public texture
{
public:
	checker_texture() {}
	checker_texture(shared_ptr<texture> t0, shared_ptr<texture> t1) :even(t0), odd(t1) {}

	virtual color value(double u, double v, const point3& p) const
	{
		auto sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
		if (sines < 0)
			return odd->value(u, v, p);
		else
			return even->value(u, v, p);

		//pbr 10.5.3
		//auto sines = (int)std::floor(p.x()) + (int)std::floor(p.y()) + (int)std::floor(p.z());
		//if (sines % 2 == 0)
		//	return odd->value(u, v, p);
		//else
		//	return even->value(u, v, p);
	}

	shared_ptr<texture> even;
	shared_ptr<texture> odd;
};