cmake_minimum_required(VERSION 4.0)
project(TechMD)


# Set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")


# Importing the Libraries
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
add_subdirectory(vendored/RmlUi EXCLUDE_FROM_ALL)
set(BUILD_MD2HTML_EXECUTABLE OFF)
add_subdirectory(vendored/md4c EXCLUDE_FROM_ALL)


# Create executable target
add_executable(${PROJECT_NAME} WIN32 src/main.cpp src/App.cpp src/TextEditListener.cpp)


# Set C++ version
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)


# Linking to the Libraries

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

## MD4C
target_link_libraries(${PROJECT_NAME} PRIVATE md4c md4c-html)

# Copy assets
## Adapted from https://github.com/Moros1138/pge-template-project/blob/fb5c4bf23ececd2cc9eee880fd8a2e7fa05c2ece/CMakeLists.txt#L271-L287
set(ASSETS_DIR "assets")
set(SOURCE_ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_DIR})
set(ASSETS_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${ASSETS_DIR})

file(GLOB_RECURSE src_assets_files
		RELATIVE ${SOURCE_ASSETS_DIR}/
		"${SOURCE_ASSETS_DIR}/*.*" "${SOURCE_ASSETS_DIR}/*")
foreach (fn ${src_assets_files})
	set(out_f ${ASSETS_OUTPUT_DIR}/${fn})
	add_custom_command(
			OUTPUT ${out_f}
			COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_ASSETS_DIR}/${fn} ${out_f}
			MAIN_DEPENDENCY ${SOURCE_ASSETS_DIR}/${fn}
	)
	list(APPEND out_assets_files ${out_f})
endforeach ()

add_custom_target(copy_assets DEPENDS ${out_assets_files})
add_dependencies(${PROJECT_NAME} copy_assets)
