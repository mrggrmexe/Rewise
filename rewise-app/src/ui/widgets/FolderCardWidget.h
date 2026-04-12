#ifndef REWISE_UI_WIDGETS_FOLDERCARDWIDGET_H
#define REWISE_UI_WIDGETS_FOLDERCARDWIDGET_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

namespace rewise::ui::widgets {

class FolderCardWidget final : public QWidget {
    Q_OBJECT
public:
    explicit FolderCardWidget(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    QString title() const;

    void setFolderName(const QString& name);
    QString folderName() const;

    void setActionText(const QString& text);
    void setActionEnabled(bool enabled);

signals:
    void submitted(const QString& name);
    void cancelled();

private:
    void validateNow();

    QLabel* m_title = nullptr;
    QLabel* m_label = nullptr;
    QLineEdit* m_name = nullptr;
    QPushButton* m_btnCancel = nullptr;
    QPushButton* m_btnSubmit = nullptr;
};

} // namespace rewise::ui::widgets

#endif // REWISE_UI_WIDGETS_FOLDERCARDWIDGET_H