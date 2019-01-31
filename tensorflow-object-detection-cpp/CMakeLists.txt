cmake_minimum_required(VERSION 2.8.8)
project(tf_detector_example)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -W")

set(SOURCE_FILES main.cpp utils.cpp utils.h)
add_executable(tf_detector_example ${SOURCE_FILES})

# OpenCV libs
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(tf_detector_example ${OpenCV_LIBS})


# ==================== PATHS TO SPECIFY! ==================== #

# Eigen lib headers
#include_directories("/usr/local/include/eigen3")

# TensorFlow headers
#include_directories("/usr/local/include/tf/")
#include_directories("/usr/local/include/tf/bazel-genfiles/")
#include_directories("/usr/local/include/tf/tensorflow/")
#include_directories("/usr/local/include/tf/third-party/")

link_directories(./lib)

include_directories(
   /home/ailab/Downloads/tensorflow
   /home/ailab/Downloads/tensorflow/bazel-genfiles
   /home/ailab/Downloads/tensorflow/bazel-bin/tensorflow
   /home/ailab/Downloads/tensorflow/tensorflow/contrib/makefile/downloads/nsync/public
   /home/ailab/Downloads/tensorflow/tensorflow/contrib/makefile/gen/protobuf/include
   /usr/local/include/eigen3
   /home/ailab/Downloads/tensorflow/tensorflow/contrib/makefile/downloads/absl
   )


# Link TensorFlow libs
#target_link_libraries(tf_detector_example tensorflow_cc tensorflow_framework)
#target_link_libraries(tf_detector_example "/usr/local/lib/libtensorflow_framework.so")

target_link_libraries(tf_detector_example "/home/ailab/Downloads/tensorflow/bazel-bin/tensorflow/libtensorflow_cc.so")
target_link_libraries(tf_detector_example "/home/ailab/Downloads/tensorflow/bazel-bin/tensorflow/libtensorflow_framework.so")

