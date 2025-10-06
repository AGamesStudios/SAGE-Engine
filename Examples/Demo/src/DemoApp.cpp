#include "SAGE.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_set>

using namespace SAGE;

namespace {
	constexpr float kOrbitRadiusX = 160.0f;
	constexpr float kOrbitRadiusY = 90.0f;
	constexpr float kOrbitSpeed = 0.9f;
	constexpr float kClickInterval = 4.0f;
	constexpr float kTestHighlightDuration = 2.5f;

	std::filesystem::path ResolveAssetsDirectory();
}

class DemoApp final : public Application {
public:
	DemoApp();
	~DemoApp() override;

	void ConfigureRuntime(bool headless, float autoExitSeconds);

protected:
	void OnInit() override;
	void OnUpdate(float deltaTime) override;
	void OnRender() override;

private:
	void CreateSceneContent();
	void LoadAudioAssets();
	void LoadFonts();
	void BuildHud();
	void ToggleAmbient();
	void UpdateAmbientButtonAppearance();
	void RunUiTest();

	GameObject* m_Focus = nullptr;
	float m_PlayTime = 0.0f;
	bool m_Headless = false;
	float m_AutoExitRemaining = -1.0f;
	float m_LastClickTime = 0.0f;
	bool m_AmbientEnabled = false;
	bool m_ColorFrozen = false;
	Ref<Sound> m_ClickSound;
	Ref<Sound> m_AmbientSound;
	UI::Button* m_ToggleAmbientButton = nullptr;
	UI::ProgressBar* m_ClickProgressBar = nullptr;
	UI::Label* m_TestStatusLabel = nullptr;
	UI::Button* m_RunUiTestButton = nullptr;
	UI::Panel* m_HudPanel = nullptr;
	UI::Panel* m_TestPanel = nullptr;
	Ref<Texture> m_UiBadge;
	float m_TestHighlightTimer = 0.0f;
	float m_LastUiTestTime = -1.0f;
	bool m_HasRunUiTest = false;
	UI::Color m_TestStatusBaseColor = UI::Color(0.2f, 0.16f, 0.08f, 1.0f);
	UI::Color m_TestStatusHighlightColor = UI::Color(0.32f, 0.62f, 0.48f, 1.0f);
	std::filesystem::path m_AssetsRoot;
	Ref<Font> m_TitleFont;
	Ref<Font> m_BodyFont;
	Ref<Font> m_ButtonFont;
};

Application* SAGE::CreateApplication();

int main(int argc, char** argv);

namespace {

	std::filesystem::path ResolveAssetsDirectory() {
		namespace fs = std::filesystem;
		fs::path current = fs::current_path();
		for (int i = 0; i < 6; ++i) {
			if (fs::exists(current / "Demo" / "assets")) {
				return current / "Demo" / "assets";
			}
			if (fs::exists(current / "assets")) {
				return current / "assets";
			}
			if (!current.has_parent_path()) {
				break;
			}
			current = current.parent_path();
		}
		return fs::path("Demo/assets");
	}

} // namespace

DemoApp::DemoApp()
	: Application("SAGE Engine Demo") {}

DemoApp::~DemoApp() {
	if (m_AmbientSound && m_AmbientSound->IsValid()) {
		m_AmbientSound->Stop();
	}
	if (m_ClickSound && m_ClickSound->IsValid()) {
		m_ClickSound->Stop();
	}
	m_AmbientSound.reset();
	m_ClickSound.reset();
	SoundManager::Clear();
}

void DemoApp::ConfigureRuntime(bool headless, float autoExitSeconds) {
	m_Headless = headless;
	m_AutoExitRemaining = autoExitSeconds;
}

void DemoApp::OnInit() {
	m_PlayTime = 0.0f;
	m_LastClickTime = 0.0f;
	m_ColorFrozen = false;
	m_AmbientEnabled = false;
	m_LastUiTestTime = -1.0f;
	m_TestHighlightTimer = 0.0f;
	m_HasRunUiTest = false;
	m_TestStatusLabel = nullptr;
	m_RunUiTestButton = nullptr;
	m_HudPanel = nullptr;
	m_TestPanel = nullptr;
	m_TitleFont.reset();
	m_BodyFont.reset();
	m_ButtonFont.reset();

	InputBindings::Clear();
	InputBindings::RegisterAction("quit", { SAGE_KEY_ESCAPE });
	InputBindings::RegisterAction("toggle_ambient", { SAGE_KEY_M });
	InputBindings::RegisterAction("toggle_color_lock", { SAGE_KEY_C });

	GameObject::DestroyAll();
	CreateSceneContent();
	m_AssetsRoot = ResolveAssetsDirectory();
	LoadFonts();
	LoadAudioAssets();

	if (!m_Headless) {
		UI::UISystem::Clear();
		BuildHud();
	} else {
		m_ToggleAmbientButton = nullptr;
		m_ClickProgressBar = nullptr;
		m_TestStatusLabel = nullptr;
		m_RunUiTestButton = nullptr;
	}
}

