#pragma once

#include <vector>
#include <functional>
#include <algorithm>
#include <math.h>

std::vector<int> x;

template <typename T, typename U>
static T vreduce(const std::vector<T>& vectorToReduce,
	const std::function<U(U, T)>& reduceFn,
	const U initialValue) {
	U result = initialValue;
	for (auto e : vectorToReduce) {
		result = reduceFn(result, e);
	}
	return result;
}

template <typename T, typename U>
static std::vector<U> vmap(const std::vector<T>& data, const std::function<U(const T)> mapper) {
	std::vector<U> result;
	for (auto e : data) {
		result.push_back(mapper(e));
	};
	return result;
}

template <typename T>
static std::vector<T> vfilter(const std::vector<T>& data, std::function<bool(const T)> filterFn) {
	std::vector<T> result = data;
	result.erase(remove_if(result.begin(), result.end(), [filterFn](T element) {
		return !filterFn(element);
		}), result.end());
	return result;
}


template <typename NUMBER>
static NUMBER percentile(const std::vector<NUMBER>& numbers, float percentileOf100)
{
	if (numbers.size() == 0) return 0;

	std::vector<NUMBER> sorted = numbers;
	uint lower = (uint) floor(numbers.size() * percentileOf100 / 100);
	uint upper = std::min<uint>((uint) ceil(numbers.size() * percentileOf100 / 100), ((uint)numbers.size())-1);
	std::sort(sorted.begin(), sorted.end(), [](NUMBER a, NUMBER b) { return a < b; });
	return (sorted[lower] + sorted[upper]) / 2;
}

template <typename NUMBER>
static NUMBER medianInPlace(std::vector<NUMBER> &numbers)
{
	std::sort(numbers.begin(), numbers.end(), [](NUMBER a, NUMBER b) { return a < b; });
	if (numbers.size() == 0) {
		return 0;
	} else if (numbers.size() % 2 > 0) {
		return numbers[numbers.size() / 2];
	} else {
		auto c = numbers.size() / 2;
		return (numbers[c] + numbers[c-1]) / 2;
	}
}


template <typename NUMBER>
static NUMBER median(const std::vector<NUMBER> &numbers)
{
	std::vector<NUMBER> sortable(numbers);
	return medianInPlace(sortable);
}


template <typename NUMBER>
static NUMBER bimodalThreshold(
	const std::vector<NUMBER>& numbers,
	size_t minCountBelowThreshold = 1,
	size_t minCountAboveThreshold = 1
) {
	// Must be at least 1, but cannot be greater than the last element
	size_t minIndex = MIN(MAX(1, minCountBelowThreshold), numbers.size() - 1);
	size_t maxIndex = numbers.size() - MAX(1, minCountAboveThreshold);
	if (maxIndex <= minIndex) {
		// There's no valid range to search
		return 0;
	}

	NUMBER maxDistanceFound = 0;
	size_t indexOfMaxDistanceFound = 1;
	std::vector<NUMBER> sorted = numbers;
	std::sort(sorted.begin(), sorted.end(), [](NUMBER a, NUMBER b) { return a < b; });

	for (size_t i = minIndex; i <= maxIndex; i++) {
		const NUMBER distance = sorted[i] - sorted[i - 1];
		if (distance > maxDistanceFound) {
			maxDistanceFound = distance;
			indexOfMaxDistanceFound = i;
		}
	}
	// Put threshold halfway between the edges
	const NUMBER threshold = (sorted[indexOfMaxDistanceFound - 1] + sorted[indexOfMaxDistanceFound]) / 2;
	return threshold;
}

template <typename NUMBER>
static NUMBER bimodalThresholdWithFractionalDensities(
	const std::vector<NUMBER>& numbers,
	float minDensityBelowThreshold,
	float minDensityAboveThreshold
) {
	return bimodalThreshold(
		numbers,
		ceil(minDensityBelowThreshold * numbers.size()),
		ceil(minDensityAboveThreshold * numbers.size())
	);
}

template <typename NUMBER>
static NUMBER signedModReal(const NUMBER x, const NUMBER mod) {
	return x - (round(x / mod) * mod);
}

template <typename NUMBER>
static NUMBER reduceToSignedRange(const NUMBER x, const NUMBER magnitudeInPlusAndMinusDirection) {
	const NUMBER range = 2 * magnitudeInPlusAndMinusDirection;
	const float xModRange = x - (round(x / range) * range);
	if (xModRange > magnitudeInPlusAndMinusDirection) {
		return xModRange - range;
	}
	else if (xModRange <= -magnitudeInPlusAndMinusDirection) {
		return xModRange + range;
	}
	else {
		return xModRange;
	}
}

template <typename NUMBER>
static NUMBER distanceInCircularRangeFromNegativeNToNWithInputsInRange(const NUMBER aModN, const NUMBER bModN, const NUMBER N) {
	const NUMBER absDist = abs(aModN - bModN);
	return absDist * 2 > N ? N - absDist : absDist; 
}

template <typename NUMBER>
static NUMBER distanceInModCircularRangeFromNegativeNToN(const NUMBER a, const NUMBER b, const NUMBER N) {
	return distanceInCircularRangeFromNegativeNToNWithInputsInRange(reduceToSignedRange(a, N), reduceToSignedRange(b, N), N);
}


static float findPointOnCircularSignedNumberLineClosestToCenterOfMass(const std::vector<float> points, const float N) {
	if (points.size() == 0)
		return NAN;
	std::vector<float> pointsInRange = vmap<float, float>(points, [N](const float point) {
		return reduceToSignedRange(point, N);
		});
	float minDistance = std::numeric_limits<float>::max();
	size_t indexAtWhichMinDistanceFound = 0;
	for (size_t candidateIndex = 0; candidateIndex < pointsInRange.size(); candidateIndex++) {
		float distance = 0;
		float candidatePointModN = pointsInRange[candidateIndex];
		for (const float otherPointModN : pointsInRange) {
			distance += distanceInCircularRangeFromNegativeNToNWithInputsInRange(candidatePointModN, otherPointModN, N);
		}
		if (distance < minDistance) {
			minDistance = distance;
			indexAtWhichMinDistanceFound = candidateIndex;
		}
	}
	return points[indexAtWhichMinDistanceFound];
}