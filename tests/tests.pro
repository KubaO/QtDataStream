QT = core
greaterThan(QT_MAJOR_VERSION, 4) {
  QT = core-private testlib
  CONFIG += c++14
} else {
  CONFIG += qtestlib
  macx:QMAKE_CXXFLAGS += -std=c++14
}
DEFINES += \
  QT_DEPRECATED_WARNINGS \
  QT_DISABLE_DEPRECATED_BEFORE=0x060000 \
  QT_RESTRICTED_CAST_FROM_ASCII
INCLUDEPATH += ../include
SOURCES = qtdatastream.cpp
HEADERS = ../include/qtdatastream.h
