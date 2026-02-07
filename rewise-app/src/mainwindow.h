#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "storage/Database.h"
#include "storage/Repository.h"

#include <QMainWindow>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QStackedWidget;

namespace rewise::ui::pages {
class LibraryPage;
class ReviewPage;
}

namespace rewise::ui::widgets {
class NotificationCenter;
}

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void loadDb();
    void applyAndRefresh(QString successInfo = {});
    void saveNow();

private slots:
    void onFolderCreate(const QString& name);
    void onFolderRename(const rewise::domain::Id& id, const QString& newName);
    void onFolderDelete(const rewise::domain::Id& id);

    void onCardCreate(const rewise::domain::Id& folderId, const QString& question, const QString& answer);
    void onCardUpdate(const rewise::domain::Id& cardId, const QString& question, const QString& answer);
    void onCardDelete(const rewise::domain::Id& cardId);

    void onStartReview(const rewise::domain::Id& folderId);

private:
    Ui::MainWindow* ui = nullptr;

    QStackedWidget* m_stack = nullptr;

    rewise::storage::Database m_db;
    std::unique_ptr<rewise::storage::Repository> m_repo;

    rewise::ui::widgets::NotificationCenter* m_notify = nullptr;

    rewise::ui::pages::LibraryPage* m_library = nullptr;
    rewise::ui::pages::ReviewPage* m_review = nullptr;
};

#endif // MAINWINDOW_H
