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

color ray_color(const ray& r, const hittable& world, int depth)
{
	hit_record rec;
	if (depth <= 0)
		return color(0, 0, 0);
	if (world.hit(r, 0.001, infinity, rec))
	{
		ray scattered;
		color attenuation;
		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered))
			return attenuation * ray_color(scattered, world, depth - 1);
		return color(0, 0, 0);
	}
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

	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
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

hittable_list two_perlin_spheres()
{
	hittable_list objects;

	auto pertext = make_shared<noise_texture>(4);

	objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

	return objects;
}

int main()
{
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 384;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	int channel = 3;

	auto world = two_perlin_spheres();//random_scene();

	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10.0;
	auto aperture = 0.0;
	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
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
				pixel_color += ray_color(r, world, max_depth);
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