void DemoApp::OnUpdate(float deltaTime) {
	m_PlayTime += deltaTime;

	if (m_AutoExitRemaining >= 0.0f) {
		m_AutoExitRemaining -= deltaTime;
		if (m_AutoExitRemaining <= 0.0f) {
			m_Running = false;
		}
	}

	if (InputBindings::IsActionPressed("quit")) {
		m_Running = false;
	}

	if (!m_Headless && InputBindings::IsActionPressed("toggle_ambient")) {
		ToggleAmbient();
	}

	if (InputBindings::IsActionPressed("toggle_color_lock")) {
		m_ColorFrozen = !m_ColorFrozen;
	}

	if (m_Focus) {
		const float t = m_PlayTime * kOrbitSpeed;
		const float offsetX = std::sin(t) * kOrbitRadiusX;
		const float offsetY = std::cos(t * 0.75f) * kOrbitRadiusY;
		m_Focus->MoveTo(320.0f + offsetX, 220.0f + offsetY);

		if (!m_ColorFrozen) {
			const float r = 0.55f + 0.35f * std::sin(t * 1.7f);
			const float g = 0.48f + 0.42f * std::sin(t * 1.1f + 2.1f);
			const float b = 0.44f + 0.36f * std::sin(t * 0.8f + 4.2f);
			m_Focus->color = Color(r, g, b, 1.0f);
		}
	}

	if (!m_Headless && m_ClickSound && m_ClickSound->IsValid()) {
		if (m_PlayTime - m_LastClickTime >= kClickInterval) {
			m_ClickSound->Play(true);
			m_LastClickTime = m_PlayTime;
		}
	}

	if (m_ClickProgressBar) {
		const float sinceClick = std::max(0.0f, m_PlayTime - m_LastClickTime);
		const float normalized = std::min(sinceClick / kClickInterval, 1.0f);
		auto& style = m_ClickProgressBar->GetStyle();
		if (normalized > 0.85f) {
			style.fillColor = UI::Color(0.92f, 0.35f, 0.35f, 0.96f);
		} else if (normalized > 0.5f) {
			style.fillColor = UI::Color(0.95f, 0.69f, 0.28f, 0.96f);
		} else {
			style.fillColor = UI::Color(0.33f, 0.68f, 0.53f, 0.96f);
		}
	}

	if (m_TestStatusLabel) {
		if (m_TestHighlightTimer > 0.0f) {
			const float blend = std::clamp(m_TestHighlightTimer / kTestHighlightDuration, 0.0f, 1.0f);
			UI::Color current(
				m_TestStatusBaseColor.r + (m_TestStatusHighlightColor.r - m_TestStatusBaseColor.r) * blend,
				m_TestStatusBaseColor.g + (m_TestStatusHighlightColor.g - m_TestStatusBaseColor.g) * blend,
				m_TestStatusBaseColor.b + (m_TestStatusHighlightColor.b - m_TestStatusBaseColor.b) * blend,
				m_TestStatusBaseColor.a + (m_TestStatusHighlightColor.a - m_TestStatusBaseColor.a) * blend);
			m_TestStatusLabel->SetColor(current);
			m_TestHighlightTimer = std::max(0.0f, m_TestHighlightTimer - deltaTime);
		} else {
			m_TestStatusLabel->SetColor(m_TestStatusBaseColor);
		}
	}
}

void DemoApp::OnRender() {
	if (m_Headless) {
		return;
	}

	const Vector2 vignettePos(12.0f, 10.0f);
	const Vector2 vignetteSize(372.0f, 404.0f);
	QuadDesc outer;
	outer.position = vignettePos;
	outer.size = vignetteSize;
	outer.color = Color(0.04f, 0.04f, 0.05f, 0.32f);
	outer.screenSpace = true;
	Renderer::DrawQuad(outer);

	QuadDesc inner;
	inner.position = vignettePos + Vector2(6.0f, 6.0f);
	inner.size = vignetteSize - Vector2(12.0f, 12.0f);
	inner.color = Color(0.07f, 0.07f, 0.09f, 0.26f);
	inner.screenSpace = true;
	Renderer::DrawQuad(inner);
}

void DemoApp::CreateSceneContent() {
	GameObject* backdrop = GameObject::Create("backdrop");
	backdrop->x = 0.0f;
	backdrop->y = 0.0f;
	backdrop->width = 1280.0f;
	backdrop->height = 720.0f;
	backdrop->color = Color(0.05f, 0.06f, 0.07f, 1.0f);
	backdrop->alpha = 1.0f;

	GameObject* floor = GameObject::Create("floor");
	floor->x = 60.0f;
	floor->y = 420.0f;
	floor->width = 540.0f;
	floor->height = 36.0f;
	floor->color = Color(0.88f, 0.82f, 0.62f, 1.0f);
	floor->alpha = 1.0f;
	floor->collision = false;

	GameObject* accent = GameObject::Create("accent_strip");
	accent->x = 120.0f;
	accent->y = 148.0f;
	accent->width = 420.0f;
	accent->height = 12.0f;
	accent->color = Color(0.94f, 0.78f, 0.28f, 0.65f);
	accent->alpha = 0.65f;
	accent->collision = false;

	m_Focus = GameObject::Create("focus_cube");
	m_Focus->x = 320.0f;
	m_Focus->y = 220.0f;
	m_Focus->width = 120.0f;
	m_Focus->height = 120.0f;
	m_Focus->color = Color(0.72f, 0.58f, 0.96f, 1.0f);
	m_Focus->alpha = 1.0f;
	m_Focus->collision = false;
}

