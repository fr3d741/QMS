QT -= gui
QT += network

TEMPLATE = lib
DEFINES += MEDIASERVER_LIB

CONFIG += c++17

DESTDIR = ../bin

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    CommonDefines.cpp \
    Configuration/Configuration.cpp \
    Configuration/IConfiguration.cpp \
    Factories/ConfigurationFactory.cpp \
    Factories/MessageQueueFactory.cpp \
    Factories/TcpServerFactory.cpp \
    FileSystem/FileSystemWatcher.cpp \
    FileSystem/WinFSWatcher.cpp \
    Logging/Logger.cpp \
    MediaServer.cpp \
    Model/CommonMedia.cpp \
    Model/TvShow.cpp \
    Networking/RestApi.cpp \
    Networking/WinTcpServer.cpp \
    Utility/GeneralUtilities.cpp \
    Utility/JsonNode.cpp \
    Utility/MessageQueue.cpp \
    Utility/ScopedFunction.cpp \
    Utility/StreamWriter.cpp \
    Utility/XmlNode.cpp

HEADERS += \
    CommonDefines.h \
    Configuration/Configuration.h \
    Configuration/IConfiguration.h \
    Factories/ConfigurationFactory.h \
    Factories/MessageQueueFactory.h \
    Factories/TcpServerFactory.h \
    FileSystem/FileSystemWatcher.h \
    FileSystem/WinFSWatcher.h \
    Logging/ILogger.h \
    Logging/Logger.h \
    Macros.h \
    MediaServer.h \
    Model/CommonMedia.h \
    Model/TvShow.h \
    Networking/ITcpServer.h \
    Networking/RestApi.h \
    Networking/WinTcpServer.h \
    Utility/GeneralUtilities.h \
    Utility/IMessageQueue.h \
    Utility/JsonNode.h \
    Utility/MessageQueue.h \
    Utility/ScopedFunction.h \
    Utility/StreamWriter.h \
    Utility/XmlNode.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}

!isEmpty(target.path): INSTALLS += target
