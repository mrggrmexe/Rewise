QT       += widgets
CONFIG   += c++17 warn_on

TEMPLATE = app
TARGET   = rewise-app

QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic

# --- Sources / Headers / Forms ---
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/review/TextNormalize.cpp \
    src/review/Levenshtein.cpp \
    src/review/WordDiff.cpp \
    src/review/ReviewEngine.cpp \
    src/storage/Database.cpp \
    src/storage/Repository.cpp \
    src/ui/widgets/InlineMessageWidget.cpp \
    src/ui/widgets/FolderListModel.cpp \
    src/ui/widgets/CardTableModel.cpp \
    src/ui/widgets/FolderEditDialog.cpp \
    src/ui/widgets/CardEditDialog.cpp \
    src/ui/widgets/DiffTextWidget.cpp \
    src/ui/pages/LibraryPage.cpp \
    src/ui/pages/ReviewPage.cpp

HEADERS += \
    src/mainwindow.h \
    src/domain/DomainJson.h \
    src/domain/Id.h \
    src/domain/Folder.h \
    src/domain/Card.h \
    src/review/ReviewTypes.h \
    src/review/TextNormalize.h \
    src/review/Levenshtein.h \
    src/review/WordDiff.h \
    src/review/ReviewEngine.h \
    src/storage/StorageJson.h \
    src/storage/Database.h \
    src/storage/Repository.h \
    src/ui/widgets/InlineMessageWidget.h \
    src/ui/widgets/FolderListModel.h \
    src/ui/widgets/CardTableModel.h \
    src/ui/widgets/FolderEditDialog.h \
    src/ui/widgets/CardEditDialog.h \
    src/ui/widgets/DiffTextWidget.h \
    src/ui/pages/LibraryPage.h \
    src/ui/pages/ReviewPage.h

FORMS += \
    src/mainwindow.ui

# --- Include paths ---
INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/domain \
    $$PWD/src/storage \
    $$PWD/src/review \
    $$PWD/src/ui
