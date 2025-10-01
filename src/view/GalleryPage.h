#pragma once

#include "BasePage.h"
#include <QAbstractItemModel>

class GalleryPage : public BasePage {
    Q_OBJECT
public:
    explicit GalleryPage(QAbstractItemModel* model, QWidget* parent = nullptr);
    ~GalleryPage();
};
