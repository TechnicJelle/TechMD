#pragma once

// RmlUi
#include <RmlUi/Core.h>
#include <RmlUi_Platform_SDL.h>
#include <RmlUi_Renderer_SDL.h>

// C++
#include <filesystem>
#include <optional>

// Project
#include "ClickListener.hpp"
#include "TextEditListener.hpp"

class App {
	std::string appDirectory{};
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	RenderInterface_SDL* rmlRenderInterface = nullptr;
	SystemInterface_SDL* rmlSystemInterface = nullptr;
	Rml::Context* rmlContext = nullptr;
	Rml::ElementDocument* rmlDocument = nullptr;
	ClickListener* clickListener = nullptr;
	TextEditListener* textEditListener = nullptr;
	std::string html{};
	std::optional<std::filesystem::path> openFile{};
	bool unsavedChanges = false;

private:
	/// Returns true if the fonts were loaded successfully.
	[[nodiscard]] bool LoadFonts() const;

	void DumpHTML() const;

	[[nodiscard]] SDL_AppResult TryClose();

	void OpenFile(const std::filesystem::path& filepathToOpen);

	void SaveFile(const std::filesystem::path& filepathToSave);

	void SetViewerHTML(const std::string& newHtml);

public:
	[[nodiscard]] SDL_AppResult Init(int width, int height, const std::optional<std::filesystem::path>& filepathToOpen);

	[[nodiscard]] SDL_AppResult Event(SDL_Event* event);

	[[nodiscard]] SDL_AppResult Iterate() const;

	void Quit(SDL_AppResult result) const;

public:
	void SetUnsavedChanges(bool newValue);

	void SetViewerMarkdown(const std::string& newMarkdown);

	void OpenFileDialog();

	void SaveOpenFile();
};
