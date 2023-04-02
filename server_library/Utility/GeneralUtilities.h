#pragma once

#include "Macros.h"
#include <CommonDefines.h>

class DLL GeneralUtilities
{
public:
//    static void RightTrim(QString& str);
//    static void RightTrim(std::string& str);
//    static void LeftTrim(QString& str);
//    static void LeftTrim(std::string& str);
//    static bool AreEqualCaseInsensitive(std::string str1, std::string str2);
//    static bool AreEqualCaseInsensitive(QString str1, QString str2);
//    static string Convert(const std::string& str);
//    static std::string Convert(const QString& str);
//    static bool Contains(const string& in, const string& what);
    static bool GetYear(const QString& title, int& year);
    static int ConvertToYear(const QString& str);
    static int ConvertToYear(const std::string& str);
    static int HammingDistance(const QString& str1, const QString& str2);
    static QString ConvertLatinToRomanNumbers(const QString& str1);
    static bool HasNumber(const QString& str);
};

