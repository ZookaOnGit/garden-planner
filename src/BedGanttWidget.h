#pragma once

#include <QWidget>
#include <QDate>
#include <QVector>
#include "BedModel.h"

class BedGanttWidget : public QWidget {
    Q_OBJECT
public:
    explicit BedGanttWidget(QWidget* parent = nullptr);

    void setModel(BedModel* model);
    void setDayWidth(int w) { m_dayWidth = w; updateGeometry(); update(); }
    void setDefaultTemplate(const QString& name, int lengthDays) { m_templateName = name; m_templateLength = lengthDays; }
    void zoomByFactor(double f, int centerX);
    void setRowCount(int rows) { m_rows = qMax(1, rows); updateGeometry(); update(); }

    QSize sizeHint() const override { return QSize(1200, m_rowHeight * m_rows + 40); }

    int xForDate(const QDate& d) const;

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* evt) override;
    void mouseMoveEvent(QMouseEvent* evt) override;
    void mouseReleaseEvent(QMouseEvent* evt) override;
    void mouseDoubleClickEvent(QMouseEvent* evt) override;
    void wheelEvent(QWheelEvent* evt) override;

protected:
    void panBy(int dx);

private:
    enum ResizeSide { None, Left, Right };
    bool m_panning{false};
    QPoint m_panStart;
    int m_resizingId{-1};
    ResizeSide m_resizingSide{None};
    int m_selectedId{-1};

private:
    BedModel* m_model{nullptr};
    int m_dayWidth{6};
    int m_rowHeight{40};
    int m_leftMargin{80};
    int m_rows{10};
    QDate m_refDate{QDate::currentDate().addDays(-30)}; // left-most date

    // dragging state
    int m_draggingId{-1};
    QPoint m_dragStartPos;
    int m_originalBed{0};
    QDate m_originalStart;
    QString m_templateName;
    int m_templateLength{30};
};
