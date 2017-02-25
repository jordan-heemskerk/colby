#include <colby/algorithm.hpp>
#include <colby/hash.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <unordered_set>
#include <catch.hpp>

namespace colby {
namespace test {
namespace {

SCENARIO("colby::valid identifies cv::Point objects which are valid positions in a certain cv::Mat","[colby][algorithm][valid]") {
	GIVEN("An image") {
		cv::Mat mat(cv::Mat::zeros(2,2,CV_32FC3));
		WHEN("An invalid position is checked") {
			auto result = valid(mat,cv::Point(2,2));
			THEN("false is returned") {
				CHECK_FALSE(result);
			}
		}
		WHEN("A valid position is checked") {
			auto result = valid(mat,cv::Point(0,0));
			THEN("true is returned") {
				CHECK(result);
			}
		}
	}
}

SCENARIO("colby::neighbors generates callbacks for all neighbors in the four-neighborhood of a given point","[colby][algorithm][neighbors]") {
	GIVEN("An image") {
		cv::Mat mat(cv::Mat::zeros(2,2,CV_32FC3));
		WHEN("A position is checked") {
			std::unordered_set<cv::Point> points;
			neighbors(mat,cv::Point(0,0),[&] (auto p) {	points.insert(p);	});
			THEN("Callbacks are generated for the correct neighbors") {
				CHECK(points.size() == 2U);
				CHECK(points.count(cv::Point(1,0)) == 1U);
				CHECK(points.count(cv::Point(0,1)) == 1U);
			}
		}
	}
}

SCENARIO("colby::flood_fill may be used to select an area of pixels","[colby][algorithm][flood_fill]") {
	GIVEN("An image") {
		cv::Mat mat(cv::Mat::zeros(2,2,CV_32FC3));
		WHEN("colby::flood_fill is called thereupon with a functor which returns true unconditionally") {
			auto set = flood_fill(mat,cv::Point(0,0),[] (const auto &) noexcept {	return true;	});
			THEN("All pixels are included in the resulting set") {
				CHECK(set.size() == 4U);
				CHECK(set.count(cv::Point(0,0)) == 1U);
				CHECK(set.count(cv::Point(0,1)) == 1U);
				CHECK(set.count(cv::Point(1,0)) == 1U);
				CHECK(set.count(cv::Point(1,1)) == 1U);
			}
		}
	}
}

}
}
}
