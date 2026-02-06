#include "DiffTextWidget.h"

#include <QTextBrowser>
#include <QVBoxLayout>

namespace rewise::ui::widgets {

DiffTextWidget::DiffTextWidget(QWidget* parent)
    : QWidget(parent)
{
    // Reuse the same “card” visuals as the rest of the app.
    setProperty("card", true);

    m_view = new QTextBrowser(this);
    m_view->setOpenExternalLinks(false);
    m_view->setReadOnly(true);
    m_view->setProperty("flat", true);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(0);
    root->addWidget(m_view, 1);

    clear();
}

void DiffTextWidget::clear() {
    m_view->setHtml("<div style='opacity:0.65'>Пока нечего показывать.</div>");
}

static QString tokenSpan(const QString& text, const QString& style) {
    return QString("<span style='%1'>%2</span>").arg(style, text.toHtmlEscaped());
}

QString DiffTextWidget::renderTokens(const QVector<rewise::review::StyledToken>& tokens, bool isReference) {
    // Keep spaces readable in HTML.
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
                    : "background: rgba(60, 180, 90, 0.18); border-radius:4px; padding:1px 3px;";
                break;
            case rewise::review::DiffRole::Removed:
                style = isReference
                    ? "background: rgba(200, 60, 60, 0.16); border-radius:4px; padding:1px 3px; text-decoration: line-through;"
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
        "<div style='white-space: pre-wrap; line-height:1.35;'>"
        "<div style='margin-bottom:10px;'>"
          "<div style='font-weight:700; margin-bottom:4px;'>Эталон</div>"
          "<div>" + ref + "</div>"
        "</div>"
        "<div>"
          "<div style='font-weight:700; margin-bottom:4px;'>Ваш ответ</div>"
          "<div>" + usr + "</div>"
        "</div>"
        "</div>";

    m_view->setHtml(html);
}

} // namespace rewise::ui::widgets
