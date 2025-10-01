#pragma once

#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QMouseEvent>

class ImageViewer : public QGraphicsView {
    Q_OBJECT

public:
    explicit ImageViewer(QWidget* parent = nullptr);
    explicit ImageViewer(const QPixmap& pixmap, QWidget* parent = nullptr);
    void setContent(const QPixmap& pixmap, bool fadeAnimation = true);
    void setContent(const QImage& image, bool fadeAnimation = true);

    [[nodiscard]] int getScale() const;
    [[nodiscard]] int getMinScale() const;
    [[nodiscard]] int getMaxScale() const;
    void setMinScale(int scale);
    void setMaxScale(int scale);
    // Image scaling function removed

signals:
    void wheelScrolled(int delta);
    void resized();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    QGraphicsScene* scene;
    QGraphicsPixmapItem* pixmapItem;
    QPoint lastMousePos;
    bool dragging;
    int cntScale;
    int minScale;
    int maxScale;

    void adjustImageToFit();
};
