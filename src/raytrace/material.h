#ifndef MATERIAL_H_
#define MATERIAL_H_
#include "ray.h"
#include "hitable.h"
#include "drand48.h"

vec3 randomInUnitSphere() {
	vec3 p;
	do {
		p = 2. * vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
	} while (p.squared_length() >= 1.);
	return p;
}

vec3 reflect(const vec3& v, const vec3& n) {
	return v - 2 * dot(v, n) * n;
}

// https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf
bool refract(const vec3& v, const vec3& n, float n1DivideN2, vec3& refreacted) {
	vec3 unitV = unit_vector(v);
	float vDotN = dot(unitV, n);
	float discriminant = 1 - n1DivideN2 * n1DivideN2 * (1 - vDotN * vDotN);
	if (discriminant >= 0) {
		refreacted = n1DivideN2 * unitV + (-n1DivideN2 * vDotN - sqrt(discriminant)) * n;
		return true;
	}
	else
		return false;

}

float schlick(float cosine, float ref_idx) {
	float r0 = (1 - ref_idx) / (1 + ref_idx); // n1 = 1
	r0 = r0 * r0;
	float x = 1 - cosine;
	return r0 + (1 - r0) * x * x * x * x *x; // replace pow for performance
}

class material {
// 光线如何散射出
public:virtual bool scatter(const ray& rayIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const = 0;
};


class lambertian : public material {
public:
	lambertian(const vec3& a): albedo(a) {}
	virtual bool scatter(const ray& rayIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const {
		vec3 center = rec.p + rec.normal;
		vec3 target = center + randomInUnitSphere();
		scattered = ray(rec.p, target - rec.p);
		attenuation = albedo;
		return true;
	}
	vec3 albedo;
};


class metal : public material {
public:
	metal(const vec3& a, float f): albedo(a) { 
		if (f < 1) fuzz = f;
		else fuzz = 1;
	}
	virtual bool scatter(const ray& rayIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const {
		vec3 reflected = reflect(unit_vector(rayIn.direction()), rec.normal);
		scattered = ray(rec.p, reflected + fuzz * randomInUnitSphere());
		attenuation = albedo;
		return (dot(scattered.direction(), rec.normal) > 0);
	}
	vec3 albedo;
	float fuzz;
};


class dielectric : public material {
public:
	dielectric(float rIn) : refIdx(rIn) {}
	virtual bool scatter(const ray& rayIn, const hitRecord& rec, vec3& attenuation, ray& scattered) const {
		float n1DivideN2;
		vec3 outwardNormal;
		vec3 reflected = reflect(rayIn.direction(), rec.normal);
		vec3 refracted;
		float reflectance;
		float cosine;
		if (dot(rayIn.direction(), rec.normal) > 0) { // 从玻璃里面射向空气，空气 n = 1
			outwardNormal = -rec.normal; // normal方向为从hitpoint指向球心，与rec里的相反
			n1DivideN2 = refIdx;
			cosine = dot(rayIn.direction(), rec.normal) / rayIn.direction().length();
			cosine = sqrt(1 - n1DivideN2 * n1DivideN2*(1 - cosine * cosine));
		}
		else {
			outwardNormal = rec.normal;
			n1DivideN2 = 1. / refIdx;
			cosine = -dot(rayIn.direction(), rec.normal) / rayIn.direction().length();
		}
		if (refract(rayIn.direction(), outwardNormal, n1DivideN2, refracted)) {
			reflectance = schlick(cosine,  refIdx);
		}
		else {
			reflectance = 1.;
		}
		if (drand48() < reflectance) { // 使用随机数来决定是反射还是折射，这儿我们只有一条出射线，而不是将反射，折射都考虑
			scattered = ray(rec.p, reflected);

		}
		else {
			scattered = ray(rec.p, refracted);
		}

		attenuation = vec3(1., 1., 1.);
		return true;
	}
	float refIdx;
};
#endif