void DemoApp::LoadAudioAssets() {
	namespace fs = std::filesystem;
	if (m_AssetsRoot.empty()) {
		m_AssetsRoot = ResolveAssetsDirectory();
	}

	const fs::path audioDir = m_AssetsRoot / "audio";

	const fs::path clickPath = audioDir / "ui_click.wav";
	if (fs::exists(clickPath)) {
		m_ClickSound = SoundManager::Load("ui_click", clickPath.string());
	}

	const fs::path ambientPath = audioDir / "ambient.wav";
	if (fs::exists(ambientPath)) {
		m_AmbientSound = SoundManager::Load("ambient_loop", ambientPath.string(), true);
		if (m_AmbientSound && m_AmbientSound->IsValid()) {
			m_AmbientSound->SetLooping(true);
			if (!m_Headless) {
				m_AmbientSound->Play(false);
				m_AmbientEnabled = true;
			}
		}
	}

}

void DemoApp::LoadFonts() {
	namespace fs = std::filesystem;

	if (m_AssetsRoot.empty()) {
		m_AssetsRoot = ResolveAssetsDirectory();
	}

	const fs::path fontsDir = m_AssetsRoot / "fonts";
	std::unordered_set<std::string> seenKeys;
	std::vector<std::string> registered;
	auto appendRegistered = [&](const std::vector<std::string>& names) {
		for (const auto& key : names) {
			if (key.empty()) {
				continue;
			}
			if (seenKeys.insert(key).second) {
				registered.push_back(key);
			}
		}
	};

	if (fs::exists(fontsDir)) {
		appendRegistered(FontManager::RegisterFontsInDirectory(fontsDir, false));
	}

	const fs::path customDir = fontsDir / "custom";
	if (fs::exists(customDir)) {
		appendRegistered(FontManager::RegisterFontsInDirectory(customDir, true));
	}

	auto parsePathList = [](const char* raw) {
		std::vector<fs::path> result;
		if (!raw || !*raw) {
			return result;
		}
#ifdef _WIN32
		const char delimiter = ';';
#else
		const char delimiter = ':';
#endif
		std::stringstream stream(raw);
		std::string token;
		while (std::getline(stream, token, delimiter)) {
			const auto begin = token.find_first_not_of(" \t\r\n\"");
			const auto end = token.find_last_not_of(" \t\r\n\"");
			if (begin == std::string::npos || end == std::string::npos) {
				continue;
			}
			std::string trimmed = token.substr(begin, end - begin + 1);
			if (!trimmed.empty()) {
				result.emplace_back(trimmed);
			}
		}
		return result;
	};

	auto registerExternal = [&](const std::vector<fs::path>& paths, bool recursive) {
		for (const auto& entry : paths) {
			if (entry.empty()) {
				continue;
			}
			std::error_code ec;
			if (fs::is_directory(entry, ec) && !ec) {
				appendRegistered(FontManager::RegisterFontsInDirectory(entry, recursive));
			}
			else if (fs::is_regular_file(entry, ec) && !ec) {
				if (auto key = FontManager::RegisterFontFile(entry)) {
					if (seenKeys.insert(*key).second) {
						registered.push_back(*key);
					}
				}
			}
			else {
				SAGE_TRACE("DemoApp: пропущен путь '{}' при регистрации шрифтов", entry.string());
			}
		}
	};

	registerExternal(parsePathList(std::getenv("SAGE_FONT_DIRS")), true);
	registerExternal(parsePathList(std::getenv("SAGE_FONT_FILES")), false);

	auto setDefaultFromList = [&](std::initializer_list<const char*> names) {
		for (const char* candidate : names) {
			if (candidate && FontManager::IsRegistered(candidate)) {
				if (FontManager::SetDefaultFontOverrideByName(candidate)) {
					return true;
				}
			}
		}
		return false;
	};

	if (!setDefaultFromList({ "Inter-Regular", "Montserrat-Regular", "Roboto-Regular" })) {
		if (!registered.empty()) {
			FontManager::SetDefaultFontOverrideByName(registered.front());
		}
	}

	auto loadPreferred = [&](std::initializer_list<const char*> names, float size) -> Ref<Font> {
		for (const char* candidate : names) {
			if (!candidate) {
				continue;
			}
			if (!FontManager::IsRegistered(candidate)) {
				continue;
			}
			Ref<Font> font = FontManager::LoadRegistered(candidate, size);
			if (font && font->IsLoaded()) {
				return font;
			}
		}
		return nullptr;
	};

	m_TitleFont = loadPreferred({ "Inter-Bold", "Inter-SemiBold", "Montserrat-Bold", "Roboto-Bold" }, 34.0f);
	m_BodyFont = loadPreferred({ "Inter-Regular", "Montserrat-Regular", "Roboto-Regular" }, 22.0f);
	m_ButtonFont = loadPreferred({ "Inter-Medium", "Inter-SemiBold", "Montserrat-SemiBold", "Roboto-Medium" }, 24.0f);

	if (!m_TitleFont) {
		m_TitleFont = FontManager::GetDefault(34.0f);
	}
	if (!m_BodyFont) {
		m_BodyFont = FontManager::GetDefault(22.0f);
	}
	if (!m_ButtonFont) {
		m_ButtonFont = FontManager::GetDefault(24.0f);
	}
	}

