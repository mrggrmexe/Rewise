#ifndef REWISE_UI_WIDGETS_DIFFTEXTWIDGET_H
#define REWISE_UI_WIDGETS_DIFFTEXTWIDGET_H

#include "review/ReviewTypes.h"

#include <QWidget>

class QTextBrowser;
class QLabel;

namespace rewise::ui::widgets {

class DiffTextWidget final : public QWidget {
    Q_OBJECT
public:
    explicit DiffTextWidget(QWidget* parent = nullptr);

    void clear();
    void setReviewResult(const rewise::review::ReviewResult& r);

private:
    static QString renderTokens(const QVector<rewise::review::StyledToken>& tokens, bool isReference);

    QLabel* m_title = nullptr;
    QTextBrowser* m_view = nullptr;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_DIFFTEXTWIDGET_H
