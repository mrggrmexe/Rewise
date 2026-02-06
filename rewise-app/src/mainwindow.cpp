#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ui/pages/LibraryPage.h"
#include "ui/pages/ReviewPage.h"

#include <QMessageBox>
#include <QStackedWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_repo("db.json")
{
    ui->setupUi(this);
    setWindowTitle("Rewise");

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    m_library = new rewise::ui::pages::LibraryPage(this);
    m_review = new rewise::ui::pages::ReviewPage(this);

    m_stack->addWidget(m_library);
    m_stack->addWidget(m_review);
    m_stack->setCurrentWidget(m_library);

    // Autosave debounce
    m_autosave.setSingleShot(true);
    m_autosave.setInterval(250);
    connect(&m_autosave, &QTimer::timeout, this, &MainWindow::saveNow);

    // Wire library signals
    connect(m_library, &rewise::ui::pages::LibraryPage::folderCreateRequested, this, &MainWindow::onFolderCreate);
    connect(m_library, &rewise::ui::pages::LibraryPage::folderRenameRequested, this, &MainWindow::onFolderRename);
    connect(m_library, &rewise::ui::pages::LibraryPage::folderDeleteRequested, this, &MainWindow::onFolderDelete);

    connect(m_library, &rewise::ui::pages::LibraryPage::cardCreateRequested, this, &MainWindow::onCardCreate);
    connect(m_library, &rewise::ui::pages::LibraryPage::cardUpdateRequested, this, &MainWindow::onCardUpdate);
    connect(m_library, &rewise::ui::pages::LibraryPage::cardDeleteRequested, this, &MainWindow::onCardDelete);

    connect(m_library, &rewise::ui::pages::LibraryPage::startReviewRequested, this, &MainWindow::onStartReview);

    connect(m_review, &rewise::ui::pages::ReviewPage::exitRequested, this, [this] {
        m_stack->setCurrentWidget(m_library);
    });

    loadDb();
    applyAndRefresh();

#ifndef QT_NO_DEBUG
    statusBar()->showMessage("DB: " + m_repo.databaseFilePath());
#else
    statusBar()->hide();
#endif
}

MainWindow::~MainWindow() {
    // Попытка сохранить перед выходом (best-effort)
    saveNow();
    delete ui;
}

void MainWindow::loadDb() {
    QString err;
    rewise::storage::Database db;
    if (!m_repo.load(&db, &err)) {
        QMessageBox::warning(this, "Rewise",
                             "Не удалось загрузить базу.\n\n" + err + "\n\nБудет создана новая база.");
        db.folders.clear();
        db.cards.clear();
        db.ensureDefaultFolder();
    }
    m_db = std::move(db);
}

void MainWindow::applyAndRefresh(const QString& successInfo) {
    // self-heal names
    m_db.ensureUniqueFolderNames();

    QString why;
    if (!m_db.validate(&why)) {
        m_library->showError("DB invalid: " + why);
    } else if (!successInfo.isEmpty()) {
        m_library->showInfo(successInfo);
    } else {
        m_library->clearMessage();
    }

    m_library->setDatabase(m_db);
    scheduleSave();
}

void MainWindow::scheduleSave() {
    m_autosave.start();
}

void MainWindow::saveNow() {
    if (!m_db.validate()) return;
    QString err;
    if (!m_repo.save(m_db, &err)) {
        // Только сообщение: не рушим работу пользователя.
        if (m_library) m_library->showError("Не удалось сохранить базу: " + err);
    }
}

void MainWindow::onFolderCreate(const QString& name) {
    rewise::domain::Folder f;
    f.id = rewise::domain::Id::create();
    f.name = name.trimmed();
    if (!f.isValid()) {
        m_library->showError("Некорректное имя папки.");
        return;
    }
    m_db.folders.push_back(f);
    applyAndRefresh("Папка создана.");
}

void MainWindow::onFolderRename(const rewise::domain::Id& id, const QString& newName) {
    auto* f = m_db.folderById(id);
    if (!f) return;
    f->name = newName.trimmed();
    applyAndRefresh("Папка переименована.");
}

void MainWindow::onFolderDelete(const rewise::domain::Id& id) {
    // Перенос карточек в Default
    const auto defaultId = m_db.ensureDefaultFolder();
    if (id == defaultId) {
        m_library->showError("Папку Default удалить нельзя.");
        return;
    }

    for (auto& c : m_db.cards) {
        if (c.folderId == id) {
            c.folderId = defaultId;
            c.touchUpdatedNow();
        }
    }

    const int idx = m_db.folderIndexById(id);
    if (idx >= 0 && idx < m_db.folders.size()) {
        m_db.folders.removeAt(idx);
    }
    applyAndRefresh("Папка удалена, карточки перенесены в Default.");
}

void MainWindow::onCardCreate(const rewise::domain::Id& folderId, const QString& q, const QString& a) {
    rewise::domain::Id targetFolder = folderId.isValid() ? folderId : m_db.ensureDefaultFolder();
    if (!m_db.folderById(targetFolder)) targetFolder = m_db.ensureDefaultFolder();

    rewise::domain::Card c;
    c.id = rewise::domain::Id::create();
    c.folderId = targetFolder;
    c.question = q.trimmed();
    c.answer = a.trimmed();
    c.touchCreatedNow();

    QString why;
    if (!c.isValid(&why)) {
        m_library->showError("Карточка невалидна: " + why);
        return;
    }

    m_db.cards.push_back(c);
    applyAndRefresh("Карточка создана.");
}

void MainWindow::onCardUpdate(const rewise::domain::Id& cardId, const QString& q, const QString& a) {
    auto* c = m_db.cardById(cardId);
    if (!c) return;
    c->question = q.trimmed();
    c->answer = a.trimmed();
    c->touchUpdatedNow();
    applyAndRefresh("Карточка обновлена.");
}

void MainWindow::onCardDelete(const rewise::domain::Id& cardId) {
    const int idx = m_db.cardIndexById(cardId);
    if (idx < 0 || idx >= m_db.cards.size()) return;
    m_db.cards.removeAt(idx);
    applyAndRefresh("Карточка удалена.");
}

void MainWindow::onStartReview(const rewise::domain::Id& folderId) {
    QVector<rewise::domain::Card> cards;
    cards.reserve(m_db.cards.size());

    QString title = "Все карточки";
    if (folderId.isValid()) {
        const auto* f = m_db.folderById(folderId);
        title = f ? f->name : "Папка";
    }

    for (const auto& c : m_db.cards) {
        if (!folderId.isValid() || c.folderId == folderId) {
            cards.push_back(c);
        }
    }

    if (cards.isEmpty()) {
        m_library->showError("В выбранной папке нет карточек.");
        return;
    }

    m_review->startSession(std::move(cards), title);
    m_stack->setCurrentWidget(m_review);
}
