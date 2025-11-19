#pragma once
#include "CropWindow.h"
#include <QWidget>
#include <QVector>
#include <QDate>
#include <QMenu>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QPainter>
#include <QGuiApplication>
#include <QImageWriter>
#include <QScrollArea>


class QPainter;

class GanttChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit GanttChartWidget(QWidget* parent = nullptr);

    void setItems(QVector<CropWindow> items);
    void setDayWidth(int pixelsPerDay);
    void showTodayLine(bool on);
    void showSubrowLabels(bool on);
    void setLeftMargin(int pixels);
    void setTopMargin(int pixels);
    int topMargin() const;

    // Return x pixel position for a given date in the chart's content coords
    int xForDate(const QDate& d) const;

    QSize sizeHint() const override;

    // Tooltip support
    void mouseMoveEvent(QMouseEvent* ev) override;
    void leaveEvent(QEvent* ev) override;
protected:
    void paintEvent(QPaintEvent*) override;

private:
    QVector<CropWindow> m_items;
    QDate m_minDate;
    QDate m_maxDate;

    // Layout
    int m_leftMargin  = 160;
    int m_topMargin   = 40;
    int m_rightMargin = 24;

    // 3 lanes per crop
    int m_subRowHeight = 20;
    int m_subRowGap    = 4;
    int m_blockGap     = 10;

    int m_dayWidth   = 4;
    bool m_showToday = true;
    bool m_showSubLabels = true;
    int m_leftWidthOverride = 0; // if >0, use this instead of m_leftMargin for layout
    // last hovered bar (index, lane), (-1,-1) when none
    QPair<int,int> m_lastHover = QPair<int,int>(-1,-1);

    int totalDays() const;
    int dateToX(const QDate& d) const; // content X

    void computeRange();
    void drawHeader(QPainter& p);
    void drawGrid(QPainter& p);
    void drawWeekLines(QPainter& p);
    void drawBars(QPainter& p);

    static bool isValidRange(const QDate& a, const QDate& b);

    int blockHeight() const;
    int laneTopY(int cropIndex, int laneIndex) const;
};
