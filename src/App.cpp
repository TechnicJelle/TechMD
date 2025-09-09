#include "App.hpp"

// RmlUi
#include <RmlUi/Debugger.h>

// Markdown
#include <md4c.h>
#include <md4c-html.h>

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

SDL_AppResult App::TryClose() {
	if (!openFile.has_value() && !unsavedChanges) {
		// No file is open and there has also not been anything typed, so we can close without prompting.
		return SDL_APP_SUCCESS;
	}
	if (!unsavedChanges) return SDL_APP_SUCCESS;

	std::array buttons = {
		SDL_MessageBoxButtonData{.flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, .buttonID = 0, .text = "Save and Close"},
		SDL_MessageBoxButtonData{.flags = 0, .buttonID = 1, .text = "Close without Saving"},
		SDL_MessageBoxButtonData{.flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, .buttonID = 2, .text = "Don't Close"}

	};

	const SDL_MessageBoxData messageBoxData = {
		.flags = SDL_MESSAGEBOX_WARNING,
		.window = window,
		.title = "Are you sure you want to close?",
		.message = "You still have unsaved changes!",
		.numbuttons = buttons.size(),
		.buttons = buttons.data(),
		.colorScheme = nullptr,
	};

	int clickedButtonID = -1;
	if (!SDL_ShowMessageBox(&messageBoxData, &clickedButtonID)) {
		SDL_Log("Failed to show message box: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	switch (clickedButtonID) {
		case 0:
			// Save and Close
			SaveOpenFile();
		case 1:
			// Close without Saving
			return SDL_APP_SUCCESS;
		default:
			// Don't close
			return SDL_APP_CONTINUE;
	}
}

void App::OpenFile(const std::filesystem::path& filepathToOpen) {
	openFile = filepathToOpen;

	const std::filesystem::path& filePath = openFile.value();

	if (Rml::Element* openFileElement = rmlDocument->GetElementById("open-file")) {
		openFileElement->SetInnerRML(filePath.string());
	}

	// Only load the text contents if the file exists.
	// But we don't crash or anything if the file doesn't exist.
	// We will just treat it like a new file.
	if (exists(filePath)) {
		SetUnsavedChanges(false);
		if (void* contents = SDL_LoadFile(filePath.string().c_str(), nullptr)) {
			const std::string markdown(static_cast<const char*>(contents));
			Rml::ElementFormControlTextArea* textarea = dynamic_cast<Rml::ElementFormControlTextArea*>(rmlDocument->GetElementById("editor"));
			textarea->SetValue(markdown);
			SetViewerMarkdown(markdown);
			SDL_free(contents);
		} else {
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to load file", ("Failed to load file: " + filePath.string() + "\n" + SDL_GetError()).c_str(), window);
		}
	} else {
		SetUnsavedChanges(true);
	}
}

void App::SaveFile(const std::filesystem::path& filepathToSave) {
	SDL_Log("Saving file... %s", filepathToSave.string().c_str());
	const Rml::ElementFormControlTextArea* textarea = dynamic_cast<Rml::ElementFormControlTextArea*>(rmlDocument->GetElementById("editor"));
	const Rml::String mdText = textarea->GetValue();
	if (!SDL_SaveFile(filepathToSave.string().c_str(), mdText.c_str(), mdText.size())) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to save file", ("Failed to save file: " + filepathToSave.string() + "\n" + SDL_GetError()).c_str(), window);
		return;
	}
	SetUnsavedChanges(false);
}

void App::SetViewerHTML(const std::string& newHtml) {
	html = newHtml;
	Rml::Element* viewer = rmlDocument->GetElementById("viewer");
	viewer->SetInnerRML(html);
}

SDL_AppResult App::Init(const int width, const int height, const std::optional<std::filesystem::path>& filepathToOpen) {
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

	clickListener = new ClickListener(this);
	rmlDocument->AddEventListener(Rml::EventId::Click, clickListener);

	textEditListener = new TextEditListener(this);
	Rml::ElementFormControlTextArea* textarea = dynamic_cast<Rml::ElementFormControlTextArea*>(rmlDocument->GetElementById("editor"));
	textarea->AddEventListener(Rml::EventId::Change, textEditListener);

	if (filepathToOpen.has_value()) {
		OpenFile(filepathToOpen.value());
	}

	return SDL_APP_CONTINUE;
}

