#ifndef REWISE_UI_PAGES_REVIEWPAGE_H
#define REWISE_UI_PAGES_REVIEWPAGE_H

#include "domain/Card.h"
#include "domain/Folder.h"

#include <QWidget>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui { class ReviewPage; }
QT_END_NAMESPACE

namespace rewise::ui::widgets {
class CardWidget;
class NotificationCenter;
}

namespace rewise::ui::pages {

class ReviewPage final : public QWidget {
    Q_OBJECT
public:
    explicit ReviewPage(QWidget* parent = nullptr);
    ~ReviewPage() override;

    void setNotifier(rewise::ui::widgets::NotificationCenter* n);

    void startSession(const rewise::domain::Folder& folder,
                      const QVector<rewise::domain::Card>& cards);

    void stopSession();

signals:
    void exitRequested();

private:
    void pickAndShowNextCard();

private:
    Ui::ReviewPage* ui = nullptr;

    rewise::ui::widgets::NotificationCenter* m_notify = nullptr;
    rewise::ui::widgets::CardWidget* m_cardWidget = nullptr;

    rewise::domain::Folder m_folder;
    QVector<rewise::domain::Card> m_cards;
    int m_index = -1;
};

} // namespace rewise::ui::pages

#endif // REWISE_UI_PAGES_REVIEWPAGE_H
