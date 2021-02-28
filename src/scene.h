#ifndef _SCENE_H_
#define _SCENE_H_
#include "hitable.h"
#include "job.h"


class RenderThread;
class RenderPixel;

class Scene {
public:
	Scene(void);
	vec3 hitColor(const ray& r, hitable* world, int depth);
	void setup();
	void render(const std::vector<Pixel>& pixels);
	vec3 render_pixel(int x, int y);
	int ns = 300;

	int nx = 600;
	int ny = 400;
	RenderThread* paintArea;
	void display_pixels(const list<RenderedInt>& toRender);

private:
	hitable* random_scene();
	hitable* world;
};

#endif