#include "Drawing.h"
#include "FileTools.h"

#include <iostream>
#include <string>
#include <windows.h>

#include "json.hpp"

#pragma warning(push)
#pragma warning(disable : 4996)

std::string MyPath = []() -> std::string
	{
		char* path = new char[MAX_PATH];
		GetModuleFileNameA(NULL, path, MAX_PATH);
		{
			if (char* c = strrchr(path, '\\'))
				*c = 0;

			std::string strPath(path);
			
			delete[] path;
			return strPath;
		}
		delete[] path;
		return "";
	}();

std::string ConvertWideToUTF8(const std::wstring& wideStr) 
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, NULL, 0, NULL, NULL);

	char* utf8Str = new char[size_needed];

	WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, utf8Str, size_needed, NULL, NULL);

	std::string result(utf8Str);

	delete[] utf8Str;

	return result;
}


LPCSTR Drawing::lpWindowName = "Battle Parameter Editor";
ImVec2 Drawing::vWindowSize = { 350, 75 };
ImGuiWindowFlags Drawing::WindowFlags = ImGuiWindowFlags_MenuBar;
bool Drawing::bDraw = true;

void Drawing::Active()
{
	bDraw = true;
}

bool Drawing::isActive()
{
	return bDraw == true;
}

std::vector<std::pair<std::string, std::pair<int, std::string>>> Objects; // Load the JSON data only once, and change the units according if it found a object
std::vector<std::pair<int, std::string>> CurrentUnits;

std::vector<std::pair<int, std::string>> GetObjectUnit(const char* objectName)
{
	std::vector<std::pair<int, std::string>> result;
	for (const auto& object : Objects)
	{
		if (object.first == objectName)
		{
			result.push_back(object.second);
		}
	}
	return result;
}

void ReadObjectsJSON()
{
	Objects.clear();
	CurrentUnits.clear();

	FILE* file = fopen((MyPath + "\\data\\objects.json").c_str(), "r");
	printf("path: %s\n", (MyPath + "\\data\\objects.json").c_str());
	if (file)
	{
		nlohmann::json jsonData = nlohmann::json::parse(file, nullptr, false, true);
		for (const auto& item : jsonData.items())
		{
			std::string objectName = item.key();
			printf("object: %s\n", objectName.c_str());
			const nlohmann::json& unitArray = item.value();
			for (const auto &unit : unitArray)
			{
				if (!unit.is_object())
					continue;

				int id = unit.at("id").get<int>();
				std::string desc = unit.at("name").get<std::string>();
				printf("\t unit: %d, desc: %s\n", id, desc.c_str());
				Objects.emplace_back(std::pair<std::string, std::pair<int, std::string>>(objectName, {id, desc}));

				// printf("I think object %s with %d and %s was added\n", Objects.back().first.c_str(), Objects.back().second.first, Objects.back().second.second.c_str());
			}
		}
		fclose(file);
	}
}

std::string AutoGuessObject = "";

