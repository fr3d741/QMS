#pragma once

#include "Macros.h"
#include <CommonDefines.h>

class DLL GeneralUtilities
{
public:
    GeneralUtilities() = delete;

    static bool IsValid(const int value);
    static QString GetTitle(const QString& str);
    static bool GetYear(const QString& title, int& year);
    static int ConvertToYear(const QString& str);
    static int ConvertToYear(const std::string& str);
    static int HammingDistance(const QString& str1, const QString& str2);
    static QString ConvertLatinToRomanNumbers(const QString& str1);
    static bool HasNumber(const QString& str);
};

