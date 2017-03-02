/**
 *	\file
 */

#pragma once

#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace colby {

/**
 *	Converts a 32-bit floating point XYZ value to a 32-bit floating point value in CIELAB.
 *
 *	\param [in] lab
 *		The color in XYZ
 *	\returns
 *		The color in CIELAB
 */
inline cv::Vec3f xyz2lab(const cv::Vec3f xyz) noexcept {
	auto x = xyz[0]/95.047f;
	auto y = xyz[1]/100.f;
	auto z = xyz[2]/108.883f;

	x = x > 0.008856f ? std::pow(x, 1.f/3.f) : 7.787f*x+16.f/116.f;
	y = y > 0.008856f ? std::pow(y, 1.f/3.f) : 7.787f*y+16.f/116.f;
	z = z > 0.008856f ? std::pow(z, 1.f/3.f) : 7.787f*z+16.f/116.f;

	auto l = 116.f*y-16.f;
	auto a = 500.f*(x-y);
	auto b = 200.f*(y-z);

	return cv::Vec3f(l,a,b);
}

/**
 *	Converts a 32-bit floating point CIELAB value to a 32-bit floating point value in XYZ.
 *
 *	\param [in] lab
 *		The color in CIELAB
 *	\returns
 *		The color in XYZ
 */
inline cv::Vec3f lab2xyz(const cv::Vec3f lab) noexcept {
	auto l = lab[0];
	auto a = lab[1];
	auto b = lab[2];

	auto y = (l+16.f)/116.f;
	auto x = a/500.f+y;
	auto z = y-b/200.f;

	x = std::pow(x, 3.f)>0.008856f ? std::pow(x, 3.f)  : (x-16.f/116.f)/7.787f;
	y = std::pow(y, 3.f)>0.008856f ? std::pow(y, 3.f)  : (y-16.f/116.f)/7.787f;
	z = std::pow(z, 3.f)>0.008856f ? std::pow(z, 3.f)  : (z-16.f/116.f)/7.787f;

	x *= 95.047f;
	y *= 100.f;
	z *= 108.883f;

	return cv::Vec3f(x,y,z);
}

/**
 *	Converts an 8-bit BGR value to a 32-bit floating point value in XYZ.
 *
 *	\param [in] bgr
 *		The color in BGR
 *	\returns
 *		The color in XYZ
 */
inline cv::Vec3f bgr2xyz(const cv::Vec3b bgr) noexcept {
	auto r = bgr[2]/255.f;
	auto g = bgr[1]/255.f;
	auto b = bgr[0]/255.f;

	r = r>0.04045f ? std::pow(((r+0.055f)/1.055f), 2.4f) : r/12.92f;
	g = g>0.04045f ? std::pow(((g+0.055f)/1.055f), 2.4f) : g/12.92f;
	b = b>0.04045f ? std::pow(((b+0.055f)/1.055f), 2.4f) : b/12.92f;

	r *= 100.f;
	g *= 100.f;
	b *= 100.f;

	auto x = r*0.4124f+g*0.3576f+b*0.1805f;
	auto y = r*0.2126f+g*0.7152f+b*0.0722f;
	auto z = r*0.0193f+g*0.1192f+b*0.9505f;

	return cv::Vec3f(x,y,z);
}

/**
 *	Converts a 32-bit floating point XYZ value to an 8-bit BGR value.
 *
 *	\param [in] xyz
 *		The color in XYZ
 *	\returns
 *		The color in BGR
 */
inline cv::Vec3b xyz2bgr(const cv::Vec3f xyz) noexcept {
	auto x = xyz[0]/100.f;
	auto y = xyz[1]/100.f;
	auto z = xyz[2]/100.f;

	auto r = x*3.2406f+y*-1.5372f+z*-0.4986f;
	auto g = x*-0.9689f+y*1.8758f+z*0.0415f;
	auto b = x*0.0557f+y*-0.2040f+z*1.0570f;

	r = r > 0.0031308f ? 1.055f*std::pow(r, 1.f/2.4f)-0.055f : 12.92f*r;
	g = g > 0.0031308f ? 1.055f*std::pow(g, 1.f/2.4f)-0.055f : 12.92f*g;
	b = b > 0.0031308f ? 1.055f*std::pow(b, 1.f/2.4f)-0.055f : 12.92f*b;

	r *= 255.f;
	g *= 255.f;
	b *= 255.f;

	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;

	if (r > 255.f) r = 255.f;
	if (g > 255.f) g = 255.f;
	if (b > 255.f) b = 255.f;

	return cv::Vec3b(b,g,r);
}

/**
 *	Converts a 8-bit BGR value to 32-bit floating
 *	point CIELAB value.
 *
 *	\param [in] bgr
 *		The color in BGR
 *	\returns
 *		The color in CIELAB space
 */
inline cv::Vec3f bgr2lab (const cv::Vec3b bgr) noexcept {
	return xyz2lab(bgr2xyz(bgr));
}

/**
 *	Converts a 32-bit floating point CIELAB value
 *	to an 8-bit BGR value
 *
 *	\param [in] lab
 *		The color in CIELAB space
 *	\returns
 *		The color in BGR
 */
inline cv::Vec3b lab2bgr (const cv::Vec3f lab) noexcept {
	return xyz2bgr(lab2xyz(lab));
}

/**
 *	Converts a cv::Mat of 8-bit BGR values to 32-bit floating
 *	point CIELAB values.
 *
 *	\param [in] bgr
 *		A cv::Mat of BGR values
 *	\returns
 *		A cv::Mat of CIELAB values
 */
inline cv::Mat bgr2lab(const cv::Mat & bgr) {
	if (bgr.type() != CV_8UC3) throw std::logic_error("Expected 3 channel 8 bit image");
	cv::Mat retr(bgr.rows, bgr.cols, CV_32FC3);
	for (int i = 0; i < bgr.cols; ++i) {
		for (int j = 0; j < bgr.rows; ++j) {
			retr.at<cv::Vec3f>(j,i) = bgr2lab(bgr.at<cv::Vec3b>(j,i));
		}
	}
	return retr;
}

/**
 *	Converts a cv::Mat of 32-bit floating point CIELAB values
 *	to an 8-bit BGR values
 *
 *	\param [in] lab
 *		A cv::Mat of colors in CIELAB space
 *	\returns
 *		A cv::Mat of colors in BGR space
 */
inline cv::Mat lab2bgr(const cv::Mat & lab) {
	if (lab.type() != CV_32FC3) throw std::logic_error("Expected 3 channel 32 bit floating point image");
	cv::Mat retr(lab.rows, lab.cols, CV_8UC3);
	for (int i = 0; i < lab.cols; ++i) {
		for (int j = 0; j < lab.rows; ++j) {
			retr.at<cv::Vec3b>(j,i) = lab2bgr(lab.at<cv::Vec3f>(j,i));
		}
	} 
	return retr;
}

}
