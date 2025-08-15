#include "TextEditListener.hpp"

// Project
#include "App.hpp"

void TextEditListener::ProcessEvent(Rml::Event& event) {
	app->SetUnsavedChanges(true);

	const Rml::ElementFormControlTextArea* textarea = dynamic_cast<Rml::ElementFormControlTextArea*>(event.GetTargetElement());
	const Rml::String mdText = textarea->GetValue();

	app->SetViewerMarkdown(mdText);
}
