#ifndef TAIMANIN_H_
#define TAIMANIN_H_

#include <string>
#include <vector>

#include "adv.h"

namespace taimanin
{
	void CreateScriptFilePathList(const std::wstring& wstrFilePath, std::vector<std::wstring>& filePaths);

	bool LoadScenario(
		const std::wstring& wstrScenarioFilePath,
		std::vector<adv::TextDatum>& textData,
		std::vector<std::vector<std::wstring>> &imageFilePathsList,
		std::vector<adv::SceneDatum>& sceneData
	);
}

#endif // !TAIMANIN_H_
