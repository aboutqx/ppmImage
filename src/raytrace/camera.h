#ifndef CAMERA_H
#define CAMERA_H
#include "ray.h"
#include <corecrt_math_defines.h>
#include "drand48.h"

class camera {
public:
	camera(vec3 lookFrom, vec3 lookAt, vec3 up, float vfov, float aspect, float aperture, float focusDist) { // vectical fov
		lensRadius = aperture / 2;
		origin = lookFrom;
		float theta = vfov * M_PI / 180;
		float halfHeight = tan(theta / 2);
		float halfWidth = aspect * halfHeight;
		w = unit_vector(lookFrom - lookAt);
		u = unit_vector(cross(up, w));
		v = cross(w, u);

		//lower_left_corner = vec3(-halfWidth, -halfHeight, -1.);
		// 乘以focusDist保证夹角大小不变
		lower_left_corner = origin - halfWidth * u * focusDist - halfHeight * v * focusDist - w * focusDist;
		hVec = 2 * halfWidth * u * focusDist;
		vVec = 2 * halfHeight * v * focusDist;
	}
	ray getRay(float x, float y) {
		vec3 rd = lensRadius * randomInunitDisk();
		vec3 offset = u * rd.x() + v * rd.y();
		return ray(origin + offset, lower_left_corner + x * hVec + y * vVec - origin - offset);
	}
	vec3 lower_left_corner;
	vec3 hVec;
	vec3 vVec;
	vec3 origin;
	float lensRadius;
	vec3 u, v, w;
};
#endif