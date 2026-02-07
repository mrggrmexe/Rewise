#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "ui/pages/LibraryPage.h"
#include "ui/pages/ReviewPage.h"

#include "ui/widgets/NotificationCenter.h"

#include <QStackedWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);

    // Global notification layer (anchored to stacked widget)
    m_notify = new rewise::ui::widgets::NotificationCenter(m_stack, this);

    // Data layer (Repository сам выбирает AppDataLocation и хранит db.json)
    m_repo = std::make_unique<rewise::storage::Repository>(QStringLiteral("db.json"));

    loadDb();

    // Pages
    m_library = new rewise::ui::pages::LibraryPage(this);
    m_library->setNotifier(m_notify);
    m_library->setDatabase(m_db);

    m_review = new rewise::ui::pages::ReviewPage(this);
    m_review->setNotifier(m_notify);

    m_stack->addWidget(m_library);
    m_stack->addWidget(m_review);
    m_stack->setCurrentWidget(m_library);

    // Wiring: library -> actions
    connect(m_library, &rewise::ui::pages::LibraryPage::folderCreateRequested,
            this, &MainWindow::onFolderCreate);
    connect(m_library, &rewise::ui::pages::LibraryPage::folderRenameRequested,
            this, &MainWindow::onFolderRename);
    connect(m_library, &rewise::ui::pages::LibraryPage::folderDeleteRequested,
            this, &MainWindow::onFolderDelete);

    connect(m_library, &rewise::ui::pages::LibraryPage::cardCreateRequested,
            this, &MainWindow::onCardCreate);
    connect(m_library, &rewise::ui::pages::LibraryPage::cardUpdateRequested,
            this, &MainWindow::onCardUpdate);
    connect(m_library, &rewise::ui::pages::LibraryPage::cardDeleteRequested,
            this, &MainWindow::onCardDelete);

    connect(m_library, &rewise::ui::pages::LibraryPage::startReviewRequested,
            this, &MainWindow::onStartReview);

    // Wiring: review -> back
    connect(m_review, &rewise::ui::pages::ReviewPage::exitRequested, this, [this] {
        m_stack->setCurrentWidget(m_library);
    });
}

MainWindow::~MainWindow() {
    saveNow();
    delete ui;
}

void MainWindow::loadDb() {
    // Если репозитория нет — просто поднимем пустую БД
    if (!m_repo) {
        m_db = rewise::storage::Database{};
        m_db.ensureDefaultFolder();
        return;
    }

    rewise::storage::Database tmp;
    QString err;

    const bool ok = m_repo->load(&tmp, &err);
    if (!ok) {
        if (m_notify) {
            m_notify->showError(QStringLiteral("Не удалось загрузить базу: %1\nСоздана новая база.")
                                    .arg(err.isEmpty() ? QStringLiteral("unknown error") : err));
        }
        m_db = rewise::storage::Database{};
        m_db.ensureDefaultFolder();
        return;
    }

    m_db = tmp;
    m_db.ensureDefaultFolder();

    QString why;
    if (!m_db.validate(&why)) {
        if (m_notify) {
            m_notify->showError(QStringLiteral("База повреждена: %1\nСоздана новая база.").arg(why));
        }
        m_db = rewise::storage::Database{};
        m_db.ensureDefaultFolder();
    }
}

void MainWindow::applyAndRefresh(QString successInfo) {
    QString why;
    if (!m_db.validate(&why)) {
        if (m_notify) m_notify->showError(QStringLiteral("DB invalid: %1").arg(why));
        m_db = rewise::storage::Database{};
        m_db.ensureDefaultFolder();
        if (m_library) m_library->setDatabase(m_db);
        return;
    }

    if (m_library) m_library->setDatabase(m_db);

    if (!successInfo.isEmpty() && m_notify) {
        m_notify->showInfo(successInfo);
    }

    saveNow();
}

