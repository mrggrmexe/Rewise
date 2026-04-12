#include "FolderCardWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace rewise::ui::widgets {

FolderCardWidget::FolderCardWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("FolderCardWidget");
    setAttribute(Qt::WA_StyledBackground, true);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(12);

    m_title = new QLabel(tr("Новая папка"), this);
    QFont titleFont = m_title->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    m_title->setFont(titleFont);

    m_label = new QLabel(tr("Название папки"), this);
    m_label->setProperty("muted", true);

    m_name = new QLineEdit(this);
    m_name->setPlaceholderText(tr("Например: Английский, История, Math…"));
    m_name->setClearButtonEnabled(true);

    auto* buttons = new QHBoxLayout();
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->setSpacing(10);
    buttons->addStretch(1);

    m_btnCancel = new QPushButton(tr("Отмена"), this);
    m_btnCancel->setProperty("ghost", true);

    m_btnSubmit = new QPushButton(tr("Создать"), this);
    m_btnSubmit->setProperty("primary", true);

    buttons->addWidget(m_btnCancel);
    buttons->addWidget(m_btnSubmit);

    root->addWidget(m_title);
    root->addWidget(m_label);
    root->addWidget(m_name);
    root->addLayout(buttons);

    connect(m_name, &QLineEdit::textChanged, this, [this] { validateNow(); });
    connect(m_name, &QLineEdit::returnPressed, this, [this] {
        if (m_btnSubmit->isEnabled()) emit submitted(folderName());
    });
    connect(m_btnCancel, &QPushButton::clicked, this, &FolderCardWidget::cancelled);
    connect(m_btnSubmit, &QPushButton::clicked, this, [this] {
        if (m_btnSubmit->isEnabled()) emit submitted(folderName());
    });

    validateNow();
}

void FolderCardWidget::setTitle(const QString& title) {
    m_title->setText(title);
}

QString FolderCardWidget::title() const {
    return m_title->text();
}

void FolderCardWidget::setFolderName(const QString& name) {
    m_name->setText(name);
    m_name->selectAll();
    m_name->setFocus();
    validateNow();
}

QString FolderCardWidget::folderName() const {
    return m_name->text().trimmed();
}

void FolderCardWidget::setActionText(const QString& text) {
    m_btnSubmit->setText(text);
}

void FolderCardWidget::setActionEnabled(bool enabled) {
    m_btnSubmit->setEnabled(enabled);
}

void FolderCardWidget::validateNow() {
    m_btnSubmit->setEnabled(!folderName().isEmpty());
}

} // namespace rewise::ui::widgets