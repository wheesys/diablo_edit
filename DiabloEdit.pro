# Diablo Edit - Qt6 qmake project file
QT       += core gui widgets
TARGET    = DiabloEdit
TEMPLATE  = app
CXX_STANDARD = 17
CONFIG   += c++17 debug

INCLUDEPATH += $$PWD/src $$PWD/src/core $$PWD/src/data $$PWD/src/ui

SOURCES += \
    src/core/BinDataStream.cpp \
    src/core/MetaData.cpp \
    src/core/D2Item.cpp \
    src/core/D2S_Struct.cpp \
    src/core/ImageManager.cpp \
    src/core/quicklz.cpp \
    src/data/DataManager.cpp \
    src/ui/MainWindow.cpp \
    src/ui/CharInfoWidget.cpp \
    src/ui/WaypointsWidget.cpp \
    src/ui/QuestInfoWidget.cpp \
    src/ui/SkillsDialog.cpp \
    src/ui/SelectCharDialog.cpp \
    src/ui/ItemTooltip.cpp \
    src/ui/ItemGridWidget.cpp \
    src/ui/NewItemDialog.cpp \
    src/ui/FoundryDialog.cpp \
    src/main.cpp

HEADERS += \
    src/core/D2Types.h \
    src/core/D2Version.h \
    src/core/BinDataStream.h \
    src/core/MayExist.h \
    src/core/MetaData.h \
    src/core/DataManagerFwd.h \
    src/core/D2Item.h \
    src/core/D2S_Struct.h \
    src/core/ImageManager.h \
    src/core/quicklz.h \
    src/core/compress_quicklz.h \
    src/data/DataManager.h \
    src/ui/MainWindow.h \
    src/ui/CharInfoWidget.h \
    src/ui/WaypointsWidget.h \
    src/ui/QuestInfoWidget.h \
    src/ui/SkillsDialog.h \
    src/ui/SelectCharDialog.h \
    src/ui/ItemTooltip.h \
    src/ui/ItemGridWidget.h \
    src/ui/NewItemDialog.h \
    src/ui/FoundryDialog.h

RESOURCES += resources/resources.qrc
