#pragma once

#include <ElaIconButton.h>
#include <QImage>
#include <QString>
#include <QPixmap>
#include <QSize>

struct Tools {
    Tools() = delete;
    static QString fileSizeString(QString filepath);
    // Return image bit depth (bits per pixel). Returns 0 for null images.
    static int imageBitDepth(const QImage& image);
    // Return a pixmap scaled to fit targetSize while keeping aspect ratio.
    static QPixmap scaledPixmapKeepingAspect(const QPixmap& src, const QSize& targetSize, Qt::TransformationMode mode = Qt::SmoothTransformation);
    // Return a rounded pixmap with the given corner radius.
    static QPixmap roundedPixmap(const QPixmap& src, int radius = 8);
};