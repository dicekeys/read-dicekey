#pragma once

#include "vfunctional.h"

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
//   const size_t limitExclusive = MIN(toIndexExclusive, values.size());
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
	const size_t limitExclusive = MIN(toIndexExclusive, values.size());
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
	return MIN(errorAtLowerBound, errorAtUpperBound);
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
    sorted, MAX(1, minSamplesAtLowMode), sorted.size() - MAX(1, minSamplesAtHighMode)
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