void MainWindow::saveNow() {
    if (!m_repo) return;

    QString why;
    if (!m_db.validate(&why)) {
        if (m_notify) m_notify->showError(QStringLiteral("Не сохраняю: DB invalid: %1").arg(why));
        return;
    }

    QString err;
    if (!m_repo->save(m_db, &err)) {
        if (m_notify) {
            m_notify->showError(QStringLiteral("Не удалось сохранить базу: %1")
                                    .arg(err.isEmpty() ? QStringLiteral("unknown error") : err));
        }
    }
}

void MainWindow::onFolderCreate(const QString& name) {
    rewise::domain::Folder f;
    f.id = rewise::domain::Id::create();
    f.name = name;

    m_db.folders.push_back(f);
    applyAndRefresh(QStringLiteral("Папка создана"));
}

void MainWindow::onFolderRename(const rewise::domain::Id& id, const QString& newName) {
    auto* f = m_db.folderById(id);
    if (!f) {
        if (m_notify) m_notify->showError(QStringLiteral("Папка не найдена."));
        return;
    }

    f->name = newName;
    applyAndRefresh(QStringLiteral("Папка обновлена"));
}

void MainWindow::onFolderDelete(const rewise::domain::Id& id) {
    if (!id.isValid()) return;

    // Ensure Default exists
    const rewise::domain::Id defaultId = m_db.ensureDefaultFolder();

    // Move cards out
    for (auto& c : m_db.cards) {
        if (c.folderId == id) {
            c.folderId = defaultId;
            c.touchUpdatedNow();
        }
    }

    // Remove folder
    const int idx = m_db.folderIndexById(id);
    if (idx >= 0) {
        m_db.folders.removeAt(idx);
    }

    applyAndRefresh(QStringLiteral("Папка удалена"));
}

void MainWindow::onCardCreate(const rewise::domain::Id& folderId,
                              const QString& question,
                              const QString& answer) {
    rewise::domain::Id fid = folderId;
    if (!fid.isValid()) {
        fid = m_db.ensureDefaultFolder();
    }

    rewise::domain::Card c;
    c.id = rewise::domain::Id::create();
    c.folderId = fid;
    c.question = question;
    c.answer = answer;
    c.touchCreatedNow();

    m_db.cards.push_back(c);
    applyAndRefresh(QStringLiteral("Карточка создана"));
}

void MainWindow::onCardUpdate(const rewise::domain::Id& cardId,
                              const QString& question,
                              const QString& answer) {
    auto* c = m_db.cardById(cardId);
    if (!c) {
        if (m_notify) m_notify->showError(QStringLiteral("Карточка не найдена."));
        return;
    }

    c->question = question;
    c->answer = answer;
    c->touchUpdatedNow();

    applyAndRefresh(QStringLiteral("Карточка обновлена"));
}

void MainWindow::onCardDelete(const rewise::domain::Id& cardId) {
    const int idx = m_db.cardIndexById(cardId);
    if (idx < 0) {
        if (m_notify) m_notify->showError(QStringLiteral("Карточка не найдена."));
        return;
    }

    m_db.cards.removeAt(idx);
    applyAndRefresh(QStringLiteral("Карточка удалена"));
}

void MainWindow::onStartReview(const rewise::domain::Id& folderId) {
    // Collect cards
    QVector<rewise::domain::Card> cards;

    if (folderId.isValid()) {
        for (const auto& c : m_db.cards) {
            if (c.folderId == folderId) cards.push_back(c);
        }
    } else {
        cards = m_db.cards;
    }

    if (cards.isEmpty()) {
        if (m_notify) m_notify->showInfo(QStringLiteral("Нет карточек для повторения."));
        return;
    }

    rewise::domain::Folder folder;
    if (folderId.isValid()) {
        const auto* f = m_db.folderById(folderId);
        if (!f) {
            if (m_notify) m_notify->showError(QStringLiteral("Папка не найдена."));
            return;
        }
        folder = *f;
    } else {
        folder.id = {};
        folder.name = QStringLiteral("Все карточки");
    }

    m_review->startSession(folder, cards);
    m_stack->setCurrentWidget(m_review);
}
