#include "ImageViewer.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

ImageViewer::ImageViewer(QWidget* parent)
    : QGraphicsView(parent)
    , scene(new QGraphicsScene(this))
    , pixmapItem(new QGraphicsPixmapItem())
    , dragging(false)
    , cntScale(100)
    , minScale(1)
    , maxScale(800) {
    setScene(scene);
    scene->addItem(pixmapItem);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setBackgroundBrush(Qt::transparent);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setStyleSheet("background-color: transparent;");
    setFrameShape(QFrame::NoFrame);
}

ImageViewer::ImageViewer(const QPixmap& pixmap, QWidget* parent)
    : ImageViewer(parent) {
    setContent(pixmap);
}

void ImageViewer::setContent(const QPixmap& pixmap, bool fadeAnimation) {
    // Fade animation removed to eliminate image editing functionality
    pixmapItem->setPixmap(pixmap);
    adjustImageToFit();
    cntScale = 100;
}

void ImageViewer::setContent(const QImage& image, bool fadeAnimation) {
    setContent(QPixmap::fromImage(image), fadeAnimation);
}

int ImageViewer::getScale() const {
    return cntScale;
}

int ImageViewer::getMinScale() const {
    return minScale;
}

int ImageViewer::getMaxScale() const {
    return maxScale;
}

void ImageViewer::setMinScale(int scale) {
    if (scale < 1) {
        minScale = 1;
    } else if (scale > maxScale) {
        minScale = maxScale;
    }
}

void ImageViewer::setMaxScale(int scale) {
    if (scale < minScale) {
        scale = minScale;
    }
    maxScale = scale;
}

// Image scaling function removed to eliminate image editing functionality

void ImageViewer::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        lastMousePos = event->pos();
        dragging = true;
    }
    QGraphicsView::mousePressEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent* event) {
    if (dragging) {
        QPointF delta = mapToScene(event->pos()) - mapToScene(lastMousePos);
        lastMousePos = event->pos();
        setTransformationAnchor(QGraphicsView::NoAnchor);
        translate(delta.x(), delta.y());
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    }
    QGraphicsView::mouseMoveEvent(event);
}

void ImageViewer::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragging = false;
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void ImageViewer::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    auto viewCenter = mapToScene(viewport()->rect().center());
    int cntScalePercent = cntScale;
    adjustImageToFit();
    centerOn(viewCenter);
    cntScale = 100;
    // Image scaling removed
}

void ImageViewer::wheelEvent(QWheelEvent* event) {
    emit wheelScrolled(event->angleDelta().y());
    event->accept();
}

void ImageViewer::adjustImageToFit() {
    if (!pixmapItem->pixmap().isNull()) {
        setSceneRect(pixmapItem->pixmap().rect());
        fitInView(pixmapItem, Qt::KeepAspectRatio);
    }
}

void ImageViewer::scaleToPercent(int percent) {
    if (percent < minScale) percent = minScale;
    if (percent > maxScale) percent = maxScale;
    // compute factor relative to 100
    qreal factor = percent / 100.0;
    // preserve center
    auto viewCenter = mapToScene(viewport()->rect().center());
    // reset transform and then scale
    resetTransform();
    scale(factor, factor);
    centerOn(viewCenter);
    cntScale = percent;
}

void ImageViewer::rotateBy(qreal degrees) {
    // apply rotation to the pixmap item; keep current transform center
    qreal current = pixmapItem->rotation();
    pixmapItem->setRotation(current + degrees);
    // adjust scene rect and view to account for rotated bounds
    setSceneRect(pixmapItem->boundingRect());
    fitInView(pixmapItem, Qt::KeepAspectRatio);
}
