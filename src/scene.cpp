#include "wxraytracer.h"
#include "scene.h"
#include <iostream>
#include <fstream>
#include "sphere.h"
#include "hitableList.h"
#include "material.h"
#include "camera.h"
#include "job.h"

camera *cam;

Scene::Scene() {

};
vec3 Scene::hitColor(const ray& r, hitable* world, int depth) {
	hitRecord rec;
	if (world->hit(r, 0.001, FLT_MAX, rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.materialPointer->scatter(r, rec, attenuation, scattered)) {
			return attenuation * hitColor(scattered, world, depth + 1);
		}
		else {
			return vec3(0., 0., 0.);
		}
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		float t = .5 * (unit_direction.y() + 1.);
		return (1. - t) * vec3(1., 1., 1.) + t * vec3(.5, .7, 1.);
	}
}


hitable* Scene::random_scene() {

	int n = 500;
	hitable** list = new hitable * [n + 1];

	list[0] = new sphere(vec3(0, -1000, 0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));

	int i = 1;

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = drand48();
			vec3 center(a + 0.9 * drand48(), 0.2, b + 0.9 * drand48());
			if ((center - vec3(4, 0.2, 0)).length() > 0.9) {

				if (choose_mat < 0.8) {  // diffuse
					list[i++] = new sphere(center, 0.2, new lambertian(vec3(drand48() * drand48(), drand48() * drand48(), drand48() * drand48())));
				}

				else if (choose_mat < 0.95) { // metal
					list[i++] = new sphere(center, 0.2,new metal(vec3(0.5 * (1 + drand48()), 0.5 * (1 + drand48()), 0.5 * (1 + drand48())), 0.5 * drand48()));
				}

				else {  // glass

					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new hitableList(list, i);

}

void Scene::setup() {
	//std::ofstream fout("image.ppm");
	//fout << "P3\n" << nx << " " << ny << "\n255\n";

	/*hitable *list[5];
	list[0] = new sphere(vec3(0, 0, -1), .5, new lambertian(vec3(.8, .3, .3)));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(.8, .8, .0)));
	list[2] = new sphere(vec3(1, 0, -1), .5, new metal(vec3(.8, .6, .2), 0.));
	list[3] = new sphere(vec3(-1, 0, -1), .5, new dielectric(1.5));
	list[4] = new sphere(vec3(-1, 0, -1), -.45, new dielectric(1.5));
	hitable *world = new hitableList(list, sizeof(list) / sizeof(*list));*/
	world = random_scene();

	vec3 lookfrom(13, 2, 3);
	vec3 lookat(0, 0, 0);
	float aperture = .1;
	float focusDist = 10.;
	cam = new camera(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, focusDist);

	//int percentage_counter = 0;
	//for (int y = ny - 1; y >= 0; y--) {
	//	for (int x = 0; x < nx; x++) {

	//		render_pixels(x, y);
	//		//fout << ir << " " << ig << " " <<  ib  << "\n";

	//		//paintArea->setPixel(x, ny-y-1, ir, ig, ib);
	//		//float progress = 1.0 - float(y) / float(ny);
	//		//if (progress * 100 > percentage_counter) {
	//		//	std::cout << percentage_counter << "%" << std::endl;
	//		//	percentage_counter++;
	//	}
	//}

	//fout.close();
	//std::cout << "已生成文件：image.ppm";
}
void Scene::render(const std::vector<Pixel>& pixels) {

	list<RenderedInt> toRender;
	RenderedInt pixel;
	for (unsigned int i = 0; i < pixels.size(); i++)
	{
		vec3 color = render_pixel(pixels[i].x, pixels[i].y);
		pixel.x = pixels[i].x;
		pixel.y = pixels[i].y;
		pixel.red = int(255.99 * color[0]);
		pixel.green = int(255.99 * color[1]);
		pixel.blue = int(255.99 * color[2]);
		toRender.push_back(pixel);
		display_pixels(toRender);
		toRender.clear();
	}
}

vec3 Scene::render_pixel(int x, int y) {
	vec3 color(0, 0, 0);
	for (int i = 0; i < ns; i++) {
		float u = float(x + drand48()) / float(nx);
		float v = float(y + drand48()) / float(ny);
		ray r = cam->getRay(u, v);
		color += hitColor(r, world, 0);
	}
	color /= float(ns);
	color = vec3(sqrt(color[0]), sqrt(color[1]), sqrt(color[2])); // gama correction

	return color;
}

void Scene::display_pixels(const list<RenderedInt>& toRender) {
	list<RenderedInt>::const_iterator it;
	vector<RenderPixel*> rendered;
	rendered.reserve(toRender.size());

	for (it = toRender.begin(); it != toRender.end(); it++)
	{
		//deep copy
		rendered.push_back(new RenderPixel);
		rendered.back()->x = it->x;
		rendered.back()->y = ny - it->y - 1;
		rendered.back()->red = it->red;
		rendered.back()->green = it->green;
		rendered.back()->blue = it->blue;
	}

	paintArea->setPixel(rendered);
	rendered.clear();
}