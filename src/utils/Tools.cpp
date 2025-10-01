#include "Tools.h"
#include <QFileInfo>
#include <QImage>
#include <array>
#include <QPainter>
#include <QPainterPath>

QString Tools::fileSizeString(QString filepath) {
    int index = 0;
    std::array<QString, 4> unit = {"B", "KB", "MB", "GB"};
    double size = static_cast<double>(QFileInfo(filepath).size());
    for (; index < 3; index++) {
        if (size < 1024) {
            break;
        }
        size /= 1024;
        // save one decimal place
        size = static_cast<int>(size * 10) / 10.0;
    }
    return QString("%1 %2").arg(QString::number(size), unit[index]);
}

// Return image bit depth (bits per pixel). Returns 0 for null images.
int Tools::imageBitDepth(const QImage& image) {
    if (image.isNull()) {
        return 0;
    }
    return image.depth();
}

QPixmap Tools::scaledPixmapKeepingAspect(const QPixmap& src, const QSize& targetSize, Qt::TransformationMode mode) {
    if (src.isNull() || targetSize.isEmpty()) return QPixmap();
    return src.scaled(targetSize, Qt::KeepAspectRatio, mode);
}

QPixmap Tools::roundedPixmap(const QPixmap& src, int radius) {
    if (src.isNull()) return QPixmap();
    QPixmap out(src.size());
    out.fill(Qt::transparent);
    QPainter p(&out);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(src.rect(), radius, radius);
    p.setClipPath(path);
    p.drawPixmap(0, 0, src);
    p.end();
    return out;
}