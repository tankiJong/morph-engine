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
	re.value = seed;

	const uint BIT_NOISE1 = 0xD2A80A23; // 0b1101'0010'1010'1000'0000'1010'0010'0011;
	const uint BIT_NOISE2 = 0xA884F197; // 0b1010'1000'1000'0100'1111'0001'1001'0111;
	const uint BIT_NOISE3 = 0x1B56C4E9; // 0b0001'1011'0101'0110'1100'0100'1110'1001;

	uint mangledBits = seed;
	mangledBits *= BIT_NOISE1;
	mangledBits += seed;
	mangledBits ^= (mangledBits >> 7);
	mangledBits += BIT_NOISE2;
	mangledBits ^= (mangledBits >> 8);
	mangledBits *= BIT_NOISE3;
	mangledBits ^= (mangledBits >> 11);

	re.seed = mangledBits;
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
