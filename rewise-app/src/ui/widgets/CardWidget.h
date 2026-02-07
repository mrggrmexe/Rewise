#ifndef REWISE_UI_WIDGETS_CARDWIDGET_H
#define REWISE_UI_WIDGETS_CARDWIDGET_H

#include "domain/Card.h"
#include "domain/Id.h"

#include <QWidget>

class QLabel;
class QTextBrowser;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QStackedWidget;
class QToolButton;

namespace rewise::ui::widgets {

class DiffTextWidget;

class CardWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Mode { LibraryView, LibraryEdit, Review };

    explicit CardWidget(QWidget* parent = nullptr);
    ~CardWidget() override = default;

    void setElevated(bool elevated);
    void setHeaderVisible(bool visible);
    void setCloseButtonVisible(bool visible);

    void setMode(Mode m);
    Mode mode() const { return m_mode; }

    // Library use-cases
    void setCard(const rewise::domain::Card& card);              // loads & shows in view mode
    void startEdit(const rewise::domain::Card& card);            // loads & opens edit mode
    void startCreate(const rewise::domain::Id& folderContext);   // opens edit mode with empty fields

    rewise::domain::Id cardId() const { return m_cardId; }
    rewise::domain::Id folderContext() const { return m_folderCtx; }

    // Review use-cases
    void setReviewCard(const rewise::domain::Card& card);
    void clearReview();

signals:
    void closeRequested();

    void saveRequested(const rewise::domain::Id& cardId,
                       const rewise::domain::Id& folderCtx,
                       const QString& question,
                       const QString& answer);

    void deleteRequested(const rewise::domain::Id& cardId);

    void nextRequested(); // review navigation

private:
    void buildUi();

    void setHeaderTitle(const QString& title);

    void enterLibraryViewMode();
    void enterLibraryEditMode(bool isCreate);

    void resetReviewUi();
    void applyCardToUi();

    void onEdit();
    void onDelete();
    void onSave();
    void onCancel();

    void onCheck();
    void onNext();

private:
    Mode m_mode = Mode::LibraryView;

    bool m_elevated = false;

    rewise::domain::Id m_cardId;
    rewise::domain::Id m_folderCtx;

    QString m_question;
    QString m_answer;

    // Header
    QWidget* m_header = nullptr;
    QLabel* m_headerTitle = nullptr;
    QToolButton* m_btnClose = nullptr;

    QStackedWidget* m_stack = nullptr;

    // Library view
    QWidget* m_viewPage = nullptr;
    QTextBrowser* m_viewQuestion = nullptr;
    QTextBrowser* m_viewAnswer = nullptr;
    QPushButton* m_btnEdit = nullptr;
    QPushButton* m_btnDelete = nullptr;

    // Library edit
    QWidget* m_editPage = nullptr;
    QLabel* m_editTitle = nullptr;
    QLineEdit* m_leQuestion = nullptr;
    QPlainTextEdit* m_pteAnswer = nullptr;
    QPushButton* m_btnSave = nullptr;
    QPushButton* m_btnCancel = nullptr;

    // Review
    QWidget* m_reviewPage = nullptr;
    QTextBrowser* m_reviewQuestion = nullptr;
    QPlainTextEdit* m_reviewInput = nullptr;
    QPushButton* m_btnCheck = nullptr;
    QPushButton* m_btnNext = nullptr;
    QLabel* m_lblPercent = nullptr;
    DiffTextWidget* m_diff = nullptr;

    bool m_reviewChecked = false;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_CARDWIDGET_H
