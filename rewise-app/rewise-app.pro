QT       += widgets
CONFIG   += c++17 warn_on

TEMPLATE = app
TARGET   = rewise-app

# (Опционально) Для более строгих предупреждений на clang/gcc
QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic

# --- Sources / Headers / Forms ---
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp

HEADERS += \
    src/mainwindow.h \
    src/domain/DomainJson.h \
    src/domain/Id.h \
    src/domain/Folder.h \
    src/domain/Card.h


FORMS += \
    src/mainwindow.ui

# --- Include paths (если начнёшь подключать подпапки) ---
INCLUDEPATH += \
    $$PWD/src \
    $$PWD/src/domain \
    $$PWD/src/storage \
    $$PWD/src/review \
    $$PWD/src/ui
