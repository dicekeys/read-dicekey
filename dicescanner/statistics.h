#pragma once

#include <math.h>
#include <algorithm>
#include "vfunctional.h"

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


template <typename NUMBER>
inline NUMBER medianInRange(
	const std::vector<NUMBER>& sorted,
	const size_t fromIndexInclusive,
	const size_t toIndexExclusive
) {
  assert(toIndexExclusive > fromIndexInclusive);
  const size_t numValues = toIndexExclusive - fromIndexInclusive;
	return (numValues & 1) ?
		// Odd number of samples, so median is the one in the middle
		(sorted[fromIndexInclusive + (numValues / 2)]) :
		// Even number of samples, so median is mean of the two in the middle
		(
			(
				sorted[fromIndexInclusive + (numValues / 2) - 1] +
				sorted[fromIndexInclusive + (numValues / 2) + 0]
			) / 2
		);
}

// template <typename NUMBER>
// inline unsigned long sumOfSquaresOfDifferenceForValuesInArrayRange(
// 	const std::vector<NUMBER>& values,
//   const NUMBER expectedValue,
// 	const size_t fromIndexInclusive,
// 	const size_t toIndexExclusive
// ) {
//   // the sum of the squares of the differences between the observed
//   // values and the expected values
//   unsigned long error = 0;
//   const size_t limitExclusive = std::min(toIndexExclusive, values.size());
//   for (size_t i = fromIndexInclusive; i < limitExclusive; i++) {
//     // the error is the square of the difference
//     const unsigned long difference = (abs(expectedValue - values[fromIndexInclusive]));
//     error += (difference * difference);
//   }
//   return error;
// }


template <typename NUMBER>
inline double sumOfDifferenceSquaresInRange(
	const std::vector<NUMBER>& values,
	const size_t fromIndexInclusive,
	const size_t toIndexExclusive,
	const double expectedValue
) {
	// the sum of the squares of the differences between the observed
	// values and the expected values
	double error = 0;
	const size_t limitExclusive = std::min(toIndexExclusive, values.size());
	for (size_t i = fromIndexInclusive; i < limitExclusive; i++) {
		// the error is the square of the difference
		const double difference = expectedValue - double(values[i]);
		error += (difference * difference);
	}
	return error;
}


template <typename NUMBER>
static double findMinimalErrorForRange(
	const std::vector<NUMBER>& numbers,
	const size_t fromIndexInclusive,
	const size_t toIndexExclusive,
	double centerOfMassLowerBound,
	double centerOfMassUpperBound,
	const double allowablePositionError = 0.1
) {
	double errorAtLowerBound = sumOfDifferenceSquaresInRange(numbers, fromIndexInclusive, toIndexExclusive, centerOfMassLowerBound);
	double errorAtUpperBound = sumOfDifferenceSquaresInRange(numbers, fromIndexInclusive, toIndexExclusive, centerOfMassUpperBound);

	while (centerOfMassUpperBound - centerOfMassLowerBound > allowablePositionError) {
		const double oneThirdPoint = centerOfMassLowerBound + ((centerOfMassUpperBound - centerOfMassLowerBound) / 3.0);
		const double twoThirdsPoint = centerOfMassLowerBound + ((centerOfMassUpperBound - centerOfMassLowerBound) * 2.0 / 3.0);
		const double errorAtOneThirdPoint = sumOfDifferenceSquaresInRange(numbers, fromIndexInclusive, toIndexExclusive, oneThirdPoint);
		const double errorAtTwoThirdsPoint = sumOfDifferenceSquaresInRange(numbers, fromIndexInclusive, toIndexExclusive, twoThirdsPoint);
		if (errorAtOneThirdPoint < errorAtTwoThirdsPoint) {
			centerOfMassUpperBound = twoThirdsPoint;
			errorAtUpperBound = errorAtTwoThirdsPoint;
		}
		else {
			centerOfMassLowerBound = oneThirdPoint;
			errorAtLowerBound = errorAtOneThirdPoint;
		}
	}
	return std::min(errorAtLowerBound, errorAtUpperBound);
}



template <typename NUMBER>
static double errorAtBimodalSeparationIndex(
	const std::vector<NUMBER>& sorted,
	const size_t firstHighModeIndex
) {
	// const NUMBER lowModeMedian = medianInRange(sorted, 0, firstHighModeIndex);
	// const NUMBER highModeMedian = medianInRange(sorted, firstHighModeIndex, sorted.size());
	// const unsigned long lowModeError = sumOfSquaresOfDifferenceForValuesInArrayRange(sorted, lowModeMedian, 0, firstHighModeIndex);
	// const unsigned long highModeError = sumOfSquaresOfDifferenceForValuesInArrayRange(sorted, highModeMedian, firstHighModeIndex, sorted.size());
	const double lowModeError = findMinimalErrorForRange(sorted, 0, firstHighModeIndex, sorted[0], sorted[firstHighModeIndex]);
	const double highModeError = findMinimalErrorForRange(sorted, firstHighModeIndex, sorted.size(), sorted[firstHighModeIndex], sorted[sorted.size()-1]);
	return lowModeError + highModeError;
}

