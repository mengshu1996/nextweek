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

hittable_list random_scene()
{
	hittable_list world;
	
	auto checker = make_shared<checker_texture>(
		make_shared<solid_color>(0.2, 0.3, 0.1),
		make_shared<solid_color>(0.9, 0.9, 0.9));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

	for (int a = -1; a < 1; a++)
	{
		for (int b = -1; b < 1; b++)
		{
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9)
			{
				shared_ptr<material> sphere_material;
				if (choose_mat < 0.8)
				{
					//diffuse
					auto albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(make_shared<solid_color>(albedo));
					auto center2 = center + vec3(0, random_double(0, 0.5), 0);
					world.add(make_shared<moving_sphere>(center, center2, 0.0,1.0, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95)
				{
					//metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else
				{
					//glass
					sphere_material = make_shared<dielectric>(1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}
	
	auto material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));
	auto material2 = make_shared<lambertian>(make_shared<solid_color>(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));
	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	return world;
}

hittable_list simple_light() {
	hittable_list objects;

	auto pertext = make_shared<noise_texture>(4);
	objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

	auto difflight = make_shared<diffuse_light>(make_shared<solid_color>(4, 4, 4));
	objects.add(make_shared<sphere>(point3(0, 7, 0), 2, difflight));
	objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

	return objects;
}

hittable_list cornell_box()
{
	hittable_list objects;

	auto red = make_shared<lambertian>(make_shared<solid_color>(0.65, 0.05, 0.05));
	auto white = make_shared<lambertian>(make_shared<solid_color>(0.73, 0.73, 0.73));
	auto green = make_shared<lambertian>(make_shared<solid_color>(0.12, 0.45, 0.15));
	auto light = make_shared<diffuse_light>(make_shared<solid_color>(15, 15, 15));
	//light
	objects.add(make_shared<xz_rect>(213, 343, 227, 332, 554, light));
	//left
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
	//right
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	//down
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	//up
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	//back
	objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));
	return objects;
}

int main()
{
	const auto aspect_ratio = 1.0 / 1.0;
	const int image_width = 600;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	int channel = 3;

	auto world = cornell_box();

	point3 lookfrom(278, 278, -800);
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