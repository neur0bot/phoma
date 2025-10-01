#pragma once

#include <QLayout>
#include <QTimer>

// ref: https://doc.qt.io/qt-6/qtwidgets-layouts-flowlayout-example.html

// layout media in gallery and favorite

class MediaFlexLayout : public QLayout {
    Q_OBJECT
    QList<QLayoutItem*> itemList;

    struct LayoutData {
        double radio; // width/height
    };

    int preferredLineHeight = 180;

public:
    explicit MediaFlexLayout(QWidget* parent = nullptr);
    ~MediaFlexLayout();

    void setGeometry(const QRect& rect) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    Qt::Orientations expandingDirections() const override;

    void addItem(QLayoutItem* item) override;
    int count() const override;
    QLayoutItem* itemAt(int index) const override;
    QLayoutItem* takeAt(int index) override;

    void addWidgets(const QList<QWidget*>& widgets);
    void insertWidget(QWidget* widget, qsizetype index);
    void insertWidgets(const QList<QWidget*>& widgets, qsizetype index);

    // bool hasHeightForWidth() const override;
    // int heightForWidth(int) const override;

public slots:

    void doLayout();

private:
    int layoutItems(const QRect& rect, bool dryRun = false) const;
    LayoutData itemLayoutData(qsizetype i) const;

    mutable bool requireNewCachedSizeHint = true;
    mutable QSize cachedSizeHint;
};