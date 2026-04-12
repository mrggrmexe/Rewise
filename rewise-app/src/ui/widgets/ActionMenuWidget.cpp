#include "ActionMenuWidget.h"

#include <QPushButton>
#include <QVBoxLayout>

namespace rewise::ui::widgets {

ActionMenuWidget::ActionMenuWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("ActionMenuWidget");
    setAttribute(Qt::WA_StyledBackground, true);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(8);

    m_btnRename = new QPushButton(tr("Переименовать"), this);
    m_btnRename->setProperty("actionMenuItem", true);

    m_btnDelete = new QPushButton(tr("Удалить"), this);
    m_btnDelete->setProperty("actionMenuItem", true);
    m_btnDelete->setProperty("danger", true);

    m_btnCancel = new QPushButton(tr("Отмена"), this);
    m_btnCancel->setProperty("actionMenuItem", true);
    m_btnCancel->setProperty("ghost", true);

    root->addWidget(m_btnRename);
    root->addWidget(m_btnDelete);
    root->addWidget(m_btnCancel);

    connect(m_btnRename, &QPushButton::clicked, this, &ActionMenuWidget::renameRequested);
    connect(m_btnDelete, &QPushButton::clicked, this, &ActionMenuWidget::deleteRequested);
    connect(m_btnCancel, &QPushButton::clicked, this, &ActionMenuWidget::cancelled);
}

void ActionMenuWidget::setActionsEnabled(bool renameEnabled, bool deleteEnabled) {
    m_btnRename->setEnabled(renameEnabled);
    m_btnDelete->setEnabled(deleteEnabled);
}

} // namespace rewise::ui::widgets