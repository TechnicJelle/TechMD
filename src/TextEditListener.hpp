#pragma once

// RmlUi
#include <RmlUi/Core.h>

class App;

class TextEditListener final : public Rml::EventListener {
	App* app;

public:
	explicit TextEditListener(App* app)
		: app(app) {
	}

	void ProcessEvent(Rml::Event& event) override;
};
