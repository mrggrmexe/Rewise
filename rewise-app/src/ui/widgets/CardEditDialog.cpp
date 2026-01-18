#include "CardEditDialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QPushButton>


namespace rewise::ui::widgets {

CardEditDialog::CardEditDialog(const QString& title,
                               const QString& initialQuestion,
                               const QString& initialAnswer,
                               QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);
    resize(540, 420);

    m_q = new QPlainTextEdit(this);
    m_q->setPlainText(initialQuestion);
    m_q->setPlaceholderText("Вопрос…");
    m_q->setTabChangesFocus(true);

    m_a = new QPlainTextEdit(this);
    m_a->setPlainText(initialAnswer);
    m_a->setPlaceholderText("Эталонный ответ…");
    m_a->setTabChangesFocus(true);

    auto* form = new QFormLayout();
    form->addRow("Вопрос:", m_q);
    form->addRow("Ответ:", m_a);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_q, &QPlainTextEdit::textChanged, this, [this] { validateNow(); });
    connect(m_a, &QPlainTextEdit::textChanged, this, [this] { validateNow(); });

    validateNow();
    m_q->setFocus();
}

QString CardEditDialog::question() const {
    return m_q->toPlainText().trimmed();
}

QString CardEditDialog::answer() const {
    return m_a->toPlainText().trimmed();
}

void CardEditDialog::validateNow() {
    const bool ok = !question().isEmpty() && !answer().isEmpty();
    auto* box = findChild<QDialogButtonBox*>();
    if (box) box->button(QDialogButtonBox::Ok)->setEnabled(ok);
}

bool CardEditDialog::getCardText(QWidget* parent,
                                const QString& title,
                                const QString& initialQuestion,
                                const QString& initialAnswer,
                                QString* outQuestion,
                                QString* outAnswer)
{
    CardEditDialog dlg(title, initialQuestion, initialAnswer, parent);
    if (dlg.exec() != QDialog::Accepted) return false;

    const QString q = dlg.question();
    const QString a = dlg.answer();
    if (q.isEmpty() || a.isEmpty()) return false;

    if (outQuestion) *outQuestion = q;
    if (outAnswer) *outAnswer = a;
    return true;
}

} // namespace rewise::ui::widgets
