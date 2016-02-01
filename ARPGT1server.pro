TEMPLATE = app
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

QMAKE_CXXFLAGS += -std=c++1z
debug:QMAKE_CXXFLAGS += -O1 -fsanitize=address -fno-omit-frame-pointer -Wall -Werror -g
debug:QMAKE_CXXFLAGS_WARN_ON += -Wno-missing-braces #-Wno-unused-parameter
debug:QMAKE_LFLAGS += -fsanitize=address
release:QMAKE_CXXFLAGS += -O3

HEADERS += \
    Utility/asserts.h
