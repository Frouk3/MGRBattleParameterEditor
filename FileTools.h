#pragma once

#include <Windows.h>
#include <stdio.h>
#include <vector>
#include "BP.h"

namespace FileTools
{
	inline wchar_t LastPath[MAX_PATH] = {0};
	
	inline char* FileBuffer = nullptr;
	inline size_t FileBufferSize = 0;
	inline FILE *File = nullptr;

	inline std::vector<BattleParameter::Unit *> Units;

	void OpenFile(const wchar_t* fileName);
	void Cleanup();
	void SaveFile(const wchar_t* fileName);

	void ReadUnits();
	void AppendUnit(BattleParameter::Unit *unit);
	void RemoveUnit(int index);

	void SaveUnits();
}