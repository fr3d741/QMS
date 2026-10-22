#include <Utility/GeneralUtilities.h>

#include <regex>
#include <array>
#include <limits>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

bool
GeneralUtilities::IsValid(const int value){
    return value == std::numeric_limits<int>::max();
}

QString
GeneralUtilities::GetTitle(const QString& str){

    const static QRegularExpression regex("[(][0-9]{4}[)]");
    auto match = regex.match(str);
    QString title_wo_year = str.mid(0, match.capturedStart());
    return title_wo_year.trimmed();
}

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
