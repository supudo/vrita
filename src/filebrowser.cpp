#include "../include/filebrowser.hpp"

#include <algorithm>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlgpu3.h>

namespace fs = std::filesystem;

void FileBrowser::init(const std::function<void(const char*)>& processFileFunction) {
	processFile = processFileFunction;
}

void FileBrowser::render(bool* p_opened, std::string const& emulatorType) {
	if (currentFolder.empty())
		currentFolder = fs::current_path().string();

	ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);

	ImGui::Begin("ROM file", p_opened);
	ImGui::Text("%s", this->currentFolder.c_str());
	ImGui::Separator();

	// folder browser
	ImGui::BeginChild("scrolling");
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));

	// Basic columns
	ImGui::Columns(3, "fileColumns");

	ImGui::Separator();
	ImGui::Text("File");
	ImGui::NextColumn();
	ImGui::Text("Size");
	ImGui::NextColumn();
	ImGui::Text("Last Modified");
	ImGui::NextColumn();
	ImGui::Separator();

	this->drawFiles(this->currentFolder, emulatorType);

	ImGui::Columns(1);

	ImGui::Separator();
	ImGui::Spacing();

	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::End();
}

void FileBrowser::drawFiles(const std::string& fPath, std::string const& emulatorType) {
	std::string cFolder = fPath;
	std::map<std::string, VritaUtils::FBEntity> folderContents = this->getFolderContents(cFolder, emulatorType);
	int i = 0;
	static int selected = -1;
	for (std::map<std::string, VritaUtils::FBEntity>::iterator iter = folderContents.begin(); iter != folderContents.end(); ++iter) {
		VritaUtils::FBEntity entity = iter->second;
		if (ImGui::Selectable(entity.title.c_str(), selected == i, ImGuiSelectableFlags_SpanAllColumns)) {
			selected = i;
			if (entity.isFile)
				processFile(entity.path.c_str());
			else {
				try {
					this->drawFiles(entity.path, emulatorType);
					this->currentFolder = entity.path;
				}
				catch (const fs::filesystem_error&) { }
			}
		}
		ImGui::NextColumn();

		ImGui::Text("%s", entity.size.c_str()); ImGui::NextColumn();
		ImGui::Text("%s", entity.modifiedDate.c_str()); ImGui::NextColumn();

		i += 1;
	}
}

std::map<std::string, VritaUtils::FBEntity> FileBrowser::getFolderContents(std::string const& filePath, std::string const& emulatorType) {
	std::map<std::string, VritaUtils::FBEntity> folderContentsAll = VritaUtils::getFolderContents(filePath);
	std::map<std::string, VritaUtils::FBEntity> folderContents;

	auto it = allowedExtensions.find(emulatorType);
	if (it == allowedExtensions.end())
		return folderContents;
	const auto& allowed = it->second;

	for (const auto& [key, file] : folderContentsAll) {
		if (!file.isFile) {
			folderContents[file.path] = file;
			continue;
		}
		std::filesystem::path p(file.path);
		std::string filename = p.filename().string();
		if (!filename.empty() && filename[0] == '.')
			continue;
		std::string ext = p.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), [] (unsigned char c) { return std::tolower(c); });
		if (allowed.contains(ext))
			folderContents[file.path] = file;
	}

	return folderContents;
}
