#pragma once

inline int countOneBits(unsigned int x)
{
	int count = 0;
	while (x > 0) {
		count += (x & 1);
		x >>= 1;
	}
	return count;
}

inline int hammingDistance(unsigned int a, unsigned int b) {
	return countOneBits(a ^ b);
}
