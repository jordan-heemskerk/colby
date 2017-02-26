#include <colby/hash.hpp>
#include <opencv2/core/types.hpp>
#include <catch.hpp>

namespace colby {
namespace test {
namespace {

SCENARIO("std::hash is specialized for cv::Point objects","[colby][hash]") {
	GIVEN("A std::hash object for cv::Point objects") {
		std::hash<cv::Point> hasher;
		WHEN("Two identical cv::Point objects are hashed") {
			cv::Point p(0,0);
			auto a = hasher(p);
			auto b = hasher(p);
			THEN("The hashes are identical") {
				CHECK(a == b);
			}
		}
		WHEN("Two different cv::Point objects are hashed") {
			auto a = hasher({0,0});
			auto b = hasher({1,1});
			THEN("The hashes are different") {
				CHECK(a != b);
			}
		}
	}
}

}
}
}
