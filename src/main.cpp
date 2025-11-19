
#include <QApplication>
#include <QScrollArea>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QMainWindow>

#include "GanttChartWidget.h"
#include "LeftColumnWidget.h"
#include "DataLoader.h"
#include "CropEditDialog.h"
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSplitter>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <memory>
#include "SettingsManager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
        
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("garden.db");
    if (!db.open()) {
        QMessageBox::critical(nullptr, "DB Error", db.lastError().text());
        return 1;
    }

    // Create crops table if it doesn't exist
    QSqlQuery createTableQuery;
    if (!createTableQuery.exec("CREATE TABLE IF NOT EXISTS crops ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                              "name TEXT NOT NULL,"
                              "sow_start TEXT,"
                              "sow_end TEXT,"
                              "plant_start TEXT,"
                              "plant_end TEXT,"
                              "harvest_start TEXT,"
                              "harvest_end TEXT"
                              "notes TEXT"
                              ")")) {
        QMessageBox::critical(nullptr, "DB Error", 
                            QString("Failed to create crops table: %1").arg(createTableQuery.lastError().text()));
        return 1;
    }

    QSqlQuery query("SELECT id, name, sow_start, sow_end, plant_start, plant_end, harvest_start, harvest_end, notes FROM crops");
    if (!query.isActive()) {
        QMessageBox::critical(nullptr, "Query Error", query.lastError().text());
        return 1;
    }

    auto items = loadCropsFromQuery(query);

    // Create left column (frozen) and right timeline (scrollable) and
    // synchronize vertical scrolling.
    auto* left = new LeftColumnWidget;
    // Keep items in a shared container so lambdas can refresh it easily.
    auto itemsPtr = std::make_shared<QVector<CropWindow>>(items);
    left->setItems(*itemsPtr);
    // Example: reduce the left column width so it takes less horizontal space.
    // You can change this value or call left->setColumnWidth(...) from elsewhere.
    left->setColumnWidth(300);

    auto* chart = new GanttChartWidget;
    chart->setItems(items);
    chart->setDayWidth(4);
    chart->showTodayLine(true);
    chart->showSubrowLabels(true);
    // Keep the chart's internal left margin in sync with the left column width
    // so labels and grid lines align correctly.
    chart->setLeftMargin(300);

    // Left column scroller (we'll sync vertical scroll with the timeline)
    // The left column widget now draws its own small '+' add button in
    // its header, so we can put the widget itself into the scroller.
    auto* leftScroller = new QScrollArea;
    leftScroller->setWidget(left);
    leftScroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leftScroller->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    leftScroller->setWidgetResizable(false);
    left->setMinimumSize(left->sizeHint());

    // Timeline scroller
    auto* scroller = new QScrollArea;
    scroller->setWidget(chart);
    scroller->setWidgetResizable(false);
    chart->setMinimumSize(chart->sizeHint());
    scroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroller->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // Use a QSplitter so the user can resize the left column interactively.
    QSplitter* splitter = new QSplitter;
    splitter->addWidget(leftScroller);
    splitter->addWidget(scroller);
    splitter->setChildrenCollapsible(false);

    // Set initial sizes: left based on content, right gets the rest.
    //int leftPref = left->preferredWidth();
    //QList<int> sizes;
    //sizes << leftPref << 1000;
    //splitter->setSizes(sizes);

    // When the splitter is moved, update both widgets' left width so
    // labels/grid remain aligned.
    QObject::connect(splitter, &QSplitter::splitterMoved, [left, chart](int pos, int){
        // pos is the position of the splitter handle measured from the left of the splitter
        left->setColumnWidth(pos);
        chart->setLeftMargin(pos);
    });

    // Sync vertical scrolling both ways so the two views never get out of
    // sync. Use guarded lambdas to avoid feedback loops (only set the value
    // when different).
    QScrollBar* leftV = leftScroller->verticalScrollBar();
    QScrollBar* rightV = scroller->verticalScrollBar();
    QObject::connect(rightV, &QScrollBar::valueChanged, leftV, [leftV](int v){
        if (leftV->value() != v) leftV->setValue(v);
    });
    QObject::connect(leftV, &QScrollBar::valueChanged, rightV, [rightV](int v){
        if (rightV->value() != v) rightV->setValue(v);
    });

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Garden Planner");
    mainWindow.setCentralWidget(splitter);
    mainWindow.resize(1100, 600);
    // TODO: not working
    //SettingsManager::instance().loadMainWindowGeometry(&mainWindow);
    //SettingsManager::instance().loadMainSplitterState(splitter);
    mainWindow.show();



    // Helper to reload the crops from DB and update widgets
    auto reload = [itemsPtr, left, chart]() {
    QSqlQuery q("SELECT id, name, sow_start, sow_end, plant_start, plant_end, harvest_start, harvest_end, notes FROM crops");
        if (!q.isActive()) return false;
        *itemsPtr = loadCropsFromQuery(q);
        left->setItems(*itemsPtr);
        chart->setItems(*itemsPtr);
        return true;
    };

    // Add crop request (from left column's small + button) -> open dialog for new crop
    QObject::connect(left, &LeftColumnWidget::addCropRequested, [itemsPtr, left, chart, &reload]() {
        CropEditDialog dlg;
        if (dlg.exec() == QDialog::Accepted) {
            CropWindow c = dlg.crop();
            QSqlQuery ins;
            ins.prepare("INSERT INTO crops(name, sow_start, sow_end, plant_start, plant_end, harvest_start, harvest_end, notes)"
                        " VALUES(:name, :sow_start, :sow_end, :plant_start, :plant_end, :harvest_start, :harvest_end, :notes)");
            ins.bindValue(":name", c.name);
            ins.bindValue(":notes", c.notes);
            auto bindDateOrNull = [&](const QDate& d){ return d.isValid() ? QVariant(d.toString("yyyy-MM-dd")) : QVariant(); };
            ins.bindValue(":sow_start", bindDateOrNull(c.sowStart));
            ins.bindValue(":sow_end",   bindDateOrNull(c.sowEnd));
            ins.bindValue(":plant_start", bindDateOrNull(c.plantStart));
            ins.bindValue(":plant_end",   bindDateOrNull(c.plantEnd));
            ins.bindValue(":harvest_start", bindDateOrNull(c.harvestStart));
            ins.bindValue(":harvest_end",   bindDateOrNull(c.harvestEnd));
            if (!ins.exec()) {
                QMessageBox::critical(nullptr, "DB Insert Error", ins.lastError().text());
                return;
            }
            // refresh
            reload();
        }
    });

    // Double-click a crop in the left column to edit it
    QObject::connect(left, &LeftColumnWidget::cropDoubleClicked, [itemsPtr, left, chart, &reload](int idx){
        if (idx < 0 || idx >= itemsPtr->size()) return;
        CropWindow current = (*itemsPtr)[idx];
        CropEditDialog dlg;
    dlg.setCrop(current);
    dlg.setOriginalId(current.id);
        if (dlg.exec() == QDialog::Accepted) {
            CropWindow c = dlg.crop();
            QSqlQuery up;
            up.prepare("UPDATE crops SET name = :name, sow_start = :sow_start, sow_end = :sow_end,"
                       " plant_start = :plant_start, plant_end = :plant_end,"
                       " harvest_start = :harvest_start, harvest_end = :harvest_end, notes = :notes"
                       " WHERE id = :id");
            up.bindValue(":name", c.name);
            up.bindValue(":notes", c.notes);
            auto bindDateOrNull = [&](const QDate& d){ return d.isValid() ? QVariant(d.toString("yyyy-MM-dd")) : QVariant(); };
            up.bindValue(":sow_start", bindDateOrNull(c.sowStart));
            up.bindValue(":sow_end",   bindDateOrNull(c.sowEnd));
            up.bindValue(":plant_start", bindDateOrNull(c.plantStart));
            up.bindValue(":plant_end",   bindDateOrNull(c.plantEnd));
            up.bindValue(":harvest_start", bindDateOrNull(c.harvestStart));
            up.bindValue(":harvest_end",   bindDateOrNull(c.harvestEnd));
            up.bindValue(":id", current.id);
            if (!up.exec()) {
                QMessageBox::critical(nullptr, "DB Update Error", up.lastError().text());
                return;
            }
            // refresh
            reload();
        }
    });

    // Right-click -> Delete handler
    QObject::connect(left, &LeftColumnWidget::cropDeleteRequested, [itemsPtr, left, chart, &reload](int idx){
        if (idx < 0 || idx >= itemsPtr->size()) return;
        CropWindow current = (*itemsPtr)[idx];
        auto res = QMessageBox::question(nullptr, "Delete crop",
                                         QString("Delete '%1'?").arg(current.name),
                                         QMessageBox::Yes | QMessageBox::No);
        if (res != QMessageBox::Yes) return;
        QSqlQuery del;
        del.prepare("DELETE FROM crops WHERE id = :id");
        del.bindValue(":id", current.id);
        if (!del.exec()) {
            QMessageBox::critical(nullptr, "DB Delete Error", del.lastError().text());
            return;
        }
        reload();
    });

    // After the UI is shown, scroll the timeline so today's date is centered
    QTimer::singleShot(0, [scroller, leftScroller, splitter, left, chart]() {
        int tx = chart->xForDate(QDate::currentDate());
        // Left-align today's date with a 50px left padding
        const int padding = 50;
        int target = tx - padding;
        QScrollBar* h = scroller->horizontalScrollBar();
        if (target < h->minimum()) target = h->minimum();
        if (target > h->maximum()) target = h->maximum();
        h->setValue(target);

        const int leftPref = left->preferredWidth();  // your existing API
        // Ensure the left pane can't be shrunk below the intended label width.
        left->setColumnWidth(leftPref);
        left->setMinimumWidth(leftPref);
        leftScroller->setMinimumWidth(leftPref);

        // Keep chart aligned with the left pane.
        chart->setLeftMargin(leftPref);

        // Give the right pane the rest of the splitter width.
        const int total = splitter->width();
        const int rightPref = std::max(100, total - leftPref);
        splitter->setSizes({ leftPref, rightPref });

        // Optional: make the right side stretch, left side fixed.
        splitter->setStretchFactor(0, 0); // left
        splitter->setStretchFactor(1, 1); // right        
    });

    QObject::connect(&app, &QApplication::aboutToQuit, [&]() {
        SettingsManager::instance().saveMainWindowGeometry(&mainWindow);
        SettingsManager::instance().saveMainSplitterState(splitter);
    });

    return app.exec();
}
