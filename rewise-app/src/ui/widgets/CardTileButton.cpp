#include "CardTileButton.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QFontMetrics>

namespace rewise::ui::widgets {

static void makeChildTransparent(QWidget* w) {
    if (!w) return;
    w->setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

CardTileButton::CardTileButton(QWidget* parent)
    : QPushButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(64);

    setProperty("variant", "card");

    // We don't use QPushButton text drawing; we render via labels inside.
    setText(QString());

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 10, 12, 10);
    root->setSpacing(4);

    m_title = new QLabel(this);
    m_title->setObjectName("cardTitle");
    m_title->setTextInteractionFlags(Qt::NoTextInteraction);
    m_title->setWordWrap(false);

    m_preview = new QLabel(this);
    m_preview->setObjectName("cardPreview");
    m_preview->setTextInteractionFlags(Qt::NoTextInteraction);
    m_preview->setWordWrap(true);

    m_meta = new QLabel(this);
    m_meta->setObjectName("cardMeta");
    m_meta->setTextInteractionFlags(Qt::NoTextInteraction);
    m_meta->setWordWrap(false);

    makeChildTransparent(m_title);
    makeChildTransparent(m_preview);
    makeChildTransparent(m_meta);

    root->addWidget(m_title);
    root->addWidget(m_preview);
    root->addWidget(m_meta);

    m_meta->setVisible(false);
}

QString CardTileButton::makePreview(const QString& s, int maxChars) {
    QString t = s;
    t.replace('\n', ' ');
    t = t.simplified();
    if (t.size() <= maxChars) return t;
    return t.left(maxChars - 1) + "…";
}

void CardTileButton::setCard(const rewise::domain::Id& cardId,
                             const QString& question,
                             const QString& answer,
                             const QString& metaLine)
{
    m_id = cardId;

    QString q = question.simplified();
    if (q.isEmpty()) q = QStringLiteral("Без вопроса");

    QString a = answer;
    if (a.trimmed().isEmpty()) a = QStringLiteral("—");

    {
        const QFontMetrics fm(m_title->font());
        m_title->setText(fm.elidedText(q, Qt::ElideRight, 800));
        m_title->setToolTip(question);
    }

    m_preview->setText(makePreview(a, 220));
    m_preview->setToolTip(answer);

    if (!metaLine.trimmed().isEmpty()) {
        const QFontMetrics fm(m_meta->font());
        m_meta->setText(fm.elidedText(metaLine, Qt::ElideRight, 800));
        m_meta->setVisible(true);
    } else {
        m_meta->clear();
        m_meta->setVisible(false);
    }
}

} // namespace rewise::ui::widgets
