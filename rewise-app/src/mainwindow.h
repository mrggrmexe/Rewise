#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include "storage/Repository.h"
#include "storage/Database.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QStackedWidget;

namespace rewise::ui::pages {
class LibraryPage;
class ReviewPage;
}

class MainWindow final : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void loadDb();
    void applyAndRefresh(const QString& successInfo = {});
    void scheduleSave();
    void saveNow();

    // Mutations
    void onFolderCreate(const QString& name);
    void onFolderRename(const rewise::domain::Id& id, const QString& newName);
    void onFolderDelete(const rewise::domain::Id& id);

    void onCardCreate(const rewise::domain::Id& folderId, const QString& q, const QString& a);
    void onCardUpdate(const rewise::domain::Id& cardId, const QString& q, const QString& a);
    void onCardDelete(const rewise::domain::Id& cardId);

    void onStartReview(const rewise::domain::Id& folderId);

private:
    Ui::MainWindow* ui = nullptr;

    rewise::storage::Repository m_repo;
    rewise::storage::Database m_db;

    QStackedWidget* m_stack = nullptr;
    rewise::ui::pages::LibraryPage* m_library = nullptr;
    rewise::ui::pages::ReviewPage* m_review = nullptr;

    QTimer m_autosave;
};

#endif // MAINWINDOW_H
