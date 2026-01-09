QT       += widgets
CONFIG   += c++17 warn_on

TEMPLATE = app
TARGET   = rewise-app

# (Опционально) Для более строгих предупреждений на clang/gcc
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
    src/storage/Repository.cpp

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
    src/storage/Repository.h



FORMS += \
    src/mainwindow.ui

# --- Include paths (если начнёшь подключать подпапки) ---
INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/domain \
    $$PWD/src/storage \
    $$PWD/src/review \
    $$PWD/src/ui
