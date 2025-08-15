// SDL3
#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>

// Project
#include "App.hpp"

SDL_AppResult SDL_AppInit(void** appstate, [[maybe_unused]] const int argc, [[maybe_unused]] char* argv[]) {
	App* app = new App();
	*appstate = app;

	std::optional<std::filesystem::path> filepathToOpen;
	if (argc > 1) {
		filepathToOpen = argv[1];
	}

	return app->Init(1280, 720, filepathToOpen);
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	App* app = static_cast<App*>(appstate);
	return app->Event(event);
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	const App* app = static_cast<App*>(appstate);
	return app->Iterate();
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
	const App* app = static_cast<App*>(appstate);
	app->Quit(result);
	delete app;
}
