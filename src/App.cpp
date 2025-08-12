#include "App.hpp"

// RmlUi
#include <RmlUi/Debugger.h>

bool App::LoadFonts() const {
	int count{};
	const std::string font_dir = appDirectory + "assets/fonts/";
	char** const& font_files = SDL_GlobDirectory(font_dir.c_str(), "**/*.?tf", SDL_GLOB_CASEINSENSITIVE, &count);
	if (count <= 0) {
		SDL_Log("No font files found in assets/fonts");
		return false;
	}
	for (int i = 0; i < count; i++) {
		if (!Rml::LoadFontFace(font_dir + font_files[i])) {
			SDL_Log("Failed to load font face: Roboto");
			return false;
		}
	}
	SDL_free(font_files);
	return true;
}

void App::DumpHTML() const {
	SDL_Log("---\n%s", html.c_str());
}

SDL_AppResult App::Init(const int width, const int height, const std::filesystem::path& filepathToOpen) {
	appDirectory = SDL_GetBasePath();

	SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");
	if (constexpr SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;
		!SDL_CreateWindowAndRenderer("Tech's Markdown File Viewer/Editor", width, height, flags, &window, &renderer)
	) {
		SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	if (!SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE)) {
		SDL_Log("Couldn't set renderer VSync (ADAPTIVE): %s", SDL_GetError());
		if (!SDL_SetRenderVSync(renderer, 1)) {
			SDL_Log("Couldn't set renderer VSync (1): %s", SDL_GetError());
			return SDL_APP_FAILURE;
		}
	}

	rmlSystemInterface = new SystemInterface_SDL();
	rmlSystemInterface->SetWindow(window);
	Rml::SetSystemInterface(rmlSystemInterface);

	rmlRenderInterface = new RenderInterface_SDL(renderer);
	Rml::SetRenderInterface(rmlRenderInterface);

	Rml::Initialise();

	rmlContext = Rml::CreateContext("default", Rml::Vector2i{width, height});
	if (rmlContext == nullptr) {
		return SDL_APP_FAILURE;
	}

	Rml::Debugger::Initialise(rmlContext);

	if (!LoadFonts()) {
		return SDL_APP_FAILURE;
	}

	//BUG: All Markdown images are loaded from the appDirectory too, which is bad.
	// Those should be loaded from the current working directory instead.
	rmlDocument = rmlContext->LoadDocument(appDirectory + "assets/ui.html");
	if (rmlDocument)
		rmlDocument->Show();

	eventListener = new TextEditListener(this);
	Rml::ElementFormControlTextArea* textarea = dynamic_cast<Rml::ElementFormControlTextArea*>(rmlDocument->GetElementById("editor"));
	textarea->AddEventListener(Rml::EventId::Change, eventListener);

	if (!filepathToOpen.empty()) {
		if (!exists(filepathToOpen)) {
			SDL_Log("File '%s' does not exist.", filepathToOpen.string().c_str());
			return SDL_APP_FAILURE;
		}
		if (void* contents = SDL_LoadFile(filepathToOpen.string().c_str(), nullptr)) {
			const std::string markdown(static_cast<const char*>(contents));
			textarea->SetValue(markdown); //BUG: This does not refresh the screen. You have to type something in the textarea to see it.
			SDL_free(contents);
		} else {
			SDL_Log("Failed to load file '%s': %s", filepathToOpen.string().c_str(), SDL_GetError());
		}
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult App::Event(SDL_Event* event) const {
	if (RmlSDL::InputEventHandler(rmlContext, window, *event) == false) {
		return SDL_APP_CONTINUE;
	}

	switch (event->type) {
		case SDL_EVENT_KEY_DOWN:
			switch (event->key.key) {
				case SDLK_Q:
				case SDLK_ESCAPE:
					return SDL_APP_SUCCESS;
				case SDLK_F2:
					DumpHTML();
					break;
				case SDLK_F12:
					Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
					break;
				default: break;
			}
			break;

		case SDL_EVENT_QUIT:
			return SDL_APP_SUCCESS;

		default: break;
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult App::Iterate() const {
	rmlContext->Update();

	rmlRenderInterface->BeginFrame();
	rmlContext->Render();
	rmlRenderInterface->EndFrame();

	SDL_RenderPresent(renderer);
	return SDL_APP_CONTINUE;
}

void App::Quit(SDL_AppResult result) const {
	rmlDocument->Close();

	Rml::Shutdown();
	delete rmlRenderInterface;
	delete rmlSystemInterface;
	delete eventListener;
}

void App::SetViewerHTML(const std::string& newHtml) {
	html = std::move(newHtml);
	Rml::Element* viewer = rmlDocument->GetElementById("viewer");
	viewer->SetInnerRML(html);
}
