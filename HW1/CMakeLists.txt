cmake_minimum_required(VERSION 3.5)

project(ImageDisplay)

set(CMAKE_CXX_STANDARD 17)

# Add source files
set(SOURCES
    src/Main.cpp
    src/processing.cpp
    src/image.cpp
    manifest/MyImageApplication.rc
)

# Sources for the analyze tool (no wxWidgets)
set(ANALYZE_SOURCES
    src/analyze_main.cpp
    src/processing.cpp
    src/image.cpp
    src/analysis.cpp
)

if(WIN32)
    set(wxBUILD_SHARED OFF)
    # or you can set it to ON, but you may face some problems about can't find dlls, in this case, you can copy dlls to your exe directory
else()
    set(wxBUILD_SHARED ON)
endif()

add_subdirectory(../dependency/wxWidgets ${CMAKE_CURRENT_BINARY_DIR}/wxWidgets)

# Create executables
add_executable(MyImageApplication ${SOURCES})
add_executable(analyze ${ANALYZE_SOURCES})

# X11 required on Linux
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

# Link libraries
target_link_libraries(MyImageApplication wx::core wx::base ${X11_LIBRARIES})
# analyze has no wxWidgets dependency
if(UNIX AND NOT APPLE)
    target_link_libraries(analyze ${X11_LIBRARIES})
endif()