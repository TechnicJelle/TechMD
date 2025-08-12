#pragma once

// RmlUi
#include <RmlUi/Core.h>
#include <RmlUi_Platform_SDL.h>
#include <RmlUi_Renderer_SDL.h>

// Project
#include "TextEditListener.hpp"

class App {
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	RenderInterface_SDL* rmlRenderInterface = nullptr;
	SystemInterface_SDL* rmlSystemInterface = nullptr;
	Rml::Context* rmlContext = nullptr;
	Rml::ElementDocument* rmlDocument = nullptr;
	TextEditListener* eventListener = nullptr;
	std::string html{};

private:
	/// Returns true if the fonts were loaded successfully.
	static bool LoadFonts();

	void DumpHTML() const;

public:
	[[nodiscard]] SDL_AppResult Init(int width, int height);

	[[nodiscard]] SDL_AppResult Event(SDL_Event* event) const;

	[[nodiscard]] SDL_AppResult Iterate() const;

	void Quit(SDL_AppResult result) const;

public:
	void SetViewerHTML(const std::string& newHtml);
};
