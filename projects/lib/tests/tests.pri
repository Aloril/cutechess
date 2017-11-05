TEMPLATE = app

win32:config += CONSOLE
CONFIG += qtestlib
CONFIG += c++11

include(../lib.pri)

OBJECTS_DIR = .obj
MOC_DIR = .moc
