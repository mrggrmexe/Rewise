#ifndef REWISE_UI_PAGES_REVIEWPAGE_H
#define REWISE_UI_PAGES_REVIEWPAGE_H

#include "domain/Card.h"

#include <QWidget>
#include <QVector>

class QLabel;
class QPushButton;
class QPlainTextEdit;
class QTextBrowser;

namespace rewise::ui::widgets {
class DiffTextWidget;
class InlineMessageWidget;
}

namespace rewise::ui::pages {

class ReviewPage final : public QWidget {
    Q_OBJECT
public:
    explicit ReviewPage(QWidget* parent = nullptr);

    void startSession(QVector<rewise::domain::Card> cards, const QString& title);
    void stopSession();

signals:
    void exitRequested();

private:
    void buildUi();
    void wireUi();

    void pickNextCard();
    void showCard();
    void clearResultUi();

    QVector<rewise::domain::Card> m_cards;
    QString m_titleText;

    int m_current = -1;
    int m_last = -1;

    QLabel* m_title = nullptr;
    QLabel* m_percent = nullptr;

    QTextBrowser* m_question = nullptr;
    QPlainTextEdit* m_answer = nullptr;
    QTextBrowser* m_reference = nullptr;

    QPushButton* m_back = nullptr;
    QPushButton* m_check = nullptr;
    QPushButton* m_reveal = nullptr;
    QPushButton* m_next = nullptr;

    rewise::ui::widgets::InlineMessageWidget* m_msg = nullptr;
    rewise::ui::widgets::DiffTextWidget* m_diff = nullptr;

    bool m_revealed = false;
    bool m_checked = false;
};

} // namespace rewise::ui::pages

#endif // REWISE_UI_PAGES_REVIEWPAGE_H
