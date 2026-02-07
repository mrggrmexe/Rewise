#ifndef REWISE_UI_PAGES_LIBRARYPAGE_H
#define REWISE_UI_PAGES_LIBRARYPAGE_H

#include "domain/Id.h"
#include "storage/Database.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class LibraryPage; }
QT_END_NAMESPACE

namespace rewise::ui::widgets {
class FolderListModel;
class CardTableModel;
class NotificationCenter;
class CardPopupDialog;
}

namespace rewise::ui::pages {

class LibraryPage final : public QWidget {
    Q_OBJECT
public:
    explicit LibraryPage(QWidget* parent = nullptr);
    ~LibraryPage() override;

    void setNotifier(rewise::ui::widgets::NotificationCenter* n);

    void setDatabase(const rewise::storage::Database& db);

signals:
    void folderCreateRequested(const QString& name);
    void folderRenameRequested(const rewise::domain::Id& id, const QString& newName);
    void folderDeleteRequested(const rewise::domain::Id& id);

    void cardCreateRequested(const rewise::domain::Id& folderId, const QString& question, const QString& answer);
    void cardUpdateRequested(const rewise::domain::Id& cardId, const QString& question, const QString& answer);
    void cardDeleteRequested(const rewise::domain::Id& cardId);

    void startReviewRequested(const rewise::domain::Id& folderId);

private:
    enum class FolderEditMode { None, Create, Rename };

    rewise::domain::Id currentFolderId() const;

    void openFolderEditorCreate();
    void openFolderEditorRename(const rewise::domain::Folder& f);
    void closeFolderEditor();
    void commitFolderEditor();

    void openNewCardPopup();
    void openCardPopup(const rewise::domain::Card& card, const QPoint& globalAnchor);
    void closeCardPopup();

    void showError(const QString& text);
    void showInfo(const QString& text);

private:
    Ui::LibraryPage* ui = nullptr;

    rewise::storage::Database m_db;

    rewise::ui::widgets::NotificationCenter* m_notify = nullptr;
    rewise::ui::widgets::FolderListModel* m_foldersModel = nullptr;
    rewise::ui::widgets::CardTableModel* m_cardsModel = nullptr;

    FolderEditMode m_folderEditMode = FolderEditMode::None;
    rewise::domain::Id m_editFolderId;

    rewise::ui::widgets::CardPopupDialog* m_cardPopup = nullptr;
};

} // namespace rewise::ui::pages

#endif // REWISE_UI_PAGES_LIBRARYPAGE_H
