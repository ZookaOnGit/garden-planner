#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QDate>
#include <QColor>

struct BedCrop {
    int id{-1};
    QString name;
    int bed{0};
    QDate start;
    int lengthDays{0};
    QColor color{Qt::green};
};

class BedModel : public QObject {
    Q_OBJECT
public:
    explicit BedModel(QObject* parent = nullptr);

    // initialize with N empty beds
    void loadDefaults(int bedCount = 10);
    // Load crops from the database table (creates table if needed)
    void loadFromDatabase();

    const QVector<BedCrop>& crops() const { return m_crops; }
    QVector<BedCrop>& crops() { return m_crops; }

    void addCrop(const BedCrop& c);
    void updateCropById(int id, const BedCrop& c);
    void removeCropById(int id);
    // Remove all crops from model and DB
    void clearAllCrops();

    int nextId();

signals:
    void modelChanged();

private:
    QVector<BedCrop> m_crops;
    int m_nextId{1};
};