void DemoApp::BuildHud() {
	const UI::Color kPanelBackground(0.99f, 0.95f, 0.82f, 0.78f);
	const UI::Color kPanelBorder(0.96f, 0.83f, 0.33f, 0.9f);
	const UI::Color kPanelShadow(0.0f, 0.0f, 0.0f, 0.36f);
	const UI::Color kPanelAccent(1.0f, 0.98f, 0.88f, 0.52f);
	const UI::Color kPrimaryText(0.19f, 0.14f, 0.06f, 1.0f);
	const UI::Color kSecondaryText(0.33f, 0.25f, 0.12f, 1.0f);
	const UI::Color kButtonNormal(0.31f, 0.56f, 0.52f, 0.9f);
	const UI::Color kButtonHover(0.36f, 0.63f, 0.59f, 0.95f);
	const UI::Color kButtonPressed(0.24f, 0.46f, 0.44f, 0.95f);
	const UI::Color kButtonBorder(0.17f, 0.32f, 0.31f, 0.9f);
	const UI::Color kProgressFill(0.95f, 0.67f, 0.17f, 0.92f);
	const UI::Color kProgressTrack(1.0f, 0.98f, 0.9f, 0.56f);
	const float kBlockSpacing = 14.0f;
	const UI::Color kTestPanelBackground(0.18f, 0.2f, 0.26f, 0.9f);
	const UI::Color kTestPanelBorder(0.34f, 0.46f, 0.68f, 0.95f);
	const UI::Color kTestPanelShadow(0.0f, 0.0f, 0.0f, 0.4f);
	const UI::Color kTestPrimaryText(0.92f, 0.96f, 1.0f, 1.0f);
	const UI::Color kTestSecondaryText(0.74f, 0.82f, 0.95f, 1.0f);
	const UI::Color kTestButtonNormal(0.28f, 0.5f, 0.82f, 0.95f);
	const UI::Color kTestButtonHover(0.32f, 0.58f, 0.88f, 0.97f);
	const UI::Color kTestButtonPressed(0.24f, 0.44f, 0.72f, 0.97f);
	const UI::Color kTestButtonBorder(0.18f, 0.3f, 0.48f, 0.96f);
	const float kTestSpacing = 12.0f;

	UI::PanelConfig hudPanel{};
	hudPanel.id = "panel_hud";
	hudPanel.position = Vector2(28.0f, 28.0f);
	hudPanel.size = Vector2(340.0f, 380.0f);
	hudPanel.backgroundColor = kPanelBackground;
	hudPanel.borderColor = kPanelBorder;
	hudPanel.borderThickness = 3.0f;
	hudPanel.shadowColor = kPanelShadow;
	hudPanel.shadowOffset = Vector2(10.0f, 10.0f);
	hudPanel.contentPadding = Vector2(20.0f, 24.0f);
	hudPanel.title.text = "SAGE Engine Demo";
	hudPanel.title.scale = 1.08f;
	hudPanel.title.color = kPrimaryText;
	hudPanel.title.offset = Vector2(20.0f, 18.0f);
	hudPanel.title.backgroundColor = UI::Color(1.0f, 0.99f, 0.94f, 0.65f);
	hudPanel.title.backgroundPadding = Vector2(14.0f, 6.0f);
	hudPanel.draggable = true;
	hudPanel.dragHandleHeight = 64.0f;
	hudPanel.visible = true;
	hudPanel.title.font = m_TitleFont;
	m_HudPanel = UI::UISystem::CreatePanel(hudPanel);
	if (m_HudPanel) {
		m_HudPanel->SetVisible(true);
	}

	const Vector2 fallbackOrigin(hudPanel.position.x + 20.0f, hudPanel.position.y + 72.0f);
	const Vector2 fallbackSize(hudPanel.size.x - 40.0f, hudPanel.size.y - 80.0f);
	const Vector2 contentOrigin = m_HudPanel ? m_HudPanel->GetContentPosition() : fallbackOrigin;
	const Vector2 contentSize = m_HudPanel ? m_HudPanel->GetContentSize() : fallbackSize;

	Ref<Font> baseFont = (m_BodyFont && m_BodyFont->IsLoaded()) ? m_BodyFont : FontManager::GetDefault();
	auto measureBlock = [&](const std::string& text, float scale, const Ref<Font>& font, const Vector2& padding) {
		Ref<Font> useFont = (font && font->IsLoaded()) ? font : baseFont;
		Float2 textSize = Float2::Zero();
		if (useFont && useFont->IsLoaded()) {
			textSize = Renderer::MeasureText(text, useFont, scale);
		}
		return Vector2(textSize.x + padding.x * 2.0f, textSize.y + padding.y * 2.0f);
	};

	float cursorY = 0.0f;
	auto placeLabel = [&](UI::LabelConfig& cfg, const std::string& previewText) {
		Vector2 blockSize = measureBlock(previewText, cfg.scale, cfg.font, cfg.backgroundPadding);
		Vector2 desired = contentOrigin + Vector2(0.0f, cursorY);
		if (m_HudPanel) {
			cfg.position = m_HudPanel->ClampToContent(desired, blockSize);
		} else {
			cfg.position = desired;
		}
		cursorY += blockSize.y + kBlockSpacing;
	};

	UI::LabelConfig header{};
	header.id = "lbl_header";
	header.text = "Scene Overview";
	header.scale = 0.95f;
	header.color = kSecondaryText;
	header.backgroundColor = kPanelAccent;
	header.backgroundPadding = Vector2(12.0f, 6.0f);
	header.shadowColor = UI::Color::Transparent();
	header.font = m_TitleFont;
	placeLabel(header, header.text);
	if (auto* createdHeader = UI::UISystem::CreateLabel(header)) {
		createdHeader->SetVisible(true);
	}

	UI::LabelConfig info{};
	info.id = "lbl_info";
	info.scale = 0.9f;
	info.color = kPrimaryText;
	info.backgroundColor = UI::Color(1.0f, 0.99f, 0.92f, 0.42f);
	info.backgroundPadding = Vector2(12.0f, 6.0f);
	info.shadowColor = UI::Color::Transparent();
	info.textProvider = [this]() {
		std::ostringstream stream;
		stream << "Scene Stats\n";
		stream << "  Objects: " << GameObject::Count() << "\n";
		stream << "  Time Elapsed: " << std::fixed << std::setprecision(1) << m_PlayTime << " s";
		return stream.str();
	};
	const std::string infoPreview = info.textProvider ? info.textProvider() : info.text;
	info.font = m_BodyFont;
	placeLabel(info, infoPreview);
	if (auto* createdInfo = UI::UISystem::CreateLabel(info)) {
		createdInfo->SetVisible(true);
	}

	UI::LabelConfig status{};
	status.id = "lbl_status";
	status.scale = 0.88f;
	status.color = kPrimaryText;
	status.backgroundColor = UI::Color(0.99f, 0.97f, 0.88f, 0.36f);
	status.backgroundPadding = Vector2(12.0f, 6.0f);
	status.shadowColor = UI::Color::Transparent();
	status.textProvider = [this]() {
		std::ostringstream stream;
		stream << "Status\n";
		stream << "  Ambient Audio: " << (m_AmbientEnabled ? "On" : "Off") << "\n";
		stream << "  Auto-click Sound: " << (m_ClickSound && m_ClickSound->IsValid() ? "On" : "Off") << "\n";
		stream << "  Color Freeze: " << (m_ColorFrozen ? "On" : "Off");
		return stream.str();
	};
	const std::string statusPreview = status.textProvider ? status.textProvider() : status.text;
	status.font = m_BodyFont;
	placeLabel(status, statusPreview);
	if (auto* createdStatus = UI::UISystem::CreateLabel(status)) {
		createdStatus->SetVisible(true);
	}

	UI::LabelConfig controls{};
	controls.id = "lbl_controls";
	controls.text = "Controls\n  ESC - Exit demo\n  M - Toggle ambient audio\n  C - Freeze cube color";
	controls.scale = 0.86f;
	controls.color = kPrimaryText;
	controls.backgroundColor = UI::Color(0.99f, 0.97f, 0.84f, 0.32f);
	controls.backgroundPadding = Vector2(12.0f, 6.0f);
	controls.shadowColor = UI::Color::Transparent();
	controls.font = m_BodyFont;
	placeLabel(controls, controls.text);
	if (auto* createdControls = UI::UISystem::CreateLabel(controls)) {
		createdControls->SetVisible(true);
	}

	Vector2 buttonSize(contentSize.x, 48.0f);
	Vector2 buttonPos = contentOrigin + Vector2(0.0f, cursorY);
	if (m_HudPanel) {
		buttonPos = m_HudPanel->ClampToContent(buttonPos, buttonSize);
	}

	UI::ButtonConfig ambientBtn{};
	ambientBtn.id = "btn_ambient";
	ambientBtn.position = buttonPos;
	ambientBtn.size = buttonSize;
	ambientBtn.textScale = 0.9f;
	ambientBtn.textColor = UI::Color(0.97f, 0.98f, 0.99f, 1.0f);
	ambientBtn.style.normalColor = kButtonNormal;
	ambientBtn.style.hoveredColor = kButtonHover;
	ambientBtn.style.pressedColor = kButtonPressed;
	ambientBtn.style.borderColor = kButtonBorder;
	ambientBtn.style.borderThickness = 2.0f;
	ambientBtn.font = m_ButtonFont;
	ambientBtn.onClick = [this]() {
		ToggleAmbient();
		if (m_ClickSound && m_ClickSound->IsValid()) {
			m_ClickSound->Play(true);
		}
		m_LastClickTime = m_PlayTime;
	};
	ambientBtn.interactable = true;
	ambientBtn.text = m_AmbientEnabled ? "Ambient audio: ON" : "Ambient audio: OFF";
	m_ToggleAmbientButton = UI::UISystem::CreateButton(ambientBtn);
	if (m_ToggleAmbientButton) {
		m_ToggleAmbientButton->SetVisible(true);
	}
	UpdateAmbientButtonAppearance();

	cursorY += buttonSize.y + kBlockSpacing;

	UI::LabelConfig progressCaption{};
	progressCaption.id = "lbl_progress_caption";
	progressCaption.text = "Auto-click cooldown";
	progressCaption.scale = 0.82f;
	progressCaption.color = kSecondaryText;
	progressCaption.backgroundColor = kPanelAccent;
	progressCaption.backgroundPadding = Vector2(10.0f, 6.0f);
	progressCaption.shadowColor = UI::Color::Transparent();
	progressCaption.font = m_BodyFont;
	placeLabel(progressCaption, progressCaption.text);
	if (auto* createdProgressCaption = UI::UISystem::CreateLabel(progressCaption)) {
		createdProgressCaption->SetVisible(true);
	}

	Vector2 barSize(contentSize.x, 24.0f);
	Vector2 barPos = contentOrigin + Vector2(0.0f, cursorY);
	if (m_HudPanel) {
		barPos = m_HudPanel->ClampToContent(barPos, barSize);
	}

	UI::ProgressBarConfig progress{};
	progress.id = "pb_autoclick";
	progress.position = barPos;
	progress.size = barSize;
	progress.minValue = 0.0f;
	progress.maxValue = 1.0f;
	progress.value = 0.0f;
	progress.showValueLabel = true;
	progress.textScale = 0.72f;
	progress.textColor = kPrimaryText;
	progress.style.backgroundColor = kProgressTrack;
	progress.style.fillColor = kProgressFill;
	progress.style.borderColor = kPanelBorder;
	progress.style.borderThickness = 2.0f;
	progress.valueProvider = [this]() {
		const float sinceClick = std::max(0.0f, m_PlayTime - m_LastClickTime);
		return std::min(sinceClick / kClickInterval, 1.0f);
	};
	progress.labelFormatter = [this](float, float normalized) {
		std::ostringstream stream;
		const float remaining = std::max(0.0f, (1.0f - normalized) * kClickInterval);
		stream << std::fixed << std::setprecision(1) << remaining << "s";
		return stream.str();
	};
	progress.font = m_BodyFont;
	m_ClickProgressBar = UI::UISystem::CreateProgressBar(progress);
	if (m_ClickProgressBar) {
		m_ClickProgressBar->SetVisible(true);
	}

	cursorY += barSize.y + kBlockSpacing;

	std::array<unsigned char, 16 * 16 * 4> badgePixels{};
	for (int y = 0; y < 16; ++y) {
		for (int x = 0; x < 16; ++x) {
			const bool border = x == 0 || y == 0 || x == 15 || y == 15;
			const bool diagonal = x == y || x + y == 15;
			const std::size_t index = static_cast<std::size_t>((y * 16 + x) * 4);

			unsigned char r = border ? 210 : (diagonal ? 110 : 40);
			unsigned char g = border ? 230 : (diagonal ? 150 : 90);
			unsigned char b = border ? 255 : (diagonal ? 210 : 150);
			badgePixels[index + 0] = r;
			badgePixels[index + 1] = g;
			badgePixels[index + 2] = b;
			badgePixels[index + 3] = 255;
		}
	}

	m_UiBadge = CreateRef<Texture>(16, 16, Texture::Format::RGBA, badgePixels.data());

	UI::ImageConfig badge{};
	badge.id = "img_badge";
	badge.position = Vector2(hudPanel.position.x + hudPanel.size.x - 76.0f, hudPanel.position.y + 30.0f);
	badge.size = Vector2(60.0f, 60.0f);
	badge.texture = m_UiBadge;
	badge.tint = UI::Color(1.0f, 1.0f, 1.0f, 0.95f);
	UI::UISystem::CreateImage(badge);

	UI::PanelConfig testPanel{};
	testPanel.id = "panel_ui_test";
	testPanel.position = Vector2(hudPanel.position.x + hudPanel.size.x + 36.0f, hudPanel.position.y + 48.0f);
	testPanel.size = Vector2(300.0f, 280.0f);
	testPanel.backgroundColor = kTestPanelBackground;
	testPanel.borderColor = kTestPanelBorder;
	testPanel.borderThickness = 2.5f;
	testPanel.shadowColor = kTestPanelShadow;
	testPanel.shadowOffset = Vector2(8.0f, 8.0f);
	testPanel.contentPadding = Vector2(18.0f, 22.0f);
	testPanel.title.text = "UI тесты";
	testPanel.title.scale = 1.02f;
	testPanel.title.color = kTestSecondaryText;
	testPanel.title.offset = Vector2(16.0f, 16.0f);
	testPanel.title.backgroundColor = UI::Color(1.0f, 1.0f, 1.0f, 0.12f);
	testPanel.title.backgroundPadding = Vector2(12.0f, 6.0f);
	testPanel.draggable = true;
	testPanel.dragHandleHeight = 56.0f;
	testPanel.visible = true;
	testPanel.title.font = m_TitleFont;
	m_TestPanel = UI::UISystem::CreatePanel(testPanel);
	if (m_TestPanel) {
		m_TestPanel->SetVisible(true);
	}

	const Vector2 testFallbackOrigin(testPanel.position.x + 18.0f, testPanel.position.y + 72.0f);
	const Vector2 testFallbackSize(testPanel.size.x - 36.0f, testPanel.size.y - 90.0f);
	const Vector2 testContentOrigin = m_TestPanel ? m_TestPanel->GetContentPosition() : testFallbackOrigin;
	const Vector2 testContentSize = m_TestPanel ? m_TestPanel->GetContentSize() : testFallbackSize;

	float testCursorY = 0.0f;
	auto placeTestLabel = [&](UI::LabelConfig& cfg, const std::string& previewText) {
		Vector2 blockSize = measureBlock(previewText, cfg.scale, cfg.font, cfg.backgroundPadding);
		Vector2 desired = testContentOrigin + Vector2(0.0f, testCursorY);
		if (m_TestPanel) {
			cfg.position = m_TestPanel->ClampToContent(desired, blockSize);
		} else {
			cfg.position = desired;
		}
		testCursorY += blockSize.y + kTestSpacing;
	};

	UI::LabelConfig testHeader{};
	testHeader.id = "lbl_ui_test_header";
	testHeader.text = "Быстрые проверки";
	testHeader.scale = 0.94f;
	testHeader.color = kTestSecondaryText;
	testHeader.backgroundColor = UI::Color(1.0f, 1.0f, 1.0f, 0.1f);
	testHeader.backgroundPadding = Vector2(10.0f, 6.0f);
	testHeader.shadowColor = UI::Color::Transparent();
	testHeader.font = m_TitleFont;
	placeTestLabel(testHeader, testHeader.text);
	if (auto* createdTestHeader = UI::UISystem::CreateLabel(testHeader)) {
		createdTestHeader->SetVisible(true);
	}

	UI::LabelConfig testDescription{};
	testDescription.id = "lbl_ui_test_desc";
	testDescription.text = "Набор инструментов для ручной проверки UI.\nЗапустите сценарий, чтобы обновить состояние панелей.";
	testDescription.scale = 0.82f;
	testDescription.color = kTestPrimaryText;
	testDescription.backgroundColor = UI::Color(1.0f, 1.0f, 1.0f, 0.08f);
	testDescription.backgroundPadding = Vector2(10.0f, 6.0f);
	testDescription.shadowColor = UI::Color::Transparent();
	testDescription.font = m_BodyFont;
	const std::string testDescriptionPreview = testDescription.text;
	placeTestLabel(testDescription, testDescriptionPreview);
	if (auto* createdTestDescription = UI::UISystem::CreateLabel(testDescription)) {
		createdTestDescription->SetVisible(true);
	}

	Vector2 testButtonSize(testContentSize.x, 44.0f);
	Vector2 testButtonPos = testContentOrigin + Vector2(0.0f, testCursorY);
	if (m_TestPanel) {
		testButtonPos = m_TestPanel->ClampToContent(testButtonPos, testButtonSize);
	}

	UI::ButtonConfig testButton{};
	testButton.id = "btn_run_ui_test";
	testButton.position = testButtonPos;
	testButton.size = testButtonSize;
	testButton.textScale = 0.88f;
	testButton.textColor = UI::Color(0.97f, 0.99f, 1.0f, 1.0f);
	testButton.style.normalColor = kTestButtonNormal;
	testButton.style.hoveredColor = kTestButtonHover;
	testButton.style.pressedColor = kTestButtonPressed;
	testButton.style.borderColor = kTestButtonBorder;
	testButton.style.borderThickness = 2.0f;
	testButton.font = m_ButtonFont;
	testButton.text = m_HasRunUiTest ? "Повторить UI-тест" : "Запустить UI-тест";
	testButton.onClick = [this]() {
		RunUiTest();
	};
	m_RunUiTestButton = UI::UISystem::CreateButton(testButton);
	if (m_RunUiTestButton) {
		m_RunUiTestButton->SetVisible(true);
	}

	testCursorY += testButtonSize.y + kTestSpacing;

	UI::LabelConfig testStatus{};
	testStatus.id = "lbl_ui_test_status";
	testStatus.scale = 0.82f;
	testStatus.color = kTestSecondaryText;
	testStatus.backgroundColor = UI::Color(1.0f, 1.0f, 1.0f, 0.1f);
	testStatus.backgroundPadding = Vector2(10.0f, 6.0f);
	testStatus.shadowColor = UI::Color::Transparent();
	testStatus.textProvider = [this]() {
		std::ostringstream stream;
		stream << "Статус UI-теста\n";
		if (m_LastUiTestTime < 0.0f) {
			stream << "  • Не запускался";
		} else {
			const float elapsed = std::max(0.0f, m_PlayTime - m_LastUiTestTime);
			stream << "  • Последний запуск: " << std::fixed << std::setprecision(1) << elapsed << " с назад";
		}
		return stream.str();
	};
	const std::string testStatusPreview = testStatus.textProvider ? testStatus.textProvider() : testStatus.text;
	testStatus.font = m_BodyFont;
	placeTestLabel(testStatus, testStatusPreview);
	m_TestStatusLabel = UI::UISystem::CreateLabel(testStatus);
	if (m_TestStatusLabel) {
		m_TestStatusLabel->SetVisible(true);
		m_TestStatusBaseColor = UI::Color(0.78f, 0.86f, 0.98f, 1.0f);
		m_TestStatusLabel->SetColor(m_TestStatusBaseColor);
	}
}

