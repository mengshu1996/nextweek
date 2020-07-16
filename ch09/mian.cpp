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

hittable_list cornell_smoke() {
	hittable_list objects;

	auto red = make_shared<lambertian>(make_shared<solid_color>(.65, .05, .05));
	auto white = make_shared<lambertian>(make_shared<solid_color>(.73, .73, .73));
	auto green = make_shared<lambertian>(make_shared<solid_color>(.12, .45, .15));
	auto light = make_shared<diffuse_light>(make_shared<solid_color>(7, 7, 7));

	objects.add(make_shared<flip_face>(make_shared<yz_rect>(0, 555, 0, 555, 555, green)));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
	objects.add(make_shared<flip_face>(make_shared<xz_rect>(0, 555, 0, 555, 555, white)));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	objects.add(make_shared<flip_face>(make_shared<xy_rect>(0, 555, 0, 555, 555, white)));

	shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));

	shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, vec3(130, 0, 65));

	objects.add(make_shared<constant_medium>(box1, 0.01, make_shared<solid_color>(0, 0, 0)));
	objects.add(make_shared<constant_medium>(box2, 0.01, make_shared<solid_color>(1, 1, 1)));

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

	auto world = cornell_smoke();

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