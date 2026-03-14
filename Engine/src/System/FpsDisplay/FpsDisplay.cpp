#include "pch.h"
#include "FpsDisplay.hpp"
#include<Debug/ImGui/ImGuiManager.hpp>

bool Ecse::System::FpsDisplay::StartMonitoring()
{
	auto imgui = System::ServiceLocator::Get<Debug::ImGuiManager>();
	if (!imgui) return false;

	imgui->AddDebugUI([]() {
		ImGui::Begin("Performance");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		ImGui::Text("ms: %.3f", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::End();
		});
}
