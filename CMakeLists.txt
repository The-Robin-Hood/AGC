cmake_minimum_required(VERSION 3.30)
project(audioProcessor
    VERSION 1.0.0
    DESCRIPTION "Audio Gain Controller using WebRTC - GainController2"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
endif()

add_executable(audioProcessor src/main.cpp src/audioProcessor.cpp)
add_definitions(-DWEBRTC_APM_DEBUG_DUMP=0)

target_include_directories(audioProcessor PRIVATE src/ thirdparty/agc/include thirdparty/agc/include/webrtc thirdparty/agc/include/webrtc/modules/audio_processing)
target_link_directories(audioProcessor PRIVATE thirdparty/agc/lib)
target_link_libraries(audioProcessor PRIVATE webrtc-audio-processing-2)
