

#include "taimanin.h"

#include "win_filesystem.h"
#include "win_text.h"
#include "text_utility.h"
#include "json_minimal.h"

namespace taimanin
{
	constexpr wchar_t g_swzScenarioFolderName[] = L"scenario_r18";

	std::wstring DeriveStillFolderPathFromScriptFilePath(const std::wstring& wstrFilePath)
	{
		size_t nPos = wstrFilePath.find(g_swzScenarioFolderName);
		if (nPos == std::wstring::npos)return std::wstring();

		return wstrFilePath.substr(0, nPos) + L"ev_r18";
	}

	std::wstring DeriveVoiceFolderPathFromScriptFilePath(const std::wstring& wstrFilePath)
	{
		size_t nPos = wstrFilePath.find(g_swzScenarioFolderName);
		if (nPos == std::wstring::npos)return std::wstring();

		return wstrFilePath.substr(0, nPos) + L"voice_r18";
	}

	std::wstring DerivePairListPathFromScriptPath(const std::wstring& wstrFilePath)
	{

		size_t nPos = wstrFilePath.find(g_swzScenarioFolderName);
		if (nPos == std::wstring::npos)return std::wstring();

		return wstrFilePath.substr(0, nPos) + L"json_r18\\" + wstrFilePath.substr(nPos + sizeof(g_swzScenarioFolderName)/sizeof(wchar_t)) ;
	}

	void CreateScriptFilePathList(const std::wstring& wstrFilePath, std::vector<std::wstring>& filePaths)
	{
		std::wstring wstrParent = text_utility::ExtractDirectory(wstrFilePath, 2);
		if (wstrFilePath.empty())return;

		std::vector<std::wstring> folders;
		win_filesystem::CreateFilePathList(wstrParent.c_str(), nullptr, folders);
		for (const auto& folder : folders)
		{
			if (folder.find(L"chr_0001_1_p_r18") != std::wstring::npos)continue;
			win_filesystem::CreateFilePathList(folder.c_str(), L".txt", filePaths);
		}
	}

	void LoadPairListFile(const std::wstring& wstrFilePath, std::vector<std::vector<adv::ImageFileDatum>> &pairLists)
	{
		std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
		if (strFile.empty())return;

		char* p = &strFile[0];
		char* p2 = nullptr;
		json_minimal::ExtractJsonArray(&p, "pairList", &p2);
		if (p2 == nullptr)return;

		p = p2;
		std::vector<std::string> lists;
		for (;;)
		{
			char* p3 = nullptr;
			json_minimal::ExtractJsonObject(&p, nullptr, &p3);
			if (p3 == nullptr)break;
			lists.push_back(p3);
			free(p3);
		}
		free(p2);

		std::vector<char> vBuffer(512, '\0');
		for (auto& list : lists)
		{
			std::vector<adv::ImageFileDatum> pairList;
			adv::ImageFileDatum imageDatum;
			bool bRet = json_minimal::GetJsonElementValue(&list[0], "parent", vBuffer.data(), vBuffer.size());
			if (!bRet)continue;
			imageDatum.iLayer = 0;
			imageDatum.wstrFilePath = win_text::WidenUtf8(vBuffer.data());
			pairList.push_back(imageDatum);

			bRet = json_minimal::GetJsonElementValue(&list[0], "child", vBuffer.data(), vBuffer.size());
			if (!bRet)continue;

			imageDatum.iLayer = 1;
			imageDatum.wstrFilePath = win_text::WidenUtf8(vBuffer.data());
			pairList.push_back(imageDatum);

			pairLists.push_back(pairList);
		}

	}
}

bool taimanin::LoadScenario(const std::wstring& wstrFilePath, std::vector<adv::TextDatum>& textData, std::vector<std::vector<adv::ImageFileDatum>>& layerData)
{
	std::wstring wstrStillBaseFolderPath = DeriveStillFolderPathFromScriptFilePath(wstrFilePath);
	std::wstring wstrVoiceBaseFolderPath = DeriveVoiceFolderPathFromScriptFilePath(wstrFilePath);
	std::wstring wstrPairListFilePath = DerivePairListPathFromScriptPath(wstrFilePath);
	if (wstrStillBaseFolderPath.empty() || wstrVoiceBaseFolderPath.empty() || wstrPairListFilePath.empty())return false;

	std::string strFile = win_filesystem::LoadFileAsString(wstrFilePath.c_str());
	if (strFile.empty())return false;

	std::vector<std::string> lines;
	text_utility::TextToLines(strFile, lines);

	const auto SplitToCommands = [](const std::string& str, std::vector<std::string>& commands)
		{
			for (size_t nRead = 0; nRead < str.size();)
			{
				size_t nPos1 = str.find('<', nRead);
				nPos1 = nPos1 == std::string::npos ? nRead : nPos1;

				size_t nPos2 = str.find('>', nPos1);
				nPos2 = nPos2 == std::string::npos ? str.size() : nPos2 + 1;

				commands.push_back(str.substr(nPos1, nPos2 - nPos1));
				nRead = nPos2;
			}
		};

	std::vector<std::vector<std::string>> scriptCommands;
	for (const auto& line : lines)
	{
		if (line.size() > 1 && line[0] == '/' && line[1] == '/')continue;

		std::vector<std::string> commandBuffer;
		SplitToCommands(line, commandBuffer);
		scriptCommands.push_back(commandBuffer);
	}

	std::vector<std::string> evs;

	std::string voiceBuffer;
	std::string textBuffer;

	bool bOnReadingText = false;
	for (const auto& command : scriptCommands)
	{
		if (command.size() == 1)
		{
			if (command[0] == "<NAME_PLATE>")
			{
				/*語り開始*/
				bOnReadingText = true;
			}
			else if (command[0].front() != '<')
			{
				/*語り・台詞内容*/
				textBuffer += command[0];
				textBuffer += '\n';
			}
			else if (command[0] == "<PAUSE>" && !textBuffer.empty())
			{
				adv::TextDatum t;
				t.wstrText = win_text::WidenUtf8(textBuffer);
				text_utility::ReplaceAll(t.wstrText, L"　", L"");
				if (!voiceBuffer.empty())
				{
					t.wstrVoicePath = wstrVoiceBaseFolderPath + L"\\" + win_text::WidenUtf8(voiceBuffer) + L".ogg";
				}
				textData.push_back(t);
				textBuffer.clear();
				voiceBuffer.clear();
				bOnReadingText = false;
			}

		}
		else if (command.size() > 1)
		{
			if (command[0] == "<NAME_PLATE>")
			{
				/*台詞開始*/
				bOnReadingText = true;
				textBuffer += command[1];
				textBuffer += ": ";
			}
			else if (command[0] == "<VOICE_PLAY>")
			{
				voiceBuffer = command[1];
			}
			else if (command[0] == "<EV>")
			{
				std::vector<std::string> splits;
				text_utility::SplitTextBySeparator(command[1], ',', splits);
				if (!splits.empty())
				{
					evs.push_back(splits[0]);
				}
			}
			else
			{
				bOnReadingText = false;
			}
		}
	}

	LoadPairListFile(wstrPairListFilePath, layerData);

	for (auto& pairList : layerData)
	{
		for (auto& name : pairList)
		{
			name.wstrFilePath = wstrStillBaseFolderPath + L"\\" + name.wstrFilePath + L"\\" + name.wstrFilePath + L".png";
		}
	}

	return !textData.empty() && !layerData.empty();
}
