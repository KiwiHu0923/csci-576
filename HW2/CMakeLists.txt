cmake_minimum_required(VERSION 3.5)

project(HW2)

set(CMAKE_CXX_STANDARD 17)

# Add source files
set(SOURCES
    src/main.cpp
    src/controller.cpp
)

if(WIN32)
    set(wxBUILD_SHARED OFF)
    # or you can set it to ON, but you may face some problems about can't find dlls, in this case, you can copy dlls to your exe directory
else()
    set(wxBUILD_SHARED ON)
endif()

add_subdirectory(../dependency/wxWidgets ${CMAKE_CURRENT_BINARY_DIR}/wxWidgets)

# Create executable
add_executable(HW2Application ${SOURCES})

# X11 required on Linux
if(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

# Link libraries
target_link_libraries(HW2Application wx::core wx::base ${X11_LIBRARIES})
