#ifndef REWISE_UI_PAGES_LIBRARYPAGE_H
#define REWISE_UI_PAGES_LIBRARYPAGE_H

#include "storage/Database.h"
#include "domain/Id.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class LibraryPage; }
QT_END_NAMESPACE

class QListView;
class QTableView;

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
    ~LibraryPage() override;

    void setDatabase(rewise::storage::Database db);

    void showError(const QString& text);
    void showInfo(const QString& text);
    void clearMessage();

    rewise::domain::Id selectedFolderId() const; // invalid => all
    rewise::domain::Id selectedCardId() const;

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
    void refreshButtons();
    void refreshPreview();

private:
    Ui::LibraryPage* ui = nullptr;

    rewise::storage::Database m_db;

    rewise::ui::widgets::InlineMessageWidget* m_msg = nullptr;
    rewise::ui::widgets::FolderListModel* m_folderModel = nullptr;
    rewise::ui::widgets::CardTableModel* m_cardModel = nullptr;
};

} // namespace rewise::ui::pages

#endif // REWISE_UI_PAGES_LIBRARYPAGE_H
