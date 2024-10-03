#ifndef TEXT_UTILITY_H_
#define TEXT_UTILITY_H_

#include <string>
#include <vector>

namespace text_utility
{
	void TextToLines(const std::string& strText, std::vector<std::string>& lines);
	void SplitTextBySeparator(const std::string& strText, const char cSeparator, std::vector<std::string>& splits);

	void ReplaceAll(std::wstring& wstrText, const std::wstring& wstrOld, const std::wstring& wstrNew);

	std::wstring ExtractDirectory(const std::wstring& wstrFilePath, size_t nLevel = 1);
}

#endif // !TEXT_UTILITY_H_
