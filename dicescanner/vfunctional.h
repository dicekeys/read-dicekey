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
static std::vector<U> vmap(const std::vector<T>& data, const std::function<U(T)> mapper) {
	std::vector<U> result;
	for (auto e : data) {
		result.push_back(mapper(e));
	};
	return result;
}

template <typename T>
static std::vector<T> vfilter(const std::vector<T>& data, std::function<bool(T)> filterFn) {
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
static NUMBER median(const std::vector<NUMBER> &numbers)
{
	std::vector<NUMBER> sorted = numbers;
	std::sort(sorted.begin(), sorted.end(), [](NUMBER a, NUMBER b) { return a < b; });
	if (sorted.size() == 0) {
		return 0;
	} else if (sorted.size() % 2 > 0) {
		return sorted[sorted.size() / 2];
	} else {
		auto c = sorted.size() / 2;
		return (sorted[c] + sorted[c-1]) / 2;
	}
}

template <typename NUMBER>
static NUMBER bimodalThreshold(
	const std::vector<NUMBER>& numbers,
	float minDensityBelowThreshold = 0.0f,
	float minDensityAboveThreshold = 0.0f
) {
	if (numbers.size() < 2) return 0;

	NUMBER maxDistanceFound = 0;
	size_t indexOfMaxDistanceFound = 1;
	std::vector<NUMBER> sorted = numbers;
	std::sort(sorted.begin(), sorted.end(), [](NUMBER a, NUMBER b) { return a < b; });
	size_t minIndex = size_t(ceil(sorted.size() * minDensityBelowThreshold));
	size_t maxIndex = MIN(size_t(ceil(sorted.size() * (1 - minDensityAboveThreshold))), sorted.size() - 1);
	for (size_t i = minIndex; i <= maxIndex; i++) {
		const NUMBER distance = sorted[i] - sorted[i - 1];
		if (distance > maxDistanceFound) {
			maxDistanceFound = distance;
			indexOfMaxDistanceFound = i;
		}
	}
	// Put threshold halfway between the edges
	const NUMBER threshold = sorted[indexOfMaxDistanceFound - 1] + (maxDistanceFound / 2);
	return threshold;
}