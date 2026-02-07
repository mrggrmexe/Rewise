QT += core gui widgets
CONFIG += c++17

TEMPLATE = app
TARGET = rewise-app

# IMPORTANT: allow includes like "domain/Id.h", "review/ReviewEngine.h", etc.
INCLUDEPATH += $$PWD/src
DEPENDPATH  += $$PWD/src

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/review/Levenshtein.cpp \
    src/review/ReviewEngine.cpp \
    src/review/TextNormalize.cpp \
    src/review/WordDiff.cpp \
    src/storage/Database.cpp \
    src/storage/Repository.cpp \
    src/ui/pages/LibraryPage.cpp \
    src/ui/pages/ReviewPage.cpp \
    src/ui/widgets/CardEditDialog.cpp \
    src/ui/widgets/CardTableModel.cpp \
    src/ui/widgets/CardTileButton.cpp \
    src/ui/widgets/DiffTextWidget.cpp \
    src/ui/widgets/FolderEditDialog.cpp \
    src/ui/widgets/FolderListModel.cpp \
    src/ui/widgets/FolderNavButton.cpp \
    src/ui/widgets/InlineMessageWidget.cpp \
    src/ui/widgets/CardWidget.cpp \
    src/ui/widgets/CardPopupDialog.cpp \
    src/ui/widgets/ToastWidget.cpp \
    src/ui/widgets/NotificationCenter.cpp

HEADERS += \
    src/mainwindow.h \
    src/domain/Card.h \
    src/domain/DomainJson.h \
    src/domain/Folder.h \
    src/domain/Id.h \
    src/review/Levenshtein.h \
    src/review/ReviewEngine.h \
    src/review/ReviewTypes.h \
    src/review/TextNormalize.h \
    src/review/WordDiff.h \
    src/storage/Database.h \
    src/storage/Repository.h \
    src/storage/StorageJson.h \
    src/ui/pages/LibraryPage.h \
    src/ui/pages/ReviewPage.h \
    src/ui/widgets/CardEditDialog.h \
    src/ui/widgets/CardTableModel.h \
    src/ui/widgets/CardTileButton.h \
    src/ui/widgets/DiffTextWidget.h \
    src/ui/widgets/FolderEditDialog.h \
    src/ui/widgets/FolderListModel.h \
    src/ui/widgets/FolderNavButton.h \
    src/ui/widgets/InlineMessageWidget.h \
    src/ui/widgets/LayoutUtils.h \
    src/ui/widgets/CardWidget.h \
    src/ui/widgets/CardPopupDialog.h \
    src/ui/widgets/ToastWidget.h \
    src/ui/widgets/NotificationCenter.h

FORMS += \
    src/mainwindow.ui \
    src/ui/pages/LibraryPage.ui \
    src/ui/pages/ReviewPage.ui

RESOURCES += resources/resources.qrc
