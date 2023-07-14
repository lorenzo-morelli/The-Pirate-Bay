set($ENV{VULKAN_SDK} "C:/VulkanSDK/1.3.239.0") # Vulkan SDK
set(GLFW "C:/VulkanSDK/1.3.239.0/Lib/glfw-3.3.8.bin.WIN64") # GLFW
set(GLM "C:/VulkanSDK/1.3.239.0/Lib/glm") # GLM
list(APPEND INCLUDE_DIRS "${GLFW}/include" ${GLM} headers)
list(APPEND LINK_LIBS "${GLFW}/lib-mingw-w64/libglfw3.a")
set(PROJ_NAME progetto-cg)
set(MAIN main.cpp)
#########################################################

cmake_minimum_required(VERSION 3.24)
project(${PROJ_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
add_executable(${PROJ_NAME} ${MAIN})
find_package(Vulkan REQUIRED)
foreach(dir IN LISTS Vulkan_INCLUDE_DIR INCLUDE_DIRS)
    target_include_directories(${PROJ_NAME} PUBLIC ${dir})
endforeach()

target_compile_features(${PROJ_NAME} PRIVATE cxx_std_17)
foreach(lib IN LISTS Vulkan_LIBRARIES LINK_LIBS)
    target_link_libraries(${PROJ_NAME} ${lib})
endforeach()