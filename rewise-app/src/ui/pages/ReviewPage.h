#ifndef REWISE_UI_PAGES_REVIEWPAGE_H
#define REWISE_UI_PAGES_REVIEWPAGE_H

#include "domain/Card.h"

#include <QWidget>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class ReviewPage; }
QT_END_NAMESPACE

namespace rewise::ui::widgets {
class DiffTextWidget;
class InlineMessageWidget;
}

namespace rewise::ui::pages {

class ReviewPage final : public QWidget {
    Q_OBJECT
public:
    explicit ReviewPage(QWidget* parent = nullptr);
    ~ReviewPage() override;

    void startSession(QVector<rewise::domain::Card> cards, const QString& title);
    void stopSession();

signals:
    void exitRequested();

private:
    void wireUi();

    void pickNextCard();
    void showCard();
    void clearResultUi();

private:
    Ui::ReviewPage* ui = nullptr;

    QVector<rewise::domain::Card> m_cards;
    QString m_titleText;

    int m_current = -1;
    int m_last = -1;

    rewise::ui::widgets::InlineMessageWidget* m_msg = nullptr;
    rewise::ui::widgets::DiffTextWidget* m_diff = nullptr;

    bool m_revealed = false;
    bool m_checked = false;
};

} // namespace rewise::ui::pages

#endif // REWISE_UI_PAGES_REVIEWPAGE_H
