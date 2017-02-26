add_executable(color_by_numbers
	main.cpp
)
target_link_libraries(color_by_numbers colby ${Boost_LIBRARIES} ${OpenCV3_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_subdirectory(colby)
