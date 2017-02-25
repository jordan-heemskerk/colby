add_executable(tests
	hash.cpp
	main.cpp
)
target_link_libraries(tests colby)
target_include_directories(tests PRIVATE ${CATCH_INCLUDE_DIR})
add_test(NAME tests COMMAND tests)
