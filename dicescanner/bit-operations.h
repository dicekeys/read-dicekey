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

static unsigned int reverseBits(unsigned int bitsToReverse, unsigned int lengthInBits)
{
	unsigned int reversedBits = 0;
	for (unsigned int i = 0; i < lengthInBits; i++) {
		reversedBits <<= 1;
		if (bitsToReverse & 1) {
			reversedBits += 1;
		}
		bitsToReverse >>= 1;
	}
	return reversedBits;
}
