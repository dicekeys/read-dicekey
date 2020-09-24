//  © 2019 Stuart Edward Schechter (Github: @uppajung)

#pragma once

#include "cv.h"

inline cv::Mat copyRotatedRectangle(const cv::Mat &destImageMatrix, const cv::Mat &srcImageMatrix, cv::Point2f centerOfRectToBeCopiedFromSource, float angleInDegrees)
{
	// get rotation matrix for rotating the image around its center in pixel coordinates
	cv::Mat rot = cv::getRotationMatrix2D(centerOfRectToBeCopiedFromSource, angleInDegrees, 1.0);

	// adjust transformation matrix so that the center of the source rectangle
	// maps to the center of the destination rectangle
	rot.at<double>(0, 2) += destImageMatrix.size().width / 2.0 - centerOfRectToBeCopiedFromSource.x;
	rot.at<double>(1, 2) += destImageMatrix.size().height / 2.0 - centerOfRectToBeCopiedFromSource.y;

	// Copy the rectangle into the destination matrix and return it
	cv::warpAffine(srcImageMatrix, destImageMatrix, rot, destImageMatrix.size());
	return destImageMatrix;
}

/*
Capture a rectangle rotated at a given angle from an image and return a copy of the image
within that rectangle.

Positive angles result in counter-clockwise rotation.
*/
inline cv::Mat copyRotatedRectangle(const cv::Mat &srcImageMatrix, cv::Point2f centerOfRectToBeCopiedFromSource, float angleInDegrees, cv::Size sizeOfRectangleToCopied)
{
	// Allocate a destination matrix to store the copied rectangle
	cv::Mat destImageMatrix = cv::Mat(sizeOfRectangleToCopied, srcImageMatrix.type());

	return copyRotatedRectangle(destImageMatrix, srcImageMatrix, centerOfRectToBeCopiedFromSource, angleInDegrees);
}

// from: https://stackoverflow.com/questions/22041699/rotate-an-image-without-cropping-in-opencv-in-c
