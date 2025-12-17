#include "UnhideCropsDialog.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QSizePolicy>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>


UnhideCropsDialog::UnhideCropsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Unhide Crops");
    // Use window-modal modality (parent window must be passed by the caller)
    // so only the parent/main window is blocked while this dialog is open.
    setWindowModality(Qt::WindowModal);

    // Prefer a larger default size and allow the list to expand to fill space.
    setMinimumSize(480, 360);

    listWidget = new QListWidget(this);
    listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    listWidget->setMinimumHeight(240);
    QPushButton *okButton = new QPushButton("Unhide Selected", this);
    QPushButton *cancelButton = new QPushButton("Cancel", this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(listWidget);
    layout->addWidget(okButton);
    layout->addWidget(cancelButton);

    connect(okButton, &QPushButton::clicked, this, &UnhideCropsDialog::unhideSelected);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    loadHiddenCrops();
}

void UnhideCropsDialog::loadHiddenCrops() {
    QSqlQuery query("SELECT id, name FROM crops WHERE hide = 'true'");
    if (!query.exec()) {
        QMessageBox::critical(this, "DB Error", query.lastError().text());
        return;
    }

    while (query.next()) {
        int id = query.value(0).toInt();
        QString name = query.value(1).toString();

        QListWidgetItem *item = new QListWidgetItem(name, listWidget);
        item->setData(Qt::UserRole, id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }
}

void UnhideCropsDialog::unhideSelected() {
    QSqlQuery updateQuery;
    updateQuery.prepare("UPDATE crops SET hide = 'false' WHERE id = :id");

    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            int id = item->data(Qt::UserRole).toInt();
            updateQuery.bindValue(":id", id);
            if (!updateQuery.exec()) {
                QMessageBox::critical(this, "DB Update Error", updateQuery.lastError().text());
                return;
            }
        }
    }

    accept(); // Close dialog
}
