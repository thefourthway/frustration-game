include(cmake/CPM.cmake)

CPMAddPackage("gh:raysan5/raylib#5.5")
CPMAddPackage(
    NAME JoltPhysics
    GITHUB_REPOSITORY jrouwe/JoltPhysics
    GIT_TAG v5.5.0
    SOURCE_SUBDIR Build
    OPTIONS
    "USE_STATIC_MSVC_RUNTIME_LIBRARY OFF"
)

set(CMAKE_REQUIRED_FLAGS "/std:c++20")

CPMAddPackage(
    NAME abseil
    GITHUB_REPOSITORY abseil/abseil-cpp
    GIT_TAG 20260107.1
    OPTIONS
    "ABSL_PROPAGATE_CXX_STD ON"
    "ABSL_BUILD_DLL OFF"
    "ABSL_ENABLE_INSTALL OFF"
    "ABSL_USE_EXTERNAL_GOOGLETEST OFF"
    "ABSL_USE_SYSTEM_INCLUDES ON"
    "BUILD_SHARED_LIBS OFF"
    "BUILD_TESTING OFF"
)

CPMAddPackage(
    NAME Catch2
    GITHUB_REPOSITORY catchorg/Catch2
    GIT_TAG v3.13.0
    OPTIONS
        "CATCH_INSTALL_DOCS OFF"
        "CATCH_INSTALL_EXTRAS OFF"
        "CATCH_BUILD_TESTING OFF"
)