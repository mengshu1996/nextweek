#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "rtweekend.h"
#include "hittable_list.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "camera.h"
#include "material.h"
#include "aarec.h"
#include "box.h"
#include "bvh.h"
#include "constant_medium.h"

void avg_color(color& pixel_color, int samples_per_pixel)
{
	auto scale = 1.0 / samples_per_pixel;
	pixel_color[0] = sqrt(pixel_color[0] * scale);
	pixel_color[1] = sqrt(pixel_color[1] * scale);
	pixel_color[2] = sqrt(pixel_color[2] * scale);

	pixel_color[0] = static_cast<int>(256 * clamp(pixel_color[0], 0.0, 0.999));
	pixel_color[1] = static_cast<int>(256 * clamp(pixel_color[1], 0.0, 0.999));
	pixel_color[2] = static_cast<int>(256 * clamp(pixel_color[2], 0.0, 0.999));
}

color ray_color(const ray& r, const color& background, const hittable& world, int depth)
{
	hit_record rec;
	if (depth <= 0)
		return color(0, 0, 0);
	if (!world.hit(r, 0.001, infinity, rec))
		return background;

	ray scattered;
	color attenuation;
	color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

	if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered))
		return emitted;

	return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
	return color(0, 0, 0);

	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

hittable_list final_scene()
{
	hittable_list boxes1;
	auto ground = make_shared<lambertian>(make_shared<solid_color>(0.48, 0.83, 0.53));

	const int boxes_per_side = 20;
	for (int i = 0; i < boxes_per_side; i++) {
		for (int j = 0; j < boxes_per_side; j++) {
			auto w = 100;
			auto x0 = -1000.0 + i * w;
			auto z0 = -1000.0 + j * w;
			auto y0 = 0.0;
			auto x1 = x0 + w;
			auto z1 = z0 + w;
			auto y1 = random_double(1, 101);

			boxes1.add(make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
		}
	}

	hittable_list objects;

	objects.add(make_shared<bvh_node>(boxes1, 0, 1));

	auto light = make_shared<diffuse_light>(make_shared<solid_color>(7, 7, 7));
	objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

	auto center1 = point3(400, 400, 200);
	auto center2 = center1 + vec3(30, 0, 0);
	auto moving_sphere_material = make_shared<lambertian>(make_shared<solid_color>(0.7, 0.3, 0.1));
	objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

	objects.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
	objects.add(make_shared<sphere>(point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 10.0)));

	auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
	objects.add(boundary);
	objects.add(make_shared<constant_medium>(boundary, 0.2, make_shared<solid_color>(0.2, 0.4, 0.9)));
	boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
	objects.add(make_shared<constant_medium>(boundary, 0.0001, make_shared<solid_color>(1, 1, 1)));

	auto emat = make_shared<lambertian>(make_shared<image_texture>("earthmap.jpg"));
	objects.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
	auto pertext = make_shared<noise_texture>(0.1);
	objects.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));

	hittable_list boxes2;
	auto white = make_shared<lambertian>(make_shared<solid_color>(0.73, 0.73, 0.73));
	int ns = 1000;
	for (int j = 0; j < ns; j++) {
		boxes2.add(make_shared<sphere>(point3::random(0, 165), 10, white));
	}
	objects.add(make_shared<translate>(make_shared<rotate_y>(make_shared<bvh_node>(boxes2, 0.0, 1.0), 15), vec3(-100, 270, 395)));
		
	return objects;

}

int main()
{
	const auto aspect_ratio = 1.0 / 1.0;
	const int image_width = 600;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 1000;
	const int max_depth = 50;
	int channel = 3;

	auto world = final_scene();

	point3 lookfrom(478, 278, -600);
	point3 lookat(278, 278, 0);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10.0;
	auto aperture = 0.0;
	auto vfov = 40.0;
	color background(0, 0, 0);
	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
	unsigned char* data = new unsigned char[image_width * image_height * channel];
	for (int j = image_height - 1; j >= 0; --j)
	{
		for (int i = 0; i < image_width; ++i)
		{
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s)
			{
				
				auto u = double(i + random_double()) / (image_width - 1);
				auto v = double(j + random_double()) / (image_height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, background, world, max_depth);
			}
			avg_color(pixel_color, samples_per_pixel);
			data[(image_height - j - 1) * image_width * channel + i * channel] = pixel_color[0];
			data[(image_height - j - 1) * image_width * channel + i * channel + 1] = pixel_color[1];
			data[(image_height - j - 1) * image_width * channel + i * channel + 2] = pixel_color[2];
		}
	}
	stbi_write_jpg("nextwk.jpg", image_width, image_height, channel, data, 100);
	std::cout << "finish.\n";
	//system("PAUSE");
}