#pragma once
#include "ray.h"
#include "aabb.h"
class material;

void get_sphere_uv(const vec3& p, double& u, double& v)
{
	//auto phi = atan2(p.z(), p.x());
	//auto theta = acos(p.y());
	//u = phi / (2 * pi) + 0.5;
	//v = (theta + pi / 2) / pi;

	auto phi = atan2(p.z(), p.x());
	auto theta = asin(p.y());
	u = 1 - (phi + pi) / (2 * pi);
	v = (theta + pi / 2) / pi;
}

struct hit_record
{
	point3 p;
	vec3 normal;
	shared_ptr<material> mat_ptr;
	double t;
	double u;
	double v;
	bool front_face;
	inline void set_face_noraml(const ray& r, const vec3& outward_normal)
	{
		front_face = dot(r.direction(), outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};

class hittable
{
public:
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
	virtual bool bounding_box(double t0, double t1, aabb& output_box) const = 0;
};