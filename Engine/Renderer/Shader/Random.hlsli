#ifndef __RANDOM_INCLUDED__
#define __RANDOM_INCLUDED__

struct Random {
	uint value;
	uint seed;
};

struct Randomf {
	float value;
	uint seed;
};

Random rnd(uint seed)
{
	Random re;

	seed = (seed ^ 61) ^ (seed >> 16);
  seed *= 9;
  seed = seed ^ (seed >> 4);
  seed *= 0x27d4eb2d;
  seed = seed ^ (seed >> 15);
	re.value = seed;
	
	seed = (seed ^ 61) ^ (seed >> 16);
  seed *= 9;
  seed = seed ^ (seed >> 4);
  seed *= 0x27d4eb2d;
  seed = seed ^ (seed >> 15);
	re.seed = seed;

	return re;
}

Randomf rnd01(uint seed) {
	Random re = rnd(seed);
	const float MAX_UINT = 4294967296.0;

	Randomf ref;
	ref.seed = re.seed;
	ref.value = (float)re.value / MAX_UINT;

	return ref;
}

Randomf rrange(float min, float max, uint seed) {
	Randomf re = rnd01(seed);

	re.value = re.value * (max - min) + min;
	
	return re;
}


bool checkChance(inout Randomf r, float chance) {
	r = rnd01(r.seed);

	return r.value < chance;
}

#endif
