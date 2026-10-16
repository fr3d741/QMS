#include <Utility/GeneralUtilities.h>

#include <QtTest>

#include <limits>
#include <string>

class TestGeneralUtilities : public QObject
{
    Q_OBJECT

private slots:
    void getTitle_data();
    void getTitle();

    void getYear_data();
    void getYear();

    void convertToYear_data();
    void convertToYear();
    void convertToYearStdString();

    void hammingDistance_data();
    void hammingDistance();

    void convertLatinToRomanNumbers_data();
    void convertLatinToRomanNumbers();

    void hasNumber_data();
    void hasNumber();

    void isValid();
};

void
TestGeneralUtilities::getTitle_data() {
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("with year") << "The Matrix (1999)" << "The Matrix";
    QTest::newRow("year in middle") << "Alien (1979) Directors Cut" << "Alien";
    QTest::newRow("no year") << "  Heat  " << "Heat";
    QTest::newRow("empty") << "" << "";
}

void
TestGeneralUtilities::getTitle() {
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QCOMPARE(GeneralUtilities::GetTitle(input), expected);
}

void
TestGeneralUtilities::getYear_data() {
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("found");
    QTest::addColumn<int>("year");

    QTest::newRow("with year") << "The Matrix (1999)" << true << 1999;
    QTest::newRow("no parentheses") << "The Matrix 1999" << false << 0;
    QTest::newRow("short number") << "Movie (99)" << false << 0;
}

void
TestGeneralUtilities::getYear() {
    QFETCH(QString, input);
    QFETCH(bool, found);
    QFETCH(int, year);

    int result = 0;
    QCOMPARE(GeneralUtilities::GetYear(input, result), found);
    QCOMPARE(result, year);
}

void
TestGeneralUtilities::convertToYear_data() {
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("expected");

    QTest::newRow("plain") << "2004" << 2004;
    QTest::newRow("parenthesized") << "(1984)" << 1984;
    QTest::newRow("no year") << "abc" << std::numeric_limits<int>::max();
    QTest::newRow("too short") << "123" << std::numeric_limits<int>::max();
}

void
TestGeneralUtilities::convertToYear() {
    QFETCH(QString, input);
    QFETCH(int, expected);
    QCOMPARE(GeneralUtilities::ConvertToYear(input), expected);
}

void
TestGeneralUtilities::convertToYearStdString() {
    QCOMPARE(GeneralUtilities::ConvertToYear(std::string("released 2010")), 2010);
    QCOMPARE(GeneralUtilities::ConvertToYear(std::string("none")), std::numeric_limits<int>::max());
}

void
TestGeneralUtilities::hammingDistance_data() {
    QTest::addColumn<QString>("a");
    QTest::addColumn<QString>("b");
    QTest::addColumn<int>("expected");

    QTest::newRow("equal") << "karolin" << "karolin" << 0;
    QTest::newRow("classic") << "karolin" << "kathrin" << 3;
    QTest::newRow("length difference") << "abc" << "abcde" << 2;
    QTest::newRow("empty vs text") << "" << "abc" << 3;
}

void
TestGeneralUtilities::hammingDistance() {
    QFETCH(QString, a);
    QFETCH(QString, b);
    QFETCH(int, expected);
    QCOMPARE(GeneralUtilities::HammingDistance(a, b), expected);
    QCOMPARE(GeneralUtilities::HammingDistance(b, a), expected);
}

void
TestGeneralUtilities::convertLatinToRomanNumbers_data() {
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("single digit") << "Rocky 2" << "Rocky II";
    QTest::newRow("two digits") << "Friday the 13th" << "Friday the XIIIth";
    QTest::newRow("upper bound") << "Part 20" << "Part XX";
    QTest::newRow("out of range") << "Part 21" << "Part 21";
    QTest::newRow("zero") << "Part 0" << "Part 0";
    QTest::newRow("no number") << "Heat" << "Heat";
}

void
TestGeneralUtilities::convertLatinToRomanNumbers() {
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QCOMPARE(GeneralUtilities::ConvertLatinToRomanNumbers(input), expected);
}

void
TestGeneralUtilities::hasNumber_data() {
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("expected");

    // HasNumber currently checks that every character is a digit
    QTest::newRow("digits only") << "12345" << true;
    QTest::newRow("mixed") << "abc1" << false;
    QTest::newRow("letters") << "abc" << false;
    QTest::newRow("empty") << "" << true;
}

void
TestGeneralUtilities::hasNumber() {
    QFETCH(QString, input);
    QFETCH(bool, expected);
    QCOMPARE(GeneralUtilities::HasNumber(input), expected);
}

void
TestGeneralUtilities::isValid() {
    // IsValid currently reports true for the "invalid" sentinel returned by ConvertToYear
    QVERIFY(GeneralUtilities::IsValid(std::numeric_limits<int>::max()));
    QVERIFY(!GeneralUtilities::IsValid(1999));
}

QTEST_APPLESS_MAIN(TestGeneralUtilities)

#include "tst_generalutilities.moc"