template <typename NUMBER>
static size_t findFirstIndexOfHighModeInBimodalDistibution(
	const std::vector<NUMBER>& sorted,
	size_t minIndexOfHighModeStartInclusive,
	size_t maxIndexOfHighModeStartInclusive
) {
  assert(sorted.size() > 0);
  assert(minIndexOfHighModeStartInclusive <= maxIndexOfHighModeStartInclusive);

  if (minIndexOfHighModeStartInclusive == maxIndexOfHighModeStartInclusive) {
    return minIndexOfHighModeStartInclusive;
  }

  const size_t lowCenter = (minIndexOfHighModeStartInclusive + maxIndexOfHighModeStartInclusive) / 2;
  const size_t highCenter = lowCenter + 1;
  const auto errorAtLowCenter = errorAtBimodalSeparationIndex(sorted, lowCenter);
  const auto errorAtHighCenter = errorAtBimodalSeparationIndex(sorted, highCenter);

  if (errorAtLowCenter < errorAtHighCenter) {
		return findFirstIndexOfHighModeInBimodalDistibution(sorted, minIndexOfHighModeStartInclusive, lowCenter);
  } else {
    return findFirstIndexOfHighModeInBimodalDistibution(sorted, highCenter, maxIndexOfHighModeStartInclusive);
  }
}

/*
Given an array of numbers presumably drawn from a binomial distribution,
calculate a threshold value above which points can be assumed to belong
to the high mode and below which points can be assumed to belong to the lower mode.

The first parameter is an vector of numbers (unsorted is fine).
The second parameter specifies the number of values that must be below the threshold.
The third parameters specifies the number of values that must be above the threshold.

The threshold is found by identified by determining the index at which the high mode
samples start.  This is done by searching for the index at which we can have
two modes with the lowest possible errors (as defined by the sum of the squares
of the distances between each point and the closest modal point.  Modal points
are found through numerical search.)
The threshold is the mean of the highest value associated with the low mode
and the lowest value associated with the high mode (the halfway point between them.)
*/
template <typename NUMBER>
static NUMBER bimodalThreshold(
	const std::vector<NUMBER>& numbers,
  size_t minSamplesAtLowMode = 1,
	size_t minSamplesAtHighMode = 1
) {
  assert(numbers.size() > 1);
	std::vector<NUMBER> sorted(numbers);
	std::sort(sorted.begin(), sorted.end(), [](NUMBER a, NUMBER b) { return a < b; });

  const size_t firstIndexAtHighMode = findFirstIndexOfHighModeInBimodalDistibution(
    sorted, std::max((size_t)1, minSamplesAtLowMode), sorted.size() - std::max((size_t)1, minSamplesAtHighMode)
  );

  const NUMBER threshold = (sorted[firstIndexAtHighMode -1] + sorted[firstIndexAtHighMode]) / 2;
  return threshold;
}

/*
Given an array of numbers presumably drawn from a binomial distribution,
calculate a threshold between the two modes.

The first parameter is an vector of numbers.
The second parameter specifies the percent of values that must be below the threshold.
The third parameters specifies the percent of values that must be below the threshold.

The threshold is found by identifying the biggest gap in the sorted values
between the two thresholds.
*/
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


static float findAndValidateMeanDifference(const std::vector<float> sortedValues, float minBoundEdgeRange = 5.0f)
{
	if (sortedValues.size() < 2) {
		return NAN;
	}
	const float mean_difference = (sortedValues[sortedValues.size() -1] - sortedValues[0]) / float(sortedValues.size() - 1);
	const float abs_mean_difference = abs(mean_difference);
	// Ensure all delta_x and delta_y values are within 5% of the mean, though always
	// allow up to a minimum error since vertical/horizontal lines will have no delta_x/delta_y,
	// but could have a few pixel variation due to measurement errors.
	const float mean_bound_low = std::min(abs_mean_difference - minBoundEdgeRange, abs_mean_difference * 0.95f);
	const float mean_bound_high = std::max(abs_mean_difference + minBoundEdgeRange, abs_mean_difference * 1.05f);
	// Ensure all the delta_x and delta_y values are close to the mean
	bool allDistancesAreCloseToTheMeanDistance = true;
	for (int d = 1; d < sortedValues.size() && allDistancesAreCloseToTheMeanDistance; d++) {
		float difference = abs(sortedValues[d] - sortedValues[d-1]);
		float abs_difference = abs(difference);
		allDistancesAreCloseToTheMeanDistance &= 
			(mean_bound_low < abs_difference) && (abs_difference < mean_bound_high);
	}
	if (!allDistancesAreCloseToTheMeanDistance) {
		return NAN;
	}
	return mean_difference;
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
	return std::min(distanceWithinCircle, distAroundCircle);
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
	std::vector<float> pointsInRange = vmap<float, float>(points, [R](const float *point) {
		return reduceToSignedRange(*point, R);
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
