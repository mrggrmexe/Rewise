#include "FolderNavButton.h"

#include <QFontMetrics>

namespace rewise::ui::widgets {

FolderNavButton::FolderNavButton(QWidget* parent)
    : QPushButton(parent)
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::StrongFocus);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setMinimumHeight(34);

    setProperty("variant", "nav");
}

void FolderNavButton::setFolder(const rewise::domain::Id& id, const QString& name, bool isAll) {
    m_id = id;
    m_isAll = isAll;

    QString t = name.trimmed();
    if (t.isEmpty()) t = isAll ? QStringLiteral("Все карточки") : QStringLiteral("Без названия");

    const QFontMetrics fm(font());
    t = fm.elidedText(t, Qt::ElideRight, 260);
    setText(t);

    setToolTip(name);
    setProperty("isAll", isAll);
}

} // namespace rewise::ui::widgets
