#ifndef REWISE_UI_WIDGETS_CARDWIDGET_H
#define REWISE_UI_WIDGETS_CARDWIDGET_H

#include "domain/Card.h"
#include "domain/Id.h"

#include <QWidget>

class QLabel;
class QPlainTextEdit;
class QPushButton;
class QToolButton;
class QSplitter;
class QStackedWidget;

namespace rewise::ui::widgets {

class DiffTextWidget;

class CardWidget final : public QWidget {
    Q_OBJECT
public:
    explicit CardWidget(QWidget* parent = nullptr);

    // Presentation controls (used by ReviewPage and popup dialog)
    void setHeaderVisible(bool v);
    void setCloseButtonVisible(bool v);
    void setElevated(bool v);

    // Layout controls
    void setWideLayout(bool enabled);

    // Library mode
    void startCreate(const rewise::domain::Id& folderId);
    void setCard(const rewise::domain::Card& card);

    // Review mode
    void setReviewCard(const rewise::domain::Card& card);

signals:
    // Library
    void saveRequested(const rewise::domain::Card& card);
    void deleteRequested(const rewise::domain::Id& cardId);
    void closeRequested();

    // Review
    void nextRequested();

private:
    enum class Mode { View, Edit, Create, Review };

    void setMode(Mode m);
    void rebuildUiForMode();

    void applyCardToEditors();
    void collectEditorsToCard();

    void resetReviewUi();
    void doCheck();

private:
    Mode m_mode = Mode::View;
    rewise::domain::Card m_card;

    QWidget* m_headerWidget = nullptr;
    QLabel* m_title = nullptr;
    QToolButton* m_btnClose = nullptr;

    QStackedWidget* m_stack = nullptr;

    // Shared: editor/view layout
    QSplitter* m_split = nullptr;

    QLabel* m_lblQuestion = nullptr;
    QPlainTextEdit* m_question = nullptr;

    QLabel* m_lblAnswer = nullptr;
    QPlainTextEdit* m_answer = nullptr;

    // Footer buttons (library)
    QWidget* m_footerWidget = nullptr;
    QPushButton* m_btnDelete = nullptr;
    QPushButton* m_btnEdit = nullptr;
    QPushButton* m_btnSave = nullptr;
    QPushButton* m_btnCancel = nullptr;

    // Review widgets
    QWidget* m_reviewPage = nullptr;
    QLabel* m_lblYourAnswer = nullptr;
    QPlainTextEdit* m_input = nullptr;
    QLabel* m_lblPercent = nullptr;
    DiffTextWidget* m_diff = nullptr;
    QWidget* m_reviewButtons = nullptr;
    QPushButton* m_btnCheck = nullptr;
    QPushButton* m_btnNext = nullptr;

    bool m_elevated = false;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_CARDWIDGET_H