#pragma once

// RmlUi
#include <RmlUi/Core.h>

class App;

class ClickListener final : public Rml::EventListener {
	App* app;

public:
	explicit ClickListener(App* app)
		: app(app) {
	}

	void ProcessEvent(Rml::Event& event) override;
};
