
#
# Copyright (C) The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.4.1)

# configure import libs
set(distribution_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../fcpp-android)

add_library(lib_ap STATIC IMPORTED)
set_target_properties(lib_ap PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/${ANDROID_ABI}/run/main.a)

set(CMAKE_BUILD_TYPE DEBUG)

# build application's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")

add_library(ap-libs SHARED
            ap-getters.cpp)

target_include_directories(ap-libs PRIVATE
                           ${distribution_DIR}/run
                           )

target_link_libraries(ap-libs
                      android
                      lib_ap
                      log)