void Drawing::Draw()
{
	if (isActive())
	{
		ImGui::SetNextWindowSize(vWindowSize, ImGuiCond_Once);
		ImGui::SetNextWindowBgAlpha(1.0f);

		ImGui::Begin(lpWindowName, &bDraw, WindowFlags);
		
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save", ""))
				{
					if (FileTools::Units.empty())
					{
						ImGui::OpenPopup("No Units");
					}
					else
					{
						FileTools::SaveUnits();
						FileTools::SaveFile(nullptr);
					}
				}
				if (ImGui::MenuItem("Save As", ""))
				{
					if (FileTools::Units.empty())
					{
						ImGui::OpenPopup("No Units");
					}
					else
					{
						FileTools::SaveUnits();

						OPENFILENAMEW ofn = { 0 };
						wchar_t szFileName[MAX_PATH] = { 0 };

						ZeroMemory(&ofn, sizeof(ofn));
						ofn.lStructSize = sizeof(ofn);
						ofn.hwndOwner = NULL;
						ofn.lpstrFile = szFileName;
						ofn.nMaxFile = sizeof(szFileName) / sizeof(wchar_t);
						ofn.lpstrFilter = L"Battle Parameter file\0*.BIN\0All files\0*.*\0";
						ofn.nFilterIndex = 1;
						ofn.lpstrFileTitle = NULL;
						ofn.nMaxFileTitle = 0;
						ofn.lpstrInitialDir = NULL;
						ofn.lpstrTitle = L"Save File As";
						ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

						if (GetSaveFileNameW(&ofn))
						{
							FileTools::SaveFile(szFileName);
						}
					}
				}
				if (ImGui::MenuItem("Open", ""))
				{
					OPENFILENAMEW ofn = { 0 };
					wchar_t szFileName[MAX_PATH] = { 0 };
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = NULL;
					ofn.lpstrFile = szFileName;
					ofn.nMaxFile = sizeof(szFileName) / sizeof(wchar_t);
					ofn.lpstrFilter = L"Battle Parameter file\0*.BIN\0All files\0*.*\0";
					ofn.nFilterIndex = 1;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrTitle = L"Open File";
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;

					if (GetOpenFileNameW(&ofn))
					{
						FileTools::Cleanup();
						FileTools::OpenFile(szFileName);
						FileTools::ReadUnits();

						ReadObjectsJSON();

						std::string AutoGuessedObject = "";
						if (FileTools::LastPath[0] != 0)
						{
							std::string path = ConvertWideToUTF8(FileTools::LastPath).c_str();

							const char* object = strrchr(path.c_str(), '\\');
							char* _object = (char*)_malloca(strlen(object) + 1);

							if (_object)
							{
								strcpy_s(_object, strlen(object) + 1, object + 1); // skip the backslash
								_object[6] = '\0';
								AutoGuessedObject = _object;
								::AutoGuessObject = _object;
								_freea(_object);
							}
						}

						CurrentUnits.clear();
						CurrentUnits = GetObjectUnit(AutoGuessObject.c_str());
					}
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		if (ImGui::BeginPopup("No Units"))
		{
			ImGui::Text("There are no units to save.");
			ImGui::EndPopup();
		}

		if (!FileTools::Units.empty())
		{
			ImGui::Text("Opened File: %s", ConvertWideToUTF8(FileTools::LastPath).c_str());
			ImGui::Text("Units: %d", FileTools::Units.size());
			ImGui::Text("Autoguessed object: ");
			ImGui::SameLine();
			if (ImGui::BeginCombo("##ObjectCombo", AutoGuessObject.c_str()))
			{
				for (const auto& object : Objects)
				{
					const bool bSelected = (object.first == AutoGuessObject);

					if (ImGui::Selectable(object.first.c_str(), bSelected))
					{
						AutoGuessObject = object.first;
						CurrentUnits.clear();

						CurrentUnits = GetObjectUnit(AutoGuessObject.c_str());
					}

					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::Separator();
			if (ImGui::Button("Add Unit"))
			{
				BattleParameter::Unit* newUnit = new BattleParameter::Unit();
				memset(newUnit, 0, sizeof(BattleParameter::Unit));
				newUnit->m_id = FileTools::Units.size() + 1; // assigning a new ID based on the current size
				if (newUnit)
					FileTools::AppendUnit(newUnit);
			}

			for (auto& unit : FileTools::Units)
			{
				if (!unit)
					continue;
				
				std::string unitName = std::format("Unit {}", unit->m_id);
				if (!CurrentUnits.empty())
				{
					for (const auto& currentUnit : CurrentUnits)
					{
						if (currentUnit.first == unit->m_id)
						{
							unitName = currentUnit.second;
							break;
						}
					}
				}

				if (ImGui::TreeNode(unit, unitName.c_str()))
				{
					std::string* StringToChange = nullptr;
					if (!CurrentUnits.empty())
					{
						for (auto& currentUnit : CurrentUnits)
						{
							if (currentUnit.first == unit->m_id)
							{
								StringToChange = &currentUnit.second;
								break;
							}
						}
					}
					if (ImGui::BeginPopup("Change Name"))
					{
						static char lStringToChange[256] = { 0 };
						ImGui::InputText("New Name", lStringToChange, 256);
						if (ImGui::Button("OK"))
						{
							if (StringToChange)
								*StringToChange = lStringToChange;
							else
								CurrentUnits.emplace_back(unit->m_id, lStringToChange);

							lStringToChange[0] = 0;

							ImGui::CloseCurrentPopup();
						}
						ImGui::SameLine();
						if (ImGui::Button("Cancel"))
						{
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					{
						ImGui::OpenPopup("Change Name");
					}
					ImGui::InputInt("ID", &unit->m_id);
					ImGui::InputInt("Attack Power", &unit->m_AtkPower);
					ImGui::InputInt("Attack Havok Mul Scalar", &unit->m_AtkHavokMulScalar);
					ImGui::InputInt("Attack Havok Pow", &unit->m_AtkHavokPow);
					ImGui::InputInt("Hit Stop Time", &unit->m_HitStopTime);
					ImGui::InputInt("Int 0", &unit->m_Int0);
					ImGui::InputInt("Int 1", &unit->m_Int1);
					ImGui::InputFloat("Float 0", &unit->m_Float0);
					ImGui::InputFloat("Float 1", &unit->m_Float1);
					ImGui::InputFloat("Float 2", &unit->m_Float2);
					ImGui::InputFloat("Float 3", &unit->m_Float3);
					ImGui::InputInt("No", &unit->m_No);
					ImGui::InputFloat("Easy Power Scale", &unit->m_EasyPowerScale);
					ImGui::InputFloat("Hard Power Scale", &unit->m_HardPowerScale);
					ImGui::InputFloat("Very Hard Power Scale", &unit->m_VeryhardPowerScale);
					ImGui::InputFloat("Revengeance Power Scale", &unit->m_RevengeancePowerScale);
					ImGui::InputInt("Int 3", &unit->m_Int3);
					ImGui::InputInt("Int 4", &unit->m_Int4);
					ImGui::InputInt("Int 5", &unit->m_Int5);
					if (ImGui::Button("Remove"))
						FileTools::RemoveUnit(unit->m_id);
					ImGui::TreePop();
				}
			}
		}

		ImGui::End();
	}

	#ifdef _WINDLL
	if (GetAsyncKeyState(VK_INSERT) & 1)
		bDraw = !bDraw;
	#endif
}

#pragma warning(pop)