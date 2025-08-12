#include "TextEditListener.hpp"

// SDL3
#include <SDL3/SDL.h>

// MD4C
#include "../vendored/md4c/src/md4c.h"
#include "../vendored/md4c/src/md4c-html.h"

// Project
#include "App.hpp"

void TextEditListener::ProcessEvent(Rml::Event& event) {
	const Rml::ElementFormControlTextArea* textarea = dynamic_cast<Rml::ElementFormControlTextArea*>(event.GetTargetElement());
	const Rml::String mdText = textarea->GetValue();

	constexpr unsigned parserFlags = MD_DIALECT_GITHUB;
	constexpr unsigned rendererFlags = MD_HTML_FLAG_XHTML | MD_HTML_FLAG_SKIP_UTF8_BOM;

	std::string finalHtml{};
	if (md_html(mdText.c_str(), mdText.size(),
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

	app->SetViewerHTML(std::move(finalHtml));
}
