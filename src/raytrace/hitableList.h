#ifndef HITABLELIST_H
#define HITABLELIST_H

#include "hitable.h"

class hitableList: public hitable {
public:
	hitableList(hitable **l, int len) { list = l; listSize = len; }
	virtual bool hit(const ray& r, float tMin, float tMax, hitRecord& rec) const;
	hitable **list; // 指针的指针，二维数组
	int listSize;
};

bool hitableList::hit(const ray& r, float tMin, float tMax, hitRecord& rec) const {
	hitRecord tmpRecord;
	bool hitAnything = false;
	double closestSofar = tMax;
	for (int i = 0; i < listSize; i++) {
		if (list[i]->hit(r, tMin, closestSofar, tmpRecord)) {
			hitAnything = true;
			closestSofar = tmpRecord.t;
			rec = tmpRecord;
		}
	}
	return hitAnything;
}
#endif