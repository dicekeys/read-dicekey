#pragma once

#include <vector>
#include <functional>
#include <algorithm>
#include <math.h>

/*
A function equivalent to JavaScript's reduce operator.

Takes a vector of items of type T and outputs a value of type U,
using a function that is called for each item of the vector (in order)
with the current element of type T and the current result of type U.
*/
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

/*
A function similar to JavaSript's map, used to transfer a vector of type T
to a vector of type U.  Unlike the JavaScript equivalent, the lambda function
you provide does not receive the index of the item.
*/
template <typename T, typename U>
static std::vector<U> vmap(const std::vector<T>& data, const std::function<U(const T)> mapper) {
	std::vector<U> result;
	for (auto e : data) {
		result.push_back(mapper(e));
	};
	return result;
}

/*
A function similar to JavaScript's filter, which takes a vector of items of type T
and returns a vector with only those members for which the filter function returns true.
It preserves the order of the items that pass the filter function.
*/
template <typename T>
static std::vector<T> vfilter(const std::vector<T>& data, std::function<bool(const T)> filterFn) {
	std::vector<T> result = data;
	result.erase(remove_if(result.begin(), result.end(), [filterFn](T element) {
		return !filterFn(element);
		}), result.end());
	return result;
}

/*
Given threei input values a, b, and c, returns the value that is common to at least
two of the input parameters (the majority). If a, b, and c are all different values,
0 is returned.
*/
template <typename T>
static T majorityOfThree(T a, T b, T c)
{
	if (a == b || a == c) {
		return a;
	}
	else if (b == c) {
		return b;
	}
	return 0;
}

/*
Given a vector of numbers, finds the value at a given percentile,
where the 0th percentile is the lowest number, 50th percentile is the median,
and the 100th percentile is the mix.

If the percentile falls between two values, the average those two values is returned.

The percentile should be specified as a float between 0 and 100.
*/
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

/*
Calculate the median of an array of numbers, first sorting the numbers.

The input array passed by reference and will be sorted by the call.
*/
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

/*
Calculate the median of an array of numbers (without modifying the array).
*/
template <typename NUMBER>
static NUMBER median(const std::vector<NUMBER> &numbers)
{
	std::vector<NUMBER> sortable(numbers);
	return medianInPlace(sortable);
}

/*
Reduces a number to a range (-R, R], which is a range of size 2R.
For numbers > R, this is equivlaent to subtracting Rr until the number falls in the range.
For numbers <= R, this is equivlaent to add Rr until the number falls into the range.
*/
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

/*
Given two numbers in a circular number line of range (-R, R], find the distance between these numbers
such that the distance is shortest distance around the circle between the two points.
In other words, it is the smaller of
  abs(a - b)
  2R - abs(a - b)
*/
template <typename NUMBER>
static NUMBER distanceInCircularRangeFromNegativeNToNWithInputsInRange(const NUMBER aModN, const NUMBER bModN, const NUMBER R) {
	const NUMBER distanceWithinCircle = abs(aModN - bModN);
	const NUMBER distAroundCircle = (2 * R) - distanceWithinCircle;
	return MIN(distanceWithinCircle, distAroundCircle);
}

/*
Given two numbers, reduce them to a circular number line of range (-R, R],
then find the distance between these numbers such that the distance is shortest
distance around the circle.
*/
template <typename NUMBER>
static NUMBER distanceInModCircularRangeFromNegativeNToN(const NUMBER a, const NUMBER b, const NUMBER N) {
	return distanceInCircularRangeFromNegativeNToNWithInputsInRange(reduceToSignedRange(a, N), reduceToSignedRange(b, N), N);
}

/*
Given a vector of points on a number range, reduce the to the range (-R, R],
the find the point within that range that is closest to all the other points
reduced to the range.

Useful for taking a range of angles and finding the average deviation from
a right angle (in range -45 to 45 degrees)
*/
static float findPointOnCircularSignedNumberLineClosestToCenterOfMass(const std::vector<float> points, const float R) {
	if (points.size() == 0)
		return NAN;
	std::vector<float> pointsInRange = vmap<float, float>(points, [R](const float point) {
		return reduceToSignedRange(point, R);
		});
	float minDistance = std::numeric_limits<float>::max();
	size_t indexAtWhichMinDistanceFound = 0;
	for (size_t candidateIndex = 0; candidateIndex < pointsInRange.size(); candidateIndex++) {
		float distance = 0;
		float candidatePointModN = pointsInRange[candidateIndex];
		for (const float otherPointModN : pointsInRange) {
			distance += distanceInCircularRangeFromNegativeNToNWithInputsInRange(candidatePointModN, otherPointModN, R);
		}
		if (distance < minDistance) {
			minDistance = distance;
			indexAtWhichMinDistanceFound = candidateIndex;
		}
	}
	return points[indexAtWhichMinDistanceFound];
}