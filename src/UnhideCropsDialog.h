
#ifndef UNHIDECROPSDIALOG_H
#define UNHIDECROPSDIALOG_H

#include <QDialog>
#include <QListWidget>

class UnhideCropsDialog : public QDialog {
    Q_OBJECT
public:
    explicit UnhideCropsDialog(QWidget *parent = nullptr);

private slots:
    void unhideSelected();

private:
    QListWidget *listWidget;
    void loadHiddenCrops();
};

#endif // UNHIDECROPSDIALOG_H
