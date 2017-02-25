add_executable(color_by_number
	main.cpp
)
target_link_libraries(color_by_number colby ${Boost_LIBRARIES} ${OpenCV3_LIBRARIES})
add_subdirectory(colby)
