add_library(colby SHARED
)
target_link_libraries(colby ${OpenCV3_LIBRARIES})
add_subdirectory(test)
