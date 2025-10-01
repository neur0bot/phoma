#pragma once

#include "BasePage.h"
#include <QAbstractItemModel>

class FavoritePage : public BasePage {
    Q_OBJECT
public:
    explicit FavoritePage(QAbstractItemModel* model, QWidget* parent = nullptr);
    ~FavoritePage();
};
