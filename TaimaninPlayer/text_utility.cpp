

#include "text_utility.h"

namespace text_utility
{
	void TextToLines(const std::string& strText, std::vector<std::string>& lines)
	{
		std::string strTemp;
		for (size_t nRead = 0; nRead < strText.size(); ++nRead)
		{
			if (strText.at(nRead) == '\r' || strText.at(nRead) == '\n')
			{
				if (!strTemp.empty())
				{
					lines.push_back(strTemp);
					strTemp.clear();
				}
				continue;
			}

			strTemp.push_back(strText.at(nRead));
		}

		if (!strTemp.empty())
		{
			lines.push_back(strTemp);
			strTemp.clear();
		}
	}

	void TextToLines(const std::wstring& wstrText, std::vector<std::wstring>& lines)
	{
		std::wstring wstrTemp;
		for (size_t nRead = 0; nRead < wstrText.size(); ++nRead)
		{
			if (wstrText.at(nRead) == L'\r' || wstrText.at(nRead) == L'\n')
			{
				if (!wstrTemp.empty())
				{
					lines.push_back(wstrTemp);
					wstrTemp.clear();
				}
				continue;
			}

			wstrTemp.push_back(wstrText.at(nRead));
		}

		if (!wstrTemp.empty())
		{
			lines.push_back(wstrTemp);
			wstrTemp.clear();
		}
	}

	void SplitTextBySeparator(const std::wstring& wstrText, const wchar_t cSeparator, std::vector<std::wstring>& splits)
	{
		for (size_t nRead = 0; nRead < wstrText.size();)
		{
			const wchar_t* p = wcschr(&wstrText[nRead], cSeparator);
			if (p == nullptr)
			{
				size_t nLen = wstrText.size() - nRead;
				splits.emplace_back(wstrText.substr(nRead, nLen));
				break;
			}

			size_t nLen = p - &wstrText[nRead];
			splits.emplace_back(wstrText.substr(nRead, nLen));
			nRead += nLen + 1;
		}
	}

	void SplitTextBySeparator(const std::string& strText, const char cSeparator, std::vector<std::string>& splits)
	{
		for (size_t nRead = 0; nRead < strText.size();)
		{
			const char* p = strchr(&strText[nRead], cSeparator);
			if (p == nullptr)
			{
				size_t nLen = strText.size() - nRead;
				splits.emplace_back(strText.substr(nRead, nLen));
				break;
			}

			size_t nLen = p - &strText[nRead];
			splits.emplace_back(strText.substr(nRead, nLen));
			nRead += nLen + 1;
		}
	}

	void ReplaceAll(std::wstring& src, const std::wstring& strOld, const std::wstring& strNew)
	{
		if (strOld == strNew)return;

		for (size_t nRead = 0;;)
		{
			size_t nPos = src.find(strOld, nRead);
			if (nPos == std::wstring::npos)break;
			src.replace(nPos, strOld.size(), strNew);
			nRead = nPos + strNew.size();
		}
	}

	void ReplaceAll(std::string& strText, const std::string& strOld, const std::string& strNew)
	{
		if (strOld == strNew)return;

		for (size_t nRead = 0;;)
		{
			size_t nPos = strText.find(strOld, nRead);
			if (nPos == std::string::npos)break;
			strText.replace(nPos, strOld.size(), strNew);
			nRead = nPos + strNew.size();
		}
	}

	void EliminateTag(std::wstring& wstr)
	{
		std::wstring wstrResult;
		wstrResult.reserve(wstr.size());
		int iCount = 0;
		for (const auto& c : wstr)
		{
			if (c == L'<')
			{
				++iCount;
				continue;
			}
			else if (c == L'>')
			{
				--iCount;
				continue;
			}

			if (iCount == 0)
			{
				wstrResult.push_back(c);
			}
		}
		wstr = wstrResult;
	}

	std::wstring ExtractDirectory(const std::wstring& wstrFilePath, size_t nLevel)
	{
		if (nLevel == 0)return wstrFilePath;

		size_t nPos = wstrFilePath.size();
		for (size_t i = 0; i < nLevel; ++i)
		{
			nPos = wstrFilePath.find_last_of(L"\\/", nPos);
			if (nPos == std::wstring::npos)break;
			--nPos;
		}

		return wstrFilePath.substr(0, nPos + 1);
	}

	std::wstring TruncateFilePath(const std::wstring& strRelativePath)
	{
		size_t nPos = strRelativePath.rfind(L'/');
		if (nPos != std::wstring::npos)
		{
			return strRelativePath.substr(nPos + 1);
		}
		return strRelativePath;
	}

	std::string GetExtensionFromFileName(const std::string &strFileName)
	{
		size_t nPos = strFileName.rfind('/');
		nPos = nPos != std::string::npos ? nPos + 1 : 0;
		nPos = strFileName.find('.', nPos);
		if (nPos != std::string::npos)
		{
			return strFileName.substr(nPos);
		}
		return std::string();
	}
}
