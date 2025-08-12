cmake_minimum_required(VERSION 3.28)
project(TechMD)

set(CMAKE_CXX_STANDARD 23)

# Set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Importing the Libraries
add_subdirectory(vendored/md4c/src EXCLUDE_FROM_ALL)

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/freetype EXCLUDE_FROM_ALL)
add_library(Freetype::Freetype ALIAS freetype)

add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
add_subdirectory(vendored/RmlUi EXCLUDE_FROM_ALL)

# Create executable target
add_executable(${PROJECT_NAME} WIN32 src/main.cpp src/App.cpp src/TextEditListener.cpp)

# Linking to the Libraries

## MD4C
target_link_libraries(${PROJECT_NAME} PRIVATE md4c)
target_link_libraries(${PROJECT_NAME} PRIVATE md4c-html)
target_include_directories(${PROJECT_NAME} PRIVATE vendored/md4c/src)

## SDL3
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

## SDL_image
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3_image::SDL3_image)

## RmlUI
set(RMLUI_BACKEND_PATH ${RmlUi_SOURCE_DIR}/Backends)
add_library(rmlui_backend_SDL_Renderer INTERFACE)
target_sources(rmlui_backend_SDL_Renderer INTERFACE
		${RMLUI_BACKEND_PATH}/RmlUi_Platform_SDL.cpp
		${RMLUI_BACKEND_PATH}/RmlUi_Renderer_SDL.cpp
)
target_include_directories(rmlui_backend_SDL_Renderer INTERFACE ${RMLUI_BACKEND_PATH})
target_compile_definitions(rmlui_backend_SDL_Renderer INTERFACE
		RMLUI_SDL_VERSION_MAJOR=3
)
target_link_libraries(${PROJECT_NAME} PRIVATE RmlUi::RmlUi rmlui_backend_SDL_Renderer RmlUi::Debugger)

# Copy Assets
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
	add_custom_command(
		TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND}
		ARGS -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/assets
	)
endif()

