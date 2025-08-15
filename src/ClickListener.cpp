#include "ClickListener.hpp"

#include "App.hpp"

void ClickListener::ProcessEvent(Rml::Event& event) {
	const Rml::Element* element = event.GetTargetElement();

	if (const std::string& id = element->GetId();
		id == "toolbar_open") {
		app->OpenFileDialog();
	} else if (id == "toolbar_save") {
		app->SaveOpenFile();
	}

	if (const std::string& tagName = element->GetTagName();
		tagName == "a") {
		const Rml::String href = element->GetAttribute<Rml::String>("href", "");
		if (!href.empty() && href != "#" && !href.starts_with("javascript:")) {
			SDL_OpenURL(href.c_str());
		}
	}
}
