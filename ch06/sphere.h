#pragma once
#include "hittable.h"
#include "vec3.h"

class sphere : public hittable
{
public:
	point3 center;
	double radius;
	shared_ptr<material> mat_ptr;
	sphere() {}
	sphere(point3 cen, double r, shared_ptr<material> m) :center(cen), radius(r), mat_ptr(m) {}
	virtual bool hit(const ray& r, double tmin, double tmax, hit_record& rec) const;
	virtual bool bounding_box(double t0, double t1, aabb& output_box) const;
};

bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	vec3 oc = r.origin() - center;
	auto a = dot(r.direction(), r.direction());
	auto half_b = dot(oc, r.direction());
	auto c = oc.length_squared() - radius * radius;
	auto discriminator = half_b * half_b - a * c;

	if (discriminator > 0)
	{
		auto root = sqrt(discriminator);
		auto temp = (-half_b - root) / a;
		if (temp<t_max && temp>t_min)
		{
			rec.t = temp;
			rec.p = r.at(temp);
			vec3 outward_normal = (rec.p - center) / radius;
			rec.set_face_noraml(r, outward_normal);
			get_sphere_uv((rec.p - center) / radius, rec.u, rec.v);
			rec.mat_ptr = mat_ptr;
			return true;
		}
		temp = (-half_b + root) / a;
		if (temp<t_max && temp>t_min)
		{
			rec.t = temp;
			rec.p = r.at(temp);
			vec3 outward_normal = (rec.p - center) / radius;
			rec.set_face_noraml(r, outward_normal);
			get_sphere_uv((rec.p - center) / radius, rec.u, rec.v);
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}
	return false;
}
bool sphere::bounding_box(double t0, double t1, aabb& output_box) const
{
	output_box = aabb(center - vec3(radius, radius, radius),
					  center + vec3(radius, radius, radius));
	return true;
}
