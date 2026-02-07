#ifndef REWISE_UI_WIDGETS_DIFFTEXTWIDGET_H
#define REWISE_UI_WIDGETS_DIFFTEXTWIDGET_H

#include "review/ReviewTypes.h"

#include <QWidget>

class QTextBrowser;

namespace rewise::ui::widgets {

class DiffTextWidget final : public QWidget {
    Q_OBJECT
public:
    explicit DiffTextWidget(QWidget* parent = nullptr);

    void setFramed(bool framed);
    bool framed() const { return m_framed; }

    void clear();
    void setReviewResult(const rewise::review::ReviewResult& r);

private:
    static QString renderTokens(const QVector<rewise::review::StyledToken>& tokens, bool isReference);

    bool m_framed = false;
    QTextBrowser* m_view = nullptr;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_DIFFTEXTWIDGET_H
