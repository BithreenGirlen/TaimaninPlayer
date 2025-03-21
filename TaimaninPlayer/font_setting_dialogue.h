#ifndef FONT_SETTING_DIALOGUE_H_
#define FONT_SETTING_DIALOGUE_H_

#include <Windows.h>

#include <string>
#include <vector>

#include "dialogue_controls.h"

class CFontSettingDialogue
{
public:
	CFontSettingDialogue();
	~CFontSettingDialogue();

	INT_PTR Open(HINSTANCE hInstance, HWND hWndParent, const wchar_t* pwzWindowName, void *pTextWriter);

	HWND GetHwnd()const { return m_hWnd; }

	struct SFontDatum
	{
		std::wstring wstrLocaleName;
		std::wstring wstrFontFamilyName;
		std::wstring wstrFontFilePath;

		float fFontSize = 24.f;
		float fThickness = 3.2f;

		bool bBold = true;
		bool bItalic = false;
	};
	const SFontDatum& GetFontDatum()const { return m_fontDatum; }

	bool HasFontBeenChanged() const { return m_bFontHasBeenChanged; }
private:
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnInit(HWND hWnd);
	LRESULT OnClose(WPARAM wParam);
	LRESULT OnSize();
	LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnVScroll(WPARAM wParam, LPARAM lParam);

	enum Constants { kFontSize = 16 };
	enum Controls
	{
		kApplyButton = 1,
		kFontSizeSlider, kFontThicknessSlider,
		kBoldCheckButton, kItalicCheckButton
	};

	HFONT m_hFont = nullptr;

	static BOOL CALLBACK SetFontCallback(HWND hWnd, LPARAM lParam);

	CStatic m_fontNameStatic;
	CComboBox m_fontNameComboBox;

	CStatic m_fontSizeStatic;
	CSlider m_fontSizeSlider;

	CStatic m_fontThicknessStatic;
	CFloatSlider m_fontThicknessSlider;

	CButton m_boldCheckButton;
	CButton m_italicCheckButton;

	CButton m_applyButton;

	SFontDatum m_fontDatum;

	bool m_bFontHasBeenChanged = false;

	void ResizeControls();

	void OnApplyButton();
	void OnBoldCheckButton();
	void OnOtalicCheckButton();

	void SetSliderPosition();

	void* m_pTextWriter = nullptr;
};
#endif // !FONT_SETTING_DIALOGUE_H_
