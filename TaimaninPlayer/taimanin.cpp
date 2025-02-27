

#include "taimanin.h"

#include "win_filesystem.h"
#include "win_text.h"
#include "text_utility.h"
#include "json_minimal.h"

namespace taimanin
{
	struct SResourcePath
	{
		std::wstring wstrStillFolderPath;
		std::wstring wstrVoiceFolderPath;
		std::wstring wstrPairListFilePath;
	};

	/*各素材経路導出*/
	static bool DeriveResourcePathFromScriptFilePath(const std::wstring& wstrScriptFilePath, SResourcePath& resourcePath)
	{
		const wchar_t g_swzScenarioFolderName[] = L"scenario_r18";
		size_t nPos = wstrScriptFilePath.find(g_swzScenarioFolderName);
		if (nPos == std::wstring::npos)return false;

		std::wstring wstrBaseFolder = wstrScriptFilePath.substr(0, nPos);

		resourcePath.wstrStillFolderPath = wstrBaseFolder + L"ev_r18\\";
		resourcePath.wstrVoiceFolderPath = wstrBaseFolder + L"voice_r18\\";
		resourcePath.wstrPairListFilePath = wstrBaseFolder + L"json_r18\\" + wstrScriptFilePath.substr(nPos + sizeof(g_swzScenarioFolderName) / sizeof(wchar_t));
		
		return true;
	}

	template <typename CharType>
	static void SplitToCommands(const std::basic_string<CharType>& str, std::vector<std::basic_string<CharType>>& commands)
	{
		for (size_t nRead = 0; nRead < str.size();)
		{
			size_t nPos1 = str.find(CharType('<'), nRead);
			nPos1 = nPos1 == std::basic_string<CharType>::npos ? nRead : nPos1;

			size_t nPos2 = str.find(CharType('>'), nPos1);
			nPos2 = nPos2 == std::basic_string<CharType>::npos ? str.size() : nPos2 + 1;

			commands.push_back(str.substr(nPos1, nPos2 - nPos1));
			nRead = nPos2;
		}
	}

	static void LoadPairListFile(const std::wstring& wstrFilePath, std::vector<std::pair<std::wstring, std::wstring>>& pairList)
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
			bool bRet = json_minimal::GetJsonElementValue(&list[0], "parent", vBuffer.data(), vBuffer.size());
			if (!bRet)continue;

			std::wstring wstrParent = win_text::WidenUtf8(vBuffer.data());

			bRet = json_minimal::GetJsonElementValue(&list[0], "child", vBuffer.data(), vBuffer.size());
			if (!bRet)continue;

			std::wstring wstrChild = win_text::WidenUtf8(vBuffer.data());

			pairList.emplace_back(std::make_pair(wstrParent, wstrChild));
		}
	}

	static long long FindChildIndex(std::vector<std::pair<std::wstring, std::wstring>>& pairList, const std::wstring& wstrFileName)
	{
		const auto& iter = std::find_if
		(
			pairList.begin(), pairList.end(),
			[&wstrFileName](const std::pair<std::wstring, std::wstring>& pair)
			{
				return pair.second == wstrFileName;
			}
		);

		if (iter != pairList.cend())
		{
			return std::distance(pairList.begin(), iter);
		}

		return -1;
	}

} /* namespace taimanin */


/*台本経路一覧取得*/
void taimanin::CreateScriptFilePathList(const std::wstring& wstrFilePath, std::vector<std::wstring>& filePaths)
{
	/*
	* 下記のような構造から.txt経路一覧を作成する。
	* scenario_r18
	* |- ...
	* |- chr_0404_2_r18
	* |  |_ chr_0404_2_r18.txt
	* |- chr_0405_1_r18
	* |  |_ chr_0405_1_r18.txt
	* |_ ...
	*/
	size_t nPos = wstrFilePath.size();
	for (size_t i = 0; i < 2; ++i)
	{
		nPos = wstrFilePath.find_last_of(L"\\/", nPos);
		if (nPos == std::wstring::npos)return;
		--nPos;
	}

	std::wstring wstrParent = wstrFilePath.substr(0, nPos + 1);

	std::vector<std::wstring> folders;
	win_filesystem::CreateFilePathList(wstrParent.c_str(), nullptr, folders);

	folders.erase(std::remove_if(folders.begin(), folders.end(),
		[](const std::wstring& wstr)
		-> bool
		{
			return wstr.find(L"chr_0001_1_p_r18") != std::wstring::npos ||
				wstr.find(L"es019_s01a") != std::wstring::npos;
		}), folders.end());

	for (const auto& folder : folders)
	{
		win_filesystem::CreateFilePathList(folder.c_str(), L".txt", filePaths);
	}
}

