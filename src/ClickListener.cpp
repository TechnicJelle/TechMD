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
}
