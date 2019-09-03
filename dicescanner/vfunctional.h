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
static U vreduce(const std::vector<T>& vectorToReduce,
	const std::function<U(U, const T*)>& reduceFn,
	const U initialValue
) {
	U result = initialValue;
	for (size_t i = 0; i < vectorToReduce.size(); i++) {
		result = reduceFn(result, &(vectorToReduce[i]));
	}
	//for (auto e : vectorToReduce) {
	//	result = reduceFn(result, e);
	//}
	return result;
}

/*
A function similar to JavaSript's map, used to transfer a vector of type T
to a vector of type U.  Unlike the JavaScript equivalent, the lambda function
you provide does not receive the index of the item.
*/
template <typename T, typename U>
static std::vector<U> vmap(const std::vector<T>& data, const std::function<U(const T*)> mapper) {
	std::vector<U> result;
	for (T e : data) {
		result.push_back(mapper(&e));
	};
	return result;
}

/*
A function similar to JavaScript's filter, which takes a vector of items of type T
and returns a vector with only those members for which the filter function returns true.
It preserves the order of the items that pass the filter function.
*/
template <typename T>
static std::vector<T> vfilter(const std::vector<T>& data, std::function<bool(const T*)> filterFn) {
	std::vector<T> result = data;
	result.erase(remove_if(result.begin(), result.end(), [filterFn](T element) {
		return !filterFn(&element);
		}), result.end());
	return result;
}


static char dashIfNull(char c) { return c == '\0' ? '-' : c; }
