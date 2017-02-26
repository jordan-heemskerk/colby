add_library(colby SHARED
	color_by_numbers.cpp
	image_factory.cpp
	sp3000_color_by_numbers.cpp
	sp3000_color_by_numbers_observer.cpp
)
target_link_libraries(colby ${OpenCV3_LIBRARIES})
add_subdirectory(test)
