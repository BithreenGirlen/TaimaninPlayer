#ifndef TAIMANIN_SCENE_CRAFTER_H_
#define TAIMANIN_SCENE_CRAFTER_H_

#include <Windows.h>
#include <d2d1_1.h>
#include <atlbase.h>

#include <string>
#include <vector>
#include <unordered_map>

#include "adv.h"

class CTaimaninSceneCrafter
{
public:
	CTaimaninSceneCrafter(ID2D1DeviceContext* pD2d1DeviceContext);
	~CTaimaninSceneCrafter();

	bool LoadScenario(const wchar_t* pwzScenarioFilePath);

	void GetImageSize(unsigned int* uiWidth, unsigned int* uiHeight);

	void ShiftScene(bool bForward);
	bool HasReachedLastScene();

	std::vector<ID2D1Bitmap*> GetCurrentImages();
	std::wstring GetCurrentFormattedText();
	const wchar_t* GetCurrentVoiceFilePath();
private:
	ID2D1DeviceContext* m_pStoredD2d1DeviceContext = nullptr;

	std::vector<adv::TextDatum> m_textData;

	std::vector<adv::SceneDatum> m_sceneData;
	size_t m_nSceneIndex = 0;

	std::unordered_map<std::wstring, CComPtr<ID2D1Bitmap>> m_imageMap;
	std::vector<std::vector<ID2D1Bitmap*>> m_imagesList;

	void ClearScenarioData();
	ID2D1Bitmap* ImportImage(const std::wstring& wstrImageFilePath, unsigned int uiCroppedWidth = 0);
};
#endif // !TAIMANIN_SCENE_CRAFTER_H_
