add_library(colby SHARED
	color_by_numbers.cpp
)
target_link_libraries(colby ${OpenCV3_LIBRARIES})
add_subdirectory(test)