void DemoApp::ToggleAmbient() {
	if (!m_AmbientSound || !m_AmbientSound->IsValid()) {
		m_AmbientEnabled = false;
		UpdateAmbientButtonAppearance();
		return;
	}

	if (m_AmbientEnabled) {
		m_AmbientSound->Stop();
		m_AmbientEnabled = false;
	} else {
		m_AmbientSound->Play(false);
		m_AmbientEnabled = true;
	}

	UpdateAmbientButtonAppearance();
}

void DemoApp::UpdateAmbientButtonAppearance() {
	if (!m_ToggleAmbientButton) {
		return;
	}

	auto& style = m_ToggleAmbientButton->GetStyle();
	style.borderColor = UI::Color(0.93f, 0.87f, 0.62f, 0.92f);
	style.borderThickness = 2.4f;
	m_ToggleAmbientButton->SetTextColor(UI::Color(0.97f, 0.98f, 0.99f, 1.0f));

	if (m_AmbientEnabled) {
		style.normalColor = UI::Color(0.28f, 0.58f, 0.46f, 0.95f);
		style.hoveredColor = UI::Color(0.32f, 0.64f, 0.52f, 0.96f);
		style.pressedColor = UI::Color(0.24f, 0.5f, 0.42f, 0.96f);
		m_ToggleAmbientButton->SetText("Ambient audio: ON");
	} else {
		style.normalColor = UI::Color(0.42f, 0.44f, 0.5f, 0.95f);
		style.hoveredColor = UI::Color(0.46f, 0.48f, 0.56f, 0.96f);
		style.pressedColor = UI::Color(0.36f, 0.38f, 0.46f, 0.96f);
		m_ToggleAmbientButton->SetText("Ambient audio: OFF");
	}
}

