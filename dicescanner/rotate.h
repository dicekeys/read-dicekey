
#pragma once

#include <opencv2/opencv.hpp>

/*
Capture a rectangle rotated at a given angle from an image and return a copy of the image
within that rectangle.

Positive angles result in counter-clockwise rotation.
*/
static cv::Mat copyRotatedRectangle(cv::Mat srcImageMatrix, cv::Point2f centerOfRectToBeCopiedFromSource, float angleInDegrees, cv::Size sizeOfRectangleToCopied)
{
	// get rotation matrix for rotating the image around its center in pixel coordinates
	cv::Mat rot = cv::getRotationMatrix2D(centerOfRectToBeCopiedFromSource, angleInDegrees, 1.0);

	// adjust transformation matrix so that the center of the source rectangle
	// maps to the center of the destination rectangle
	rot.at<double>(0, 2) += sizeOfRectangleToCopied.width / 2.0 - centerOfRectToBeCopiedFromSource.x;
	rot.at<double>(1, 2) += sizeOfRectangleToCopied.height / 2.0 - centerOfRectToBeCopiedFromSource.y;

	// Allocate a destination matrix to store the copied rectangle
	cv::Mat destImageMatrix = cv::Mat(sizeOfRectangleToCopied, srcImageMatrix.type());

	// Copy the rectangle into the destination matrix and return it
	cv::warpAffine(srcImageMatrix, destImageMatrix, rot, sizeOfRectangleToCopied);
	return destImageMatrix;
}

// from: https://stackoverflow.com/questions/22041699/rotate-an-image-without-cropping-in-opencv-in-c
