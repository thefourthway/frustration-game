set(TESTS_INCLUDE)
set(TESTS_SRC
    src/test_ecs.cpp
    src/test_event_codec.cpp
    src/test_quest_codec.cpp
)
add_executable(tests ${TESTS_SRC} ${TESTS_INCLUDE})

target_include_directories(tests
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(tests
    PRIVATE
        gamelib
        Catch2::Catch2WithMain
)


include(${Catch2_SOURCE_DIR}/extras/Catch.cmake)
catch_discover_tests(tests)