/*台本読み取り*/
bool taimanin::LoadScenario(const std::wstring& wstrScenarioFilePath, std::vector<adv::TextDatum>& textData, std::vector<std::vector<std::wstring>>& imageFilePathsList, std::vector<adv::SceneDatum>& sceneData)
{
	SResourcePath resourcePath;
	bool bRet = DeriveResourcePathFromScriptFilePath(wstrScenarioFilePath, resourcePath);
	if (!bRet)return false;

	std::vector<std::pair<std::wstring, std::wstring>> pairList;
	LoadPairListFile(resourcePath.wstrPairListFilePath, pairList);
	if (pairList.empty())return false;

	std::wstring wstrScenerioFile = win_text::WidenUtf8(win_filesystem::LoadFileAsString(wstrScenarioFilePath.c_str()));
	if (wstrScenerioFile.empty())return false;

	text_utility::ReplaceAll(wstrScenerioFile, L"　", L"");
	text_utility::ReplaceAll(wstrScenerioFile, L" ", L"");

	std::vector<std::wstring> lines;
	text_utility::TextToLines(wstrScenerioFile, lines);

	std::vector<std::vector<std::wstring>> scriptCommands;
	for (const auto& line : lines)
	{
		if (line.size() > 1 && line[0] == L'/' && line[1] == L'/')continue;

		std::vector<std::wstring> commandBuffer;
		SplitToCommands(line, commandBuffer);
		scriptCommands.push_back(commandBuffer);
	}

	std::wstring textBuffer;
	std::wstring voiceFileNameBuffer;

	adv::SceneDatum sceneDatumBuffer;

	for (const auto& command : scriptCommands)
	{
		if (command.empty())continue;

		const std::wstring& commandToken = command[0];
		if (commandToken.empty())continue;

		if (commandToken[0] != L'<')
		{
			/*語り・台詞内容*/
			textBuffer += command[0];
			textBuffer += '\n';
		}
		else if (commandToken == L"<NAME_PLATE>")
		{
			/*語り・台詞開始*/
			if (command.size() > 1)
			{
				textBuffer += command[1];
				textBuffer += L": ";
			}
		}
		else if (commandToken == L"<VOICE_PLAY>")
		{
			/*音声ファイル名*/
			if (command.size() > 1)
			{
				voiceFileNameBuffer = command[1];
			}
		}
		else if (commandToken == L"<EV>")
		{
			/*画像名*/
			if (command.size() > 1)
			{
				std::vector<std::wstring> params;
				text_utility::SplitTextBySeparator(command[1], L',', params);
				if (!params.empty())
				{
					std::vector<std::wstring> imageFilePaths;

					const auto ToAbsolutePath = [&resourcePath](const std::wstring& wstrFileName)
						-> std::wstring
						{
							return resourcePath.wstrStillFolderPath + wstrFileName + L"\\" + wstrFileName + L".png";
						};

					long long llIndex = FindChildIndex(pairList, params[0]);
					if (llIndex == -1)
					{
						imageFilePaths.emplace_back(ToAbsolutePath(params[0]));
					}
					else
					{
						imageFilePaths.emplace_back(ToAbsolutePath(pairList[llIndex].first));
						imageFilePaths.emplace_back(ToAbsolutePath(pairList[llIndex].second));
					}

					imageFilePathsList.push_back(std::move(imageFilePaths));

					sceneDatumBuffer.nImageIndex = imageFilePathsList.size() - 1;
				}
			}
		}
		else if (commandToken == L"<PAUSE>")
		{
			/*区切り*/
			if (!textBuffer.empty())
			{
				adv::TextDatum t;
				t.wstrText = textBuffer;

				if (!voiceFileNameBuffer.empty())
				{
					t.wstrVoicePath = resourcePath.wstrVoiceFolderPath + voiceFileNameBuffer + L".ogg";

					voiceFileNameBuffer.clear();
				}

				textData.push_back(std::move(t));
				textBuffer.clear();

				sceneDatumBuffer.nTextIndex = textData.size() - 1;
				sceneData.push_back(sceneDatumBuffer);
			}
		}
	}

	return true;
}
