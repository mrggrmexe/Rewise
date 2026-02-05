QT += widgets

TEMPLATE = app
TARGET = rewise-app

CONFIG += c++17

INCLUDEPATH += $$PWD/src

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/storage/Database.cpp \
    src/storage/Repository.cpp \
    src/review/Levenshtein.cpp \
    src/review/ReviewEngine.cpp \
    src/review/TextNormalize.cpp \
    src/review/WordDiff.cpp \
    src/ui/pages/LibraryPage.cpp \
    src/ui/pages/ReviewPage.cpp \
    src/ui/widgets/CardEditDialog.cpp \
    src/ui/widgets/CardTableModel.cpp \
    src/ui/widgets/DiffTextWidget.cpp \
    src/ui/widgets/FolderEditDialog.cpp \
    src/ui/widgets/FolderListModel.cpp \
    src/ui/widgets/InlineMessageWidget.cpp \
    src/ui/widgets/FolderNavButton.cpp \
    src/ui/widgets/CardTileButton.cpp

HEADERS += \
    src/mainwindow.h \
    src/domain/Card.h \
    src/domain/DomainJson.h \
    src/domain/Folder.h \
    src/domain/Id.h \
    src/storage/Database.h \
    src/storage/Repository.h \
    src/storage/StorageJson.h \
    src/review/Levenshtein.h \
    src/review/ReviewEngine.h \
    src/review/ReviewTypes.h \
    src/review/TextNormalize.h \
    src/review/WordDiff.h \
    src/ui/pages/LibraryPage.h \
    src/ui/pages/ReviewPage.h \
    src/ui/widgets/CardEditDialog.h \
    src/ui/widgets/CardTableModel.h \
    src/ui/widgets/DiffTextWidget.h \
    src/ui/widgets/FolderEditDialog.h \
    src/ui/widgets/FolderListModel.h \
    src/ui/widgets/InlineMessageWidget.h \
    src/ui/widgets/LayoutUtils.h \
    src/ui/widgets/FolderNavButton.h \
    src/ui/widgets/CardTileButton.h

FORMS += \
    src/mainwindow.ui \
    src/ui/pages/LibraryPage.ui \
    src/ui/pages/ReviewPage.ui

RESOURCES += \
    resources/resources.qrc
