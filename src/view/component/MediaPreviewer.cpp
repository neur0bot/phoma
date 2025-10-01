#include "MediaPreviewer.h"
#include <QImageReader>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QtConcurrentRun>
#include <model/MediaListModel.h>
#include <view/MediaViewer.h>
#include <utils/Tools.h>

MediaPreviewer::MediaPreviewer(QAbstractItemModel* model, int rowIndex, QWidget* parent)
    : QLabel(parent) {
    filepath = model->data(model->index(rowIndex, MediaListModel::Path)).value<QString>();
    lastModified = model->data(model->index(rowIndex, MediaListModel::LastModifiedTime))
                       .value<QDateTime>();
    isFav = model->data(model->index(rowIndex, MediaListModel::IsFavorite)).value<bool>();
    connect(&imageLoadWatcher,
            &QFutureWatcher<QPixmap*>::finished,
            this,
            &MediaPreviewer::loadImageComplete);
    setScaledContents(true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    initMedia();
    // TODO: open the image in a MediaViewer window when double clicked
}

MediaPreviewer::~MediaPreviewer() {}

void MediaPreviewer::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);
    if (requireReloadImage) {
        imageLoadWatcher.setFuture(QtConcurrent::run(&MediaPreviewer::loadImage, this));
        requireReloadImage = false;
    }
}

QSize MediaPreviewer::sizeHint() const {
    // return mediaSize.scaled(1000, 150, Qt::KeepAspectRatio);
    return mediaSize;
}

void MediaPreviewer::setPath(const QString& path) {
    filepath = path;
    initMedia();
}
void MediaPreviewer::setLastModifiedTime(const QDateTime& time) {
    lastModified = time;
    initMedia();
}

void MediaPreviewer::setIsFavorite(bool isFavorite) {
    isFav = isFavorite;
}

QString MediaPreviewer::path() {
    return filepath;
}

QDateTime MediaPreviewer::lastModifiedTime() {
    return lastModified;
}

bool MediaPreviewer::isFavorite() {
    return isFav;
}

void MediaPreviewer::initMedia() {
    mediaSize = QImageReader(filepath).size();
    requireReloadImage = true;
}

// Rounded pixmap function removed to eliminate image editing functionality

void MediaPreviewer::loadImageComplete() {
    originalPixmap = imageLoadWatcher.result();
    setPixmap(originalPixmap);
}

QPixmap MediaPreviewer::loadImage() {
    // Load and scale thumbnail for preview
    QImageReader reader(filepath);
    QImage img = reader.read();
    if (img.isNull()) return QPixmap();
    QPixmap px = QPixmap::fromImage(img);
    // scale down to mediaSize but keep aspect ratio
    QPixmap scaled = Tools::scaledPixmapKeepingAspect(px, mediaSize);
    return Tools::roundedPixmap(scaled, 6);
}

void MediaPreviewer::enterEvent(QEnterEvent* event) {
    QLabel::enterEvent(event);
    setCursor(Qt::PointingHandCursor);
}

void MediaPreviewer::leaveEvent(QEvent* event) {
    QLabel::leaveEvent(event);
    setCursor(Qt::ArrowCursor);
}

void MediaPreviewer::mousePressEvent(QMouseEvent* event) {
    QLabel::mousePressEvent(event);
    // simple press animation: scale down slightly
    auto* anim = new QPropertyAnimation(this, "geometry");
    anim->setDuration(120);
    QRect start = geometry();
    QRect end = QRect(start.x() + start.width() * 0.02,
                      start.y() + start.height() * 0.02,
                      start.width() * 0.96,
                      start.height() * 0.96);
    anim->setStartValue(start);
    anim->setEndValue(end);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QPropertyAnimation::finished, anim, &QObject::deleteLater);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MediaPreviewer::mouseReleaseEvent(QMouseEvent* event) {
    QLabel::mouseReleaseEvent(event);
    // restore geometry
    auto* anim = new QPropertyAnimation(this, "geometry");
    anim->setDuration(120);
    QRect end = geometry();
    QRect start = QRect(end.x() - end.width() * 0.020408,
                        end.y() - end.height() * 0.020408,
                        static_cast<int>(end.width() / 0.96),
                        static_cast<int>(end.height() / 0.96));
    anim->setStartValue(start);
    anim->setEndValue(end);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    connect(anim, &QPropertyAnimation::finished, anim, &QObject::deleteLater);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void MediaPreviewer::mouseDoubleClickEvent(QMouseEvent* event) {
    QLabel::mouseDoubleClickEvent(event);
    emit doubleClicked();
}

// Image scaling function removed to eliminate image editing functionality

// Scale animation function removed to eliminate image editing functionality