void DemoApp::RunUiTest() {
	m_LastUiTestTime = m_PlayTime;
	m_TestHighlightTimer = kTestHighlightDuration;
	m_HasRunUiTest = true;

	if (m_RunUiTestButton) {
		m_RunUiTestButton->SetText("Повторить UI-тест");
	}

	if (m_TestStatusLabel) {
		m_TestStatusLabel->SetColor(m_TestStatusHighlightColor);
	}

	if (m_ClickSound && m_ClickSound->IsValid()) {
		m_ClickSound->Play(true);
		m_LastClickTime = m_PlayTime;
	}
}

Application* SAGE::CreateApplication() {
	return new DemoApp();
}

int main(int argc, char** argv) {
	bool headless = false;
	float duration = -1.0f;

	for (int i = 1; i < argc; ++i) {
		std::string arg(argv[i]);
		if (arg == "--headless") {
			headless = true;
		} else if (arg == "--duration" && i + 1 < argc) {
			duration = std::max(0.0f, static_cast<float>(std::atof(argv[++i])));
		}
	}

	Scope<Application> app(SAGE::CreateApplication());
	if (auto* demo = dynamic_cast<DemoApp*>(app.get())) {
		const float requestedDuration = duration >= 0.0f ? duration : (headless ? 2.0f : -1.0f);
		demo->ConfigureRuntime(headless, requestedDuration);
	}
	app->Run();
	return 0;
}

