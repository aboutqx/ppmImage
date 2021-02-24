#include "wxraytracer.h"
int nx = 1200;
int ny = 800;


// ppmImage.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <fstream>
#include "sphere.h"
#include "hitableList.h"
#include "camera.h"
#include <float.h>
#include "drand48.h"
#include "material.h"


vec3 hitColor(const ray& r, hitable* world, int depth) {
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


hitable* random_scene() {

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

					list[i++] = new sphere(center, 0.2,

						new metal(vec3(0.5 * (1 + drand48()), 0.5 * (1 + drand48()), 0.5 * (1 + drand48())), 0.5 * drand48()));

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

void render() {

	//std::ofstream fout("image.ppm");
	int nx = 1200;
	int ny = 800;
	int ns = 10; // sample nums
	//fout << "P3\n" << nx << " " << ny << "\n255\n";

	/*hitable *list[5];
	list[0] = new sphere(vec3(0, 0, -1), .5, new lambertian(vec3(.8, .3, .3)));
	list[1] = new sphere(vec3(0, -100.5, -1), 100, new lambertian(vec3(.8, .8, .0)));
	list[2] = new sphere(vec3(1, 0, -1), .5, new metal(vec3(.8, .6, .2), 0.));
	list[3] = new sphere(vec3(-1, 0, -1), .5, new dielectric(1.5));
	list[4] = new sphere(vec3(-1, 0, -1), -.45, new dielectric(1.5));
	hitable *world = new hitableList(list, sizeof(list) / sizeof(*list));*/
	hitable* world = random_scene();

	vec3 lookfrom(13, 2, 3);
	vec3 lookat(0, 0, 0);
	float aperture = .1;
	float focusDist = 10.;
	camera cam(lookfrom, lookat, vec3(0, 1, 0), 20, float(nx) / float(ny), aperture, focusDist);

	int percentage_counter = 0;
	for (int y = ny - 1; y >= 0; y--) {
		for (int x = 0; x < nx; x++) {
			vec3 color(0, 0, 0);
			for (int i = 0; i < ns; i++) {
				float u = float(x + drand48()) / float(nx);
				float v = float(y + drand48()) / float(ny);
				ray r = cam.getRay(u, v);
				color += hitColor(r, world, 0);
			}
			color /= float(ns);
			color = vec3(sqrt(color[0]), sqrt(color[1]), sqrt(color[2])); // gama correction
			//int ir = int(255.99 * color[0]);
			//int ig = int(255.99 * color[1]);
			//int ib = int(255.99 * color[2]);
			//fout << ir << " " << ig << " " <<  ib  << "\n";

			paintArea->setPixel(x,y, color[0], color[1], color[2]);
				//float progress = 1.0 - float(y) / float(ny);
				//if (progress * 100 > percentage_counter) {
				//	std::cout << percentage_counter << "%" << std::endl;
				//	percentage_counter++;
		}
	}

	//fout.close();
	//std::cout << "已生成文件：image.ppm";

}

