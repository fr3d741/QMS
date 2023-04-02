#include <Utility/GeneralUtilities.h>

#include <algorithm>
#include <locale>
#include <regex>
#include <array>
#include <codecvt>
#include <limits>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

//void
//GeneralUtilities::RightTrim(string& str) {

//    for (int i = static_cast<int>(str.size()) - 1; 0 <= i; --i)
//        if (str[i] == ' ')
//            str.erase(i);
//        else
//            break;
//}

//void
//GeneralUtilities::RightTrim(std::string& str) {

//    for (int i = static_cast<int>(str.size()) - 1; 0 <= i; --i)
//        if (str[i] == ' ')
//            str.erase(i);
//        else
//            break;
//}

//void
//GeneralUtilities::LeftTrim(string& str) {

//    for (auto it = str.begin(); it != str.end();)
//        if (*it == ' ')
//            str.erase(it);
//        else
//            break;
//}

//void
//GeneralUtilities::LeftTrim(std::string& str) {

//    for (auto it = str.begin(); it != str.end();)
//        if (*it == ' ')
//            str.erase(it);
//        else
//            break;
//}

//bool
//GeneralUtilities::AreEqualCaseInsensitive(std::string str1, std::string str2) {
//    return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end(), [](char a, char b) { return ::tolower(a) == ::tolower(b); });
//}

//bool
//GeneralUtilities::AreEqualCaseInsensitive(string str1, string str2) {
//    return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end(), [](wchar_t a, wchar_t b) { return ::tolower(a) == ::tolower(b); });
//}

//string
//GeneralUtilities::Convert(const std::string& str) {

//    //std::setlocale(LC_ALL, "");
//    //std::locale::global(std::locale(""));
//    //std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> converter;
//    //return converter.from_bytes(str);

//    std::wstring_convert<std::codecvt_utf8<char_t>, char_t> conv_utf8_utf32;
//    string unicode_codepoints = conv_utf8_utf32.from_bytes(str);
//    return unicode_codepoints;
//}

//std::string
//GeneralUtilities::Convert(const string& str) {

//    //std::setlocale(LC_ALL, "");
//    //std::locale::global(std::locale(""));
//    //std::vector<char> char_array(str.size() * 2);

//    //auto res = std::wcstombs(char_array.data(), str.c_str(), str.size());
//    //if (res == static_cast<std::size_t>(-1))
//    //    res = 0;
//    //return std::string(char_array.data(), res);
//    std::wstring_convert<std::codecvt_utf8<char_t>, char_t> conv_utf8_utf32;
//    std::string unicode_codepoints = conv_utf8_utf32.to_bytes(str);
//    return unicode_codepoints;
//}

//bool
//GeneralUtilities::Contains(const string& in, const string& what) {

//    for (auto it = what.begin(), it2 = in.begin(); it2 != in.end(); ++it2) {

//        if (it == what.end())
//            return true;

//        if (*it != *it2)
//            it = what.begin();
//        else
//            ++it;
//    }

//    return false;
//}

bool
GeneralUtilities::GetYear(const QString& title, int& year){
    static QRegularExpression regex("[(][0-9]{4}[)]");

    auto match = regex.match(title);
    if (match.hasMatch() == false){
        return false;
    }

    auto intermediate = match.captured();
    year = ConvertToYear(intermediate);
    return true;
}

int 
GeneralUtilities::ConvertToYear(const QString& str) {

    const QRegularExpression regex("[0-9]{4}");

    auto match = regex.match(str);
    if (match.hasMatch() == false)
    {
        return std::numeric_limits<int>::max();
    }

    return match.captured().toInt();
}

int 
GeneralUtilities::ConvertToYear(const std::string& str) {

    const std::regex regex("[0-9]{4}");
    std::smatch match;
    if (std::regex_search(str, match, regex) == false)
    {
        return std::numeric_limits<int>::max();
    }

    return stoi(match.str());
}

int
GeneralUtilities::HammingDistance(const QString& str1, const QString& str2) {

    int dist_counter = 0;
    int i = 0;
    for (; i < str1.size() && i < str2.size(); ++i) {

        if (str1[i] == str2[i])
            continue;
        dist_counter++;
    }

    return dist_counter + static_cast<int>(str1.size() - i) + static_cast<int>(str2.size() - i);
}

QString
GeneralUtilities::ConvertLatinToRomanNumbers(const QString& str1) {

    std::array<QString,21> roman_numbers = {
        "",
        "I",
        "II",
        "III",
        "IV",
        "V",
        "VI",
        "VII",
        "VIII",
        "IX",
        "X",
        "XI",
        "XII",
        "XIII",
        "XIV",
        "XV",
        "XVI",
        "XVII",
        "XVIII",
        "XIX",
        "XX",
    };

    const QRegularExpression wregex("[0-9]{1,2}");
    auto match = wregex.match(str1);
    if (match.hasMatch() == false)
        return str1;

    auto i_value = match.captured().toInt();
    if (i_value < 1 || 20 < i_value)
        return str1;

    auto offset_start = match.capturedStart();
    auto offset_end = match.capturedEnd();

    return str1.mid(0, offset_start) + roman_numbers[i_value] + str1.mid(offset_end);
}

bool 
GeneralUtilities::HasNumber(const QString& str) {

    for(auto cc : str){
        if (cc.isDigit() == false)
            return false;
    }

    return true;
}
