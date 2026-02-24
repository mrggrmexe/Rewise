#ifndef REWISE_UI_PAGES_LIBRARYPAGE_H
#define REWISE_UI_PAGES_LIBRARYPAGE_H

#include "storage/Database.h"
#include "domain/Id.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class LibraryPage; }
QT_END_NAMESPACE

namespace rewise::ui::widgets {
class FolderListModel;
class CardTableModel;
class CardPopupDialog;
}

namespace rewise::ui::pages {

class LibraryPage final : public QWidget {
    Q_OBJECT
public:
    explicit LibraryPage(QWidget* parent = nullptr);
    ~LibraryPage() override;

    void setDatabase(rewise::storage::Database db);

    // Optional: if set, messages can be routed to notifier by method name ("showError"/"showInfo")
    void setNotifier(QObject* notify);

    rewise::domain::Id selectedFolderId() const; // invalid => all
    rewise::domain::Id selectedCardId() const;   // invalid => none

    void showError(const QString& text);
    void showInfo(const QString& text);

signals:
    void folderCreateRequested(const QString& name);
    void folderRenameRequested(const rewise::domain::Id& folderId, const QString& newName);
    void folderDeleteRequested(const rewise::domain::Id& folderId);

    void cardCreateRequested(const rewise::domain::Id& folderId,
                             const QString& question,
                             const QString& answer);
    void cardUpdateRequested(const rewise::domain::Id& cardId,
                             const QString& question,
                             const QString& answer);
    void cardDeleteRequested(const rewise::domain::Id& cardId);

    void startReviewRequested(const rewise::domain::Id& folderId); // invalid => all

private:
    void wireUi();
    void refreshFromSelection();

    void requestCreateFolder();
    void requestRenameFolder();
    void requestDeleteFolder();

    void openCreateCard();
    void openEditCard(const rewise::domain::Id& cardId);

private:
    Ui::LibraryPage* ui = nullptr;

    rewise::storage::Database m_db;

    rewise::ui::widgets::FolderListModel* m_folderModel = nullptr;
    rewise::ui::widgets::CardTableModel* m_cardModel = nullptr;

    QObject* m_notify = nullptr;
    rewise::ui::widgets::CardPopupDialog* m_cardPopup = nullptr;
};

} // namespace rewise::ui::pages

#endif // REWISE_UI_PAGES_LIBRARYPAGE_H