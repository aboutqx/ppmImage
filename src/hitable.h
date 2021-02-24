#pragma once
#include "ray.h"

class material;

struct hitRecord {
	float t;
	vec3 p;
	vec3 normal;
	material *materialPointer;
};

class hitable {
	public:
		virtual bool hit(const ray& r,float tMin, float tMax, hitRecord& rec) const = 0;
};