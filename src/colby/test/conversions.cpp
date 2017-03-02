#include <colby/conversions.hpp>
#include <opencv2/core/matx.hpp>
#include <cmath>
#include <catch.hpp>

namespace colby {
namespace test {
namespace {

constexpr float eps = 0.0001f;

SCENARIO("colby::bgr2lab converts RGB points into Lab points","[colby][conversions][bgr2lab]") {

	GIVEN("An arbitrary RGB color") {
		cv::Vec3b bgr(145,38,45);
		WHEN("It is converted to Lab") {
			auto lab = bgr2lab(bgr);
			THEN("The correct Lab color is returned") {
				CHECK(std::abs(lab[0] - (23.6332172123127f)) < eps);
				CHECK(std::abs(lab[1] - (37.60780721052504f)) < eps);
				CHECK(std::abs(lab[2] - (-57.61918645116327f)) < eps);
			}
		}
	}

	GIVEN("The RGB color of black") {
		cv::Vec3b bgr(0,0,0);
		WHEN("It is converted to Lab") {
			auto lab = bgr2lab(bgr);
			THEN("The correct Lab color black is returned") {
				CHECK(std::abs(lab[0] - (0.f)) < eps);
				CHECK(std::abs(lab[1] - (0.f)) < eps);
				CHECK(std::abs(lab[2] - (0.f)) < eps);
			}
		}
	}

	GIVEN("The RGB color of white") {
		cv::Vec3b bgr(255,255,255);
		WHEN("It is converted to Lab") {
			auto lab = bgr2lab(bgr);
			THEN("The correct Lab color white is returned") {
				CHECK(std::abs(lab[0] - (100.f)) < eps);
				CHECK(std::abs(lab[1] - (0.00526049995830391f)) < eps);
				CHECK(std::abs(lab[2] - (-0.010408184525267927f)) < eps);
			}
		}
	}

	GIVEN("A cv::Mat of RGB values") {
		cv::Mat bgr(cv::Mat::zeros(2,3,CV_8UC3));
		bgr.at<cv::Vec3b>(0,2) = cv::Vec3b(145,38,45);
		bgr.at<cv::Vec3b>(1,0) = cv::Vec3b(255,255,255);
		WHEN("It is converted to Lab") {
			auto lab = bgr2lab(bgr);
			THEN("Each value of the cv::Mat is converted correctly") {

				auto black = lab.at<cv::Vec3f>(0,0);
				CHECK(std::abs(black[0]) < eps);
				CHECK(std::abs(black[1]) < eps);
				CHECK(std::abs(black[2]) < eps);

				auto arbitrary = lab.at<cv::Vec3f>(0,2);
				CHECK(std::abs(arbitrary[0] - (23.6332172123127f)) < eps);
				CHECK(std::abs(arbitrary[1] - (37.60780721052504f)) < eps);
				CHECK(std::abs(arbitrary[2] - (-57.61918645116327f)) < eps);

				auto white = lab.at<cv::Vec3f>(1,0);
				CHECK(std::abs(white[0] - (100.f)) < eps);
				CHECK(std::abs(white[1] - (0.00526049995830391f)) < eps);
				CHECK(std::abs(white[2] - (-0.010408184525267927f)) < eps);

			}
		}
	}
}

SCENARIO("colby::lab2bgr converts Lab points into RGB points","[colby][conversions][lab2bgr]") {

	GIVEN("An arbitrary Lab color") {
		cv::Vec3f lab(34.54f,10.8f,-3.2f);
		WHEN("It is converted to RGB") {
			auto bgr = lab2bgr(lab);
			THEN("The correct RGB color is returned") {
				CHECK(std::abs(int(bgr[2]) - 96) < 2);
				CHECK(std::abs(int(bgr[1]) - 75) < 2);
				CHECK(std::abs(int(bgr[0]) - 87) < 2);
			}
		}
	}

	GIVEN("The Lab color black") {
		cv::Vec3f lab(0.f, 0.f, 0.f);
		WHEN("It is converted to RGB") {
			auto bgr = lab2bgr(lab);
			THEN("The correct RGB color is returned") {
				CHECK(std::abs(int(bgr[2])) < 2);
				CHECK(std::abs(int(bgr[1])) < 2);
				CHECK(std::abs(int(bgr[0])) < 2);
			}
		}
	}

	GIVEN("The Lab color white") {
		cv::Vec3f lab(100.f, 0.f, 0.f);
		WHEN("It is converted to RGB") {
			auto bgr = lab2bgr(lab);
			THEN("The correct RGB color is returned") {
				CHECK(std::abs(int(bgr[2]) - 255) < 2);
				CHECK(std::abs(int(bgr[1]) - 255) < 2);
				CHECK(std::abs(int(bgr[0]) - 255) < 2);
			}
		}
	}

	GIVEN("A cv::Mat of Lab values") {
		cv::Mat lab(cv::Mat::zeros(2,3,CV_32FC3));
		lab.at<cv::Vec3f>(0,2) = cv::Vec3f(34.54f,10.8f,-3.2);
		lab.at<cv::Vec3f>(1,0) = cv::Vec3f(100.f,0.f,0.f);
		WHEN("It is converted to Lab") {
			auto bgr = lab2bgr(lab);
			THEN("Each value of the cv::Mat is converted correctly") {

				auto && black = bgr.at<cv::Vec3b>(0,0);
				CHECK(std::abs(int(black[2])) < 2);
				CHECK(std::abs(int(black[1])) < 2);
				CHECK(std::abs(int(black[0])) < 2);

				auto && arbitrary = bgr.at<cv::Vec3b>(0,2);
				CHECK(std::abs(int(arbitrary[2]) - 96) < 2);
				CHECK(std::abs(int(arbitrary[1]) - 75) < 2);
				CHECK(std::abs(int(arbitrary[0]) - 87) < 2);

				auto && white = bgr.at<cv::Vec3b>(1,0);
				CHECK(std::abs(int(white[2]) - 255) < 2);
				CHECK(std::abs(int(white[1]) - 255) < 2);
				CHECK(std::abs(int(white[0]) - 255) < 2);

			}
		}
	}

}

}
}
}
