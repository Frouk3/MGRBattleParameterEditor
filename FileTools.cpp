#include "FileTools.h"

#pragma warning(push)
#pragma warning(disable : 4996)

void FileTools::OpenFile(const wchar_t* fileName)
{
	File = _wfopen(fileName, L"rb");
	if (!File)
		return;

	lstrcpyW(LastPath, fileName);

	fseek(File, 0, SEEK_END);
	FileBufferSize = ftell(File);
	fseek(File, 0, SEEK_SET);

	FileBuffer = (char*)calloc(1, FileBufferSize + 32u); // 32 as reserved

	if (!FileBuffer)
	{
		fclose(File);
		File = nullptr;
		return;
	}

	fread(FileBuffer, 1, FileBufferSize, File);

	fclose(File);
	File = nullptr;
}

void FileTools::OpenFileMem(const char* pData, size_t dataSize)
{
	if (!pData || dataSize == 0)
		return;

	if (FileBuffer)
	{
		free(FileBuffer);
		FileBuffer = nullptr;
	}

	FileBufferSize = dataSize;
	FileBuffer = (char*)calloc(1, FileBufferSize + 32u); // 32 as reserved
	if (!FileBuffer)
		return;

	memcpy(FileBuffer, pData, dataSize);
}

void FileTools::Cleanup()
{
	for (auto& unit : Units)
	{
		if (unit)
		{
			delete unit;
			unit = nullptr;
		}
	}

	Units.clear();

	if (FileBuffer)
	{
		free(FileBuffer);
		FileBuffer = nullptr;
	}
	if (File)
	{
		fclose(File);
		File = nullptr;
	}
}

void FileTools::SaveFile(const wchar_t* fileName)
{
	if (!fileName || !*fileName)
		fileName = LastPath;

	if (!FileBuffer)
		return;

	File = _wfopen(fileName, L"wb");

	if (!File)
		return;

	fwrite(FileBuffer, 1, FileBufferSize, File);
	fclose(File);
	File = nullptr;
}

void FileTools::ReadUnits()
{
	BattleParameter::File* file = (BattleParameter::File*)FileBuffer;

	if (!file || file->m_unitCount <= 0)
		return;

	for (auto& unit : Units)
	{
		if (unit)
		{
			delete unit;
			unit = nullptr;
		}
	}

	Units.clear();

	for (int i = 0; i < file->m_unitCount; ++i)
	{
		BattleParameter::Unit* unit = new BattleParameter::Unit;
		if (unit)
		{
			memcpy(unit, &file->m_units[i], sizeof(BattleParameter::Unit));
			Units.push_back(unit);
		}
	}
}

void FileTools::AppendUnit(BattleParameter::Unit* unit)
{
	if (!unit)
		return;

	Units.push_back(unit);
}

void FileTools::RemoveUnit(int index)
{
	for (auto& unit : Units)
	{
		if (unit->m_id == index)
		{
			delete unit;
			unit = nullptr;
			break;
		}
	}
}

void FileTools::SaveUnits()
{
	size_t newFileSize = 4 + Units.size() * sizeof(BattleParameter::Unit);

	BattleParameter::File* file = (BattleParameter::File*)calloc(1, newFileSize + 32u);

	if (!file)
		return;

	file->m_unitCount = Units.size();
	for (size_t i = 0; i < Units.size(); ++i)
		memcpy(&file->m_units[i], Units[i], sizeof(BattleParameter::Unit));

	if (FileBuffer)
	{
		free(FileBuffer);
		FileBuffer = nullptr;
	}

	FileBuffer = (char*)file;
}

#pragma warning(pop)