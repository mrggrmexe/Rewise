#ifndef REWISE_UI_WIDGETS_FOLDEREDITDIALOG_H
#define REWISE_UI_WIDGETS_FOLDEREDITDIALOG_H

#include <QDialog>

class QLineEdit;

namespace rewise::ui::widgets {

class FolderEditDialog final : public QDialog {
    Q_OBJECT
public:
    explicit FolderEditDialog(const QString& title,
                              const QString& initialName,
                              QWidget* parent = nullptr);

    QString folderName() const;

    // Утилита “в один вызов”
    static bool getFolderName(QWidget* parent,
                              const QString& title,
                              const QString& initialName,
                              QString* outName);

private:
    void validateNow();

    QLineEdit* m_name = nullptr;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_FOLDEREDITDIALOG_H
