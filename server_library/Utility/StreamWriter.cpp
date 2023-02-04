#include <Utility/StreamWriter.h>

#include <filesystem>
#include <QTextStream>
#include <QFile>

void 
StreamWriter::Write(const std::string& path, const std::string& content) {

    std::ofstream output;
    output.open(path, std::ios::ios_base::out);
    output << content << std::endl;
    output.close();
}

void 
StreamWriter::Write(const QString& path, const QString& content) {

    QFile file(path);
    file.open(QIODeviceBase::ReadWrite);
    QTextStream output(&file);

    output << content << Qt::endl;
    file.close();
}

void 
StreamWriter::Write(const std::string& path, const QString& content) {

    auto p = std::filesystem::path(path);
    QFile file(p);
    file.open(QIODeviceBase::ReadWrite);
    QTextStream output(&file);

    output << content << Qt::endl;
    file.close();
}
