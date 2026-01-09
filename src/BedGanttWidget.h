#pragma once

#include <QWidget>
#include <QDate>
#include <QVector>
#include "BedModel.h"

class BedGanttWidget : public QWidget {
    Q_OBJECT
public:
    explicit BedGanttWidget(QWidget* parent = nullptr);

    // expose a few view properties for the header widget
    QDate refDate() const { return m_refDate; }
    int dayWidth() const { return m_dayWidth; }
    int leftMargin() const { return m_leftMargin; }
    // expose row geometry so external widgets (labels/header) can align
    int rowHeight() const { return m_rowHeight; }
    int rowCount() const { return m_rows; }

    void setModel(BedModel* model);
    void setDayWidth(int w) { m_dayWidth = w; updateGeometry(); update(); emit viewChanged(); }
    void setDefaultTemplate(const QString& name, int lengthDays) { m_templateName = name; m_templateLength = lengthDays; }
    void zoomByFactor(double f, int centerX);
    void setRowCount(int rows) { m_rows = qMax(1, rows); updateGeometry(); update(); emit viewChanged(); }
    // public facade to pan by pixels (used by header)
    void panByPixels(int dx) { panBy(dx); }

    // expose model for read-only inspection by sibling widgets
    BedModel* model() const { return m_model; }

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

signals:
    void viewChanged();

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
