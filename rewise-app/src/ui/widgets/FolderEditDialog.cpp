#include "FolderEditDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QVBoxLayout>

namespace rewise::ui::widgets {

FolderEditDialog::FolderEditDialog(const QString& title,
                                   const QString& initialName,
                                   QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);

    m_name = new QLineEdit(this);
    m_name->setText(initialName);
    m_name->setPlaceholderText("Например: Английский, История…");

    auto* form = new QFormLayout();
    form->addRow("Название папки:", m_name);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_name, &QLineEdit::textChanged, this, [this] { validateNow(); });

    validateNow();
    m_name->selectAll();
    m_name->setFocus();
}

void FolderEditDialog::validateNow() {
    const bool ok = !m_name->text().trimmed().isEmpty();
    auto* box = findChild<QDialogButtonBox*>();
    if (box) box->button(QDialogButtonBox::Ok)->setEnabled(ok);
}

QString FolderEditDialog::folderName() const {
    return m_name->text().trimmed();
}

bool FolderEditDialog::getFolderName(QWidget* parent,
                                    const QString& title,
                                    const QString& initialName,
                                    QString* outName)
{
    FolderEditDialog dlg(title, initialName, parent);
    if (dlg.exec() != QDialog::Accepted) return false;
    const QString name = dlg.folderName();
    if (name.isEmpty()) return false;
    if (outName) *outName = name;
    return true;
}

} // namespace rewise::ui::widgets
