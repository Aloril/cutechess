TARGET = cutechess-cli
DESTDIR = $$PWD

include(../lib/lib.pri)

CUTECHESS_CLI_VERSION = 0.6.0

macx-xcode {
    DEFINES += CUTECHESS_CLI_VERSION=\"$$CUTECHESS_CLI_VERSION\"
} else {
    OBJECTS_DIR = .obj/
    MOC_DIR = .moc/
    RCC_DIR = .rcc/
    DEFINES += CUTECHESS_CLI_VERSION=\\\"$$CUTECHESS_CLI_VERSION\\\"
}

win32 {
    CONFIG += console
}

mac {
    CONFIG -= app_bundle
}

CONFIG += c++11

QT = core

# Code
include(src/src.pri)

# Resources
include(res/res.pri)
