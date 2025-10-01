#include "Tools.h"
#include <QFileInfo>
#include <QImage>
#include <array>

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

// Image bit depth analysis function removed to eliminate image editing functionality