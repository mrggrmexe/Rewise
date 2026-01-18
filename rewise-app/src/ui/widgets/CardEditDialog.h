#ifndef REWISE_UI_WIDGETS_CARDEDITDIALOG_H
#define REWISE_UI_WIDGETS_CARDEDITDIALOG_H

#include <QDialog>

class QPlainTextEdit;

namespace rewise::ui::widgets {

class CardEditDialog final : public QDialog {
    Q_OBJECT
public:
    explicit CardEditDialog(const QString& title,
                            const QString& initialQuestion,
                            const QString& initialAnswer,
                            QWidget* parent = nullptr);

    QString question() const;
    QString answer() const;

    static bool getCardText(QWidget* parent,
                            const QString& title,
                            const QString& initialQuestion,
                            const QString& initialAnswer,
                            QString* outQuestion,
                            QString* outAnswer);

private:
    void validateNow();

    QPlainTextEdit* m_q = nullptr;
    QPlainTextEdit* m_a = nullptr;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_CARDEDITDIALOG_H
