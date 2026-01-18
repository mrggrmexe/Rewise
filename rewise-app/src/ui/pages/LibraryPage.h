#ifndef REWISE_UI_PAGES_LIBRARYPAGE_H
#define REWISE_UI_PAGES_LIBRARYPAGE_H

#include "storage/Database.h"
#include "domain/Id.h"

#include <QWidget>

class QListView;
class QTableView;
class QLineEdit;
class QTextBrowser;
class QPushButton;

namespace rewise::ui::widgets {
class InlineMessageWidget;
class FolderListModel;
class CardTableModel;
}

namespace rewise::ui::pages {

class LibraryPage final : public QWidget {
    Q_OBJECT
public:
    explicit LibraryPage(QWidget* parent = nullptr);

    void setDatabase(rewise::storage::Database db);

    // Баннеры
    void showError(const QString& text);
    void showInfo(const QString& text);
    void clearMessage();

    // Состояние выбора
    rewise::domain::Id selectedFolderId() const; // invalid => all
    rewise::domain::Id selectedCardId() const;

signals:
    // Folders
    void folderCreateRequested(const QString& name);
    void folderRenameRequested(const rewise::domain::Id& folderId, const QString& newName);
    void folderDeleteRequested(const rewise::domain::Id& folderId);

    // Cards
    void cardCreateRequested(const rewise::domain::Id& folderId,
                             const QString& question,
                             const QString& answer);
    void cardUpdateRequested(const rewise::domain::Id& cardId,
                             const QString& question,
                             const QString& answer);
    void cardDeleteRequested(const rewise::domain::Id& cardId);

    // Review
    void startReviewRequested(const rewise::domain::Id& folderId); // invalid => all

private:
    void buildUi();
    void wireUi();

    void refreshButtons();
    void refreshPreview();

    rewise::storage::Database m_db;

    rewise::ui::widgets::InlineMessageWidget* m_msg = nullptr;

    QListView* m_foldersView = nullptr;
    QTableView* m_cardsView = nullptr;
    QLineEdit* m_search = nullptr;
    QTextBrowser* m_preview = nullptr;

    QPushButton* m_addFolder = nullptr;
    QPushButton* m_renameFolder = nullptr;
    QPushButton* m_deleteFolder = nullptr;

    QPushButton* m_addCard = nullptr;
    QPushButton* m_editCard = nullptr;
    QPushButton* m_deleteCard = nullptr;
    QPushButton* m_review = nullptr;

    rewise::ui::widgets::FolderListModel* m_folderModel = nullptr;
    rewise::ui::widgets::CardTableModel* m_cardModel = nullptr;
};

} // namespace rewise::ui::pages

#endif // REWISE_UI_PAGES_LIBRARYPAGE_H
