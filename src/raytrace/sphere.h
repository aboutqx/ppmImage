#ifndef SPHERE_H
#define SPHERE_H
#include "hitable.h"

class sphere : public hitable {
	public:
		sphere(vec3 cen, float r, material *m):center(cen), radius(r), materialPointer(m) {}
		virtual bool hit(const ray& r, float tMin, float tMax, hitRecord& rec) const;
		vec3 center;
		float radius;
		material *materialPointer;
};

bool sphere::hit(const ray& r, float tMin, float tMax, hitRecord& rec) const {
	vec3 oc = r.origin() - center;
	float a = dot(r.direction(), r.direction());
	float b = dot(oc, r.direction());
	float c = dot(oc, oc) - radius * radius;
	float discriminant = b * b - a * c;
	if (discriminant > 0) {
		float tmp = (-b - sqrt(discriminant)) / a;
		if (tmp<tMax && tmp > tMin) {
			rec.t = tmp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center) / radius;
			rec.materialPointer = materialPointer;
			return true;
		}
		tmp = (-b + sqrt(discriminant)) / a;
		if (tmp<tMax && tmp > tMin) {
			rec.t = tmp;
			rec.p = r.point_at_parameter(rec.t);
			rec.normal = (rec.p - center) / radius;
			rec.materialPointer = materialPointer;
			return true;
		}
	}
	return false;
}
#endif