SDL_AppResult App::Event(SDL_Event* event) {
	// Handle the important global events before the RmlUi Input Event Handler
	switch (event->type) {
		case SDL_EVENT_KEY_DOWN:
			switch (event->key.key) {
				case SDLK_F2:
					DumpHTML();
					break;
				case SDLK_F12:
					Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
					break;
				case SDLK_O:
					if (event->key.mod & SDL_KMOD_CTRL) {
						OpenFileDialog();
					}
					break;
				case SDLK_S:
					if (event->key.mod & SDL_KMOD_CTRL) {
						SaveOpenFile();
					}
					break;
				case SDLK_Q:
					if (event->key.mod & SDL_KMOD_CTRL) {
						return TryClose();
					}
					break;
				case SDLK_ESCAPE:
					//Unfocus the textarea if it has focus.
					if (Rml::Element* focusedElement = rmlContext->GetFocusElement();
						focusedElement && focusedElement->GetTagName() == "textarea") {
						focusedElement->Blur();
					}
					break;
				default: break;
			}
			break;
		default: break;
	}

	// Handle RmlUi Input Events
	if (RmlSDL::InputEventHandler(rmlContext, window, *event) == false) {
		// If the RmlUi Input Event Handler handled the event, we stop processing it.
		return SDL_APP_CONTINUE;
	}

	// Handle the less important events that weren't handled by the RmlUi Input Event Handler
	switch (event->type) {
		case SDL_EVENT_KEY_DOWN:
			switch (event->key.key) {
				case SDLK_Q:
				case SDLK_ESCAPE:
					return TryClose();
				default: break;
			}
			break;

		case SDL_EVENT_QUIT:
			return TryClose();

		case SDL_EVENT_DROP_FILE:
			OpenFile(event->drop.data);
			break;

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
	delete textEditListener;
	delete clickListener;
}

void App::SetUnsavedChanges(const bool newValue) {
	if (newValue == unsavedChanges) return; // No change, do nothing.

	unsavedChanges = newValue;
	Rml::Element* unsavedChangesElement = rmlDocument->GetElementById("unsaved-changes");
	if (unsavedChanges) {
		unsavedChangesElement->SetInnerRML("*");
	} else {
		unsavedChangesElement->SetInnerRML("");
	}
}

void App::SetViewerMarkdown(const std::string& newMarkdown) {
	constexpr unsigned parserFlags = MD_DIALECT_GITHUB;
	constexpr unsigned rendererFlags = MD_HTML_FLAG_XHTML | MD_HTML_FLAG_SKIP_UTF8_BOM;

	std::string finalHtml{};
	if (md_html(newMarkdown.c_str(), newMarkdown.size(),
				[](const MD_CHAR* output, const MD_SIZE size, void* userdata) {
					std::string* html = static_cast<std::string*>(userdata);
					const std::string_view outputView(output, size);
					if (outputView == "<br>" || outputView == "<BR>") {
						html->append("<br />");
						return;
					}
					if (outputView.starts_with("<img") && outputView.ends_with(">") && !outputView.ends_with("/>")) {
						html->append(outputView.substr(0, outputView.size() - 1));
						html->append(" />");
						return;
					}
					html->append(outputView);
					if (outputView == "<li>") {
						html->append("• ");
					}
				},
				&finalHtml, parserFlags, rendererFlags) < 0) {
		SDL_Log("MD4C HTML parse failed");
	}

	SetViewerHTML(finalHtml);
}

void App::OpenFileDialog() {
	if (unsavedChanges) {
		if (const SDL_AppResult result = TryClose();
			result == SDL_APP_CONTINUE || result == SDL_APP_FAILURE) {
			// Don't close the file after all, so don't open a new one either.
			return;
		}
	}

	constexpr std::array filters = {
		SDL_DialogFileFilter{.name = "Markdown Files", .pattern = "md;markdown"},
		SDL_DialogFileFilter{.name = "All Files", .pattern = "md;markdown"},
	};
	const SDL_DialogFileCallback callback = [](void* userdata, const char* const* filelist, int filter) {
		if (filelist && filelist[0]) {
			App* app = static_cast<App*>(userdata);
			app->OpenFile(filelist[0]);
		} else {
			SDL_Log("No file selected or an error occurred.");
		}
	};
	SDL_ShowOpenFileDialog(callback, this, window, filters.data(), filters.size(), nullptr, false);
}

void App::SaveOpenFile() {
	if (openFile.has_value()) {
		if (!unsavedChanges) return;
		SaveFile(openFile.value());
	} else {
		constexpr std::array filters = {
			SDL_DialogFileFilter{.name = "Markdown File", .pattern = "md"},
		};
		const SDL_DialogFileCallback callback = [](void* userdata, const char* const* filelist, int filter) {
			App* app = static_cast<App*>(userdata);
			if (filelist && filelist[0]) {
				const std::filesystem::path fileToSave = filelist[0] + std::string(".md");
				app->SaveFile(fileToSave);
				app->OpenFile(fileToSave); // Update the open file path after saving.
			} else {
				SDL_Log("No file selected or an error occurred.");
			}
			SDL_Event event = {.type = SDL_EVENT_QUIT};
			SDL_PushEvent(&event);
		};
		SDL_ShowSaveFileDialog(callback, this, window, filters.data(), filters.size(), nullptr);

		// Wait for the dialog to close
		SDL_Event event;
		while (SDL_WaitEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				break;
			}
		}
	}
}
