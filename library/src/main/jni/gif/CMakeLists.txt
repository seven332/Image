# Copyright 2018 Hippo Seven
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.4.1)
project(image-gif C)

set(GIFLIB_SOURCES
    image_gif.c
    giflib/lib/dgif_lib.c
    giflib/lib/gif_hash.c
    giflib/lib/gifalloc.c
    giflib/lib/openbsd-reallocarray.c
)

set(GIFLIB_INCLUDES
    ${IMAGE_EXPORT_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/giflib/lib
)

set(GIFLIB_DEFINITIONS
)

set(GIFLIB_LIBRARIES
)

if(IMAGE_SINGLE_SHARED_LIB)
    # Compile a static library for image-singlelib
    set(GIFLIB_LIBRARY_NAME image-gif-static)
    set(GIFLIB_LIBRARY_TYPE STATIC)
    set(GIFLIB_INCLUDES_VISIBILITY PUBLIC) # Share include dir with image project
else()
    # Compile a shared library to be imported by image-core
    set(GIFLIB_LIBRARY_NAME image-gif)
    set(GIFLIB_LIBRARY_TYPE SHARED)
    set(GIFLIB_INCLUDES_VISIBILITY PRIVATE)
    
    # Link against image shared lib to get helper methods
    set(GIFLIB_LIBRARIES
        ${GIFLIB_LIBRARIES}
        image
        log
    )
endif()

add_library(${GIFLIB_LIBRARY_NAME} ${GIFLIB_LIBRARY_TYPE} ${GIFLIB_SOURCES})
target_include_directories(${GIFLIB_LIBRARY_NAME} ${GIFLIB_INCLUDES_VISIBILITY} ${GIFLIB_INCLUDES})
target_compile_definitions(${GIFLIB_LIBRARY_NAME} PRIVATE ${GIFLIB_DEFINITIONS})
target_link_libraries(${GIFLIB_LIBRARY_NAME} PRIVATE ${GIFLIB_LIBRARIES})