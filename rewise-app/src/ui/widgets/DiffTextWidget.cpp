#include "DiffTextWidget.h"

#include <QFrame>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QStyle>

namespace rewise::ui::widgets {

DiffTextWidget::DiffTextWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setFramed(false);

    m_view = new QTextBrowser(this);
    m_view->setOpenExternalLinks(false);
    m_view->setReadOnly(true);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setProperty("flat", true);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(m_view);

    clear();
}

void DiffTextWidget::setFramed(bool framed) {
    if (m_framed == framed) return;
    m_framed = framed;

    setProperty("framed", framed);
    style()->unpolish(this);
    style()->polish(this);
    update();
}

void DiffTextWidget::clear() {
    m_view->setHtml("<div style='opacity:0.65'>Пока нечего показывать.</div>");
}

static QString tokenSpan(const QString& text, const QString& style) {
    return QString("<span style='%1'>%2</span>").arg(style, text.toHtmlEscaped());
}

QString DiffTextWidget::renderTokens(const QVector<rewise::review::StyledToken>& tokens, bool isReference) {
    QString out;
    out.reserve(tokens.size() * 16);

    for (int i = 0; i < tokens.size(); ++i) {
        const auto& t = tokens[i];

        QString style;
        switch (t.role) {
            case rewise::review::DiffRole::Common:
                style = "";
                break;
            case rewise::review::DiffRole::Added:
                style = isReference
                            ? "opacity:0.35;"
                            : "background: rgba(60, 180, 90, 0.18); border-radius:6px; padding:1px 4px;";
                break;
            case rewise::review::DiffRole::Removed:
                style = isReference
                            ? "background: rgba(200, 60, 60, 0.16); border-radius:6px; padding:1px 4px; text-decoration: line-through;"
                            : "opacity:0.35;";
                break;
        }

        out += tokenSpan(t.text, style);
        if (i + 1 < tokens.size()) out += " ";
    }
    return out;
}

void DiffTextWidget::setReviewResult(const rewise::review::ReviewResult& r) {
    const QString ref = renderTokens(r.diff.reference, true);
    const QString usr = renderTokens(r.diff.user, false);

    const QString html =
        "<div style='white-space: pre-wrap; line-height:1.38;'>"
        "<div style='margin-bottom:12px;'>"
          "<div style='font-weight:600; margin-bottom:6px;'>Эталон</div>"
          "<div>" + ref + "</div>"
        "</div>"
        "<div>"
          "<div style='font-weight:600; margin-bottom:6px;'>Ваш ответ</div>"
          "<div>" + usr + "</div>"
        "</div>"
        "</div>";

    m_view->setHtml(html);
}

} // namespace rewise::ui::widgets
