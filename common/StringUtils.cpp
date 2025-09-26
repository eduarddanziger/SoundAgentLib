#include "os-dependencies.h"

#include "StringUtils.h"

#include <boost/locale.hpp>


std::string ed::Utf16ToUtf8(const std::wstring& str)
{
    return boost::locale::conv::utf_to_utf<char>(str);
}
