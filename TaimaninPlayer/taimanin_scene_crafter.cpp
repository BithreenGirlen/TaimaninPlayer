

#include "taimanin_scene_crafter.h"

#include "taimanin.h"
#include "win_image.h"

CTaimaninSceneCrafter::CTaimaninSceneCrafter(ID2D1DeviceContext* pD2d1DeviceContext)
	: m_pStoredD2d1DeviceContext(pD2d1DeviceContext)
{

}

CTaimaninSceneCrafter::~CTaimaninSceneCrafter()
{

}
/*台本読み込み*/
bool CTaimaninSceneCrafter::LoadScenario(const wchar_t* pwzScenarioFilePath)
{
	if (pwzScenarioFilePath == nullptr)return false;
	ClearScenarioData();

	std::vector<std::vector<std::wstring>> imageFilePathsList;
	taimanin::LoadScenario(pwzScenarioFilePath, m_textData, imageFilePathsList, m_sceneData);

	for (const auto& imageFilePaths : imageFilePathsList)
	{
		std::vector<ID2D1Bitmap*> images;
		for (const auto& imageFilePath : imageFilePaths)
		{
			ID2D1Bitmap* pD2D1Bitmap = nullptr;

			const auto& iter = m_imageMap.find(imageFilePath);
			if (iter == m_imageMap.cend())
			{
				pD2D1Bitmap = ImportWholeImage(imageFilePath);
			}
			else
			{
				pD2D1Bitmap = iter->second.p;
			}

			if (pD2D1Bitmap != nullptr)
			{
				images.push_back(pD2D1Bitmap);
			}
		}

		if (!images.empty())
		{
			m_imagesList.push_back(images);
		}
	}

	return !m_imagesList.empty();
}
/*画像寸法取得*/
void CTaimaninSceneCrafter::GetImageSize(unsigned int* uiWidth, unsigned int* uiHeight)
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		size_t nImageIndex = m_sceneData[m_nSceneIndex].nImageIndex;
		if (nImageIndex < m_imagesList.size() && !m_imagesList[nImageIndex].empty())
		{
			D2D1_SIZE_U s = m_imagesList[nImageIndex][0]->GetPixelSize();
			*uiWidth = s.width;
			*uiHeight = s.height;
		}
	}
}
/*場面移行*/
void CTaimaninSceneCrafter::ShiftScene(bool bForward)
{
	if (m_sceneData.empty())return;

	if (bForward)
	{
		if (++m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = 0;
		}
	}
	else
	{
		if (--m_nSceneIndex >= m_sceneData.size())
		{
			m_nSceneIndex = m_sceneData.size() - 1;
		}
	}
}
/*最終場面是否*/
bool CTaimaninSceneCrafter::HasReachedLastScene()
{
	return m_nSceneIndex == m_sceneData.size() - 1;
}
/*現在の画像受け渡し*/
std::vector<ID2D1Bitmap*> CTaimaninSceneCrafter::GetCurrentImages()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		size_t nImageIndex = m_sceneData[m_nSceneIndex].nImageIndex;
		if (nImageIndex < m_imagesList.size())
		{
			return m_imagesList[nImageIndex];
		}
	}

	return std::vector<ID2D1Bitmap*>();
}
/*文章生成*/
std::wstring CTaimaninSceneCrafter::GetCurrentText()
{
	std::wstring wstr;
	if (m_nSceneIndex < m_sceneData.size())
	{
		wstr.reserve(128);
		size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;

		if (nTextIndex < m_textData.size())
		{
			wstr = m_textData[nTextIndex].wstrText;
			if (!wstr.empty() && wstr.back() != L'\n')wstr.push_back(L'\n');
			wstr += std::to_wstring(nTextIndex + 1) + L"/" + std::to_wstring(m_textData.size());
		}
	}

	return wstr;
}
/*現在の音声ファイル経路受け渡し*/
std::wstring CTaimaninSceneCrafter::GetCurrentVoiceFilePath()
{
	if (m_nSceneIndex < m_sceneData.size())
	{
		size_t nTextIndex = m_sceneData[m_nSceneIndex].nTextIndex;
		if (nTextIndex < m_textData.size())
		{
			return m_textData[nTextIndex].wstrVoicePath;
		}
	}

	return std::wstring();
}
/*消去*/
void CTaimaninSceneCrafter::ClearScenarioData()
{
	m_textData.clear();

	m_sceneData.clear();
	m_nSceneIndex = 0;

	m_imagesList.clear();
	m_imageMap.clear();
}
/*画像取り込み*/
ID2D1Bitmap* CTaimaninSceneCrafter::ImportWholeImage(const std::wstring& wstrImageFilePath)
{
	ID2D1Bitmap* p = nullptr;

	SImageFrame sImageFrame{};
	bool bRet = win_image::LoadImageToMemory(wstrImageFilePath.c_str(), &sImageFrame);
	if (bRet)
	{
		CComPtr<ID2D1Bitmap> pD2d1Bitmap;

		HRESULT hr = m_pStoredD2d1DeviceContext->CreateBitmap(
			D2D1::SizeU(sImageFrame.uiWidth, sImageFrame.uiHeight),
			D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
			&pD2d1Bitmap);

		D2D1_RECT_U rc = { 0, 0, sImageFrame.uiWidth, sImageFrame.uiHeight };
		hr = pD2d1Bitmap->CopyFromMemory(&rc, sImageFrame.pixels.data(), sImageFrame.iStride);
		if (SUCCEEDED(hr))
		{
			m_imageMap.insert({ wstrImageFilePath, pD2d1Bitmap });
			p = pD2d1Bitmap.p;
		}
	}

	return p;
}
