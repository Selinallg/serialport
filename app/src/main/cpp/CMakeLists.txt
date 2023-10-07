
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.

project("serialport")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
#message(==========${CMAKE_CURRENT_SOURCE_DIR})

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.




include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/boost/include/boost-1_74/
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/eigen-3.3.9/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/glm-0.9.9.8/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/nlohmann-3.9.1/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/KinBase/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/fmt-7.1.3/include/
)

link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/boost/lib/${CMAKE_ANDROID_ARCH_ABI}/
        ${CMAKE_CURRENT_SOURCE_DIR}/3rd/fmt-7.1.3/lib/${CMAKE_ANDROID_ARCH_ABI}/
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdLibrary/${CMAKE_ANDROID_ARCH_ABI}/
)


add_library( # Sets the name of the library.
             uartusb

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             device_util.cpp
             uart_manager.cpp
             serialport.cpp )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       uartusb

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )

link_directories(3rd/fmt-7.1.3/lib/${ANDROID_ABI})
link_directories(../../../src/main/jni/libs/${ANDROID_ABI})