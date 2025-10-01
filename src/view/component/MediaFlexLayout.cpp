#include "MediaFlexLayout.h"
#include <QWidget>
#include <cstdlib>

MediaFlexLayout::MediaFlexLayout(QWidget* parent)
    : QLayout(parent) {}

MediaFlexLayout::~MediaFlexLayout() {
    QLayoutItem* item;
    while ((item = takeAt(0))) {
        delete item;
    }
}

void MediaFlexLayout::setGeometry(const QRect& rect) {
    QLayout::setGeometry(rect);
    auto height = layoutItems(rect);

    // update cache
    cachedSizeHint = parentWidget()->size();
    cachedSizeHint.setHeight(height);
}

QSize MediaFlexLayout::sizeHint() const {
    return minimumSize();
}

QSize MediaFlexLayout::minimumSize() const {
    if (requireNewCachedSizeHint) {
        requireNewCachedSizeHint = false;
        cachedSizeHint = parentWidget()->size();
        cachedSizeHint.setHeight(layoutItems({0, 0, cachedSizeHint.width(), 0}, true));
    }
    return cachedSizeHint;
}

Qt::Orientations MediaFlexLayout::expandingDirections() const {
    return Qt::Horizontal | Qt::Vertical;
}

void MediaFlexLayout::addItem(QLayoutItem* item) {
    itemList.append(item);
    doLayout();
}

int MediaFlexLayout::count() const {
    return itemList.size();
}

QLayoutItem* MediaFlexLayout::itemAt(int index) const {
    return itemList.value(index);
}

QLayoutItem* MediaFlexLayout::takeAt(int index) {
    if (index >= 0 && index < itemList.size()) {
        auto&& res = itemList.takeAt(index);
        layoutItems(geometry());
        return res;
    }
    return nullptr;
}

void MediaFlexLayout::addWidgets(const QList<QWidget*>& widgets) {
    for (auto* widget : widgets) {
        addChildWidget(widget);
        itemList.emplace_back(new QWidgetItemV2(widget));
    }
    doLayout();
}

void MediaFlexLayout::insertWidget(QWidget* widget, qsizetype index) {
    addChildWidget(widget);
    itemList.insert(index, new QWidgetItemV2(widget));
    doLayout();
}

void MediaFlexLayout::insertWidgets(const QList<QWidget*>& widgets, qsizetype index) {
    for (auto* widget : widgets) {
        addChildWidget(widget);
        itemList.insert(index, new QWidgetItemV2(widget));
        index++;
    }
    doLayout();
}

// bool MediaFlexLayout::hasHeightForWidth() const {
//     return true;
// }

// int MediaFlexLayout::heightForWidth(int width) const {
//     return layoutItems({0, 0, width, 0}, true);
// }

void MediaFlexLayout::doLayout() {
    layoutItems(geometry());
}

int MediaFlexLayout::layoutItems(const QRect& rect, bool dryRun) const {
    // use effectiveRect in account of margin
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);

    if (itemList.size() == 0) {
        return top + bottom;
    }

    int startItem = 0;
    int endItem = -1;
    int y = effectiveRect.y();

    // set oneline onetime
    while (startItem < itemList.size()) {
        // find start and end
        double lineHeight = 1e9;
        double previousLineHeight = 1e9;
        double sumRatio = 0;
        while (abs(lineHeight - preferredLineHeight) <= abs(previousLineHeight - preferredLineHeight)
               || lineHeight == 0) {
            endItem++;
            previousLineHeight = lineHeight;
            if (endItem >= itemList.size()) {
                break;
            }
            sumRatio += itemLayoutData(endItem).radio;
            lineHeight = (effectiveRect.width() - spacing() * (endItem - startItem)) / sumRatio;
        }
        lineHeight = previousLineHeight;
        endItem--;
        // now have startItem endItem lineHeight

        if (endItem < startItem) {
            endItem = startItem;
            lineHeight = (effectiveRect.width() - spacing() * (endItem - startItem))
                         / itemLayoutData(endItem).radio;
        }
        if (endItem == itemList.size() - 1 && lineHeight > preferredLineHeight) {
            lineHeight = preferredLineHeight;
        }

        // set widgets
        int x = effectiveRect.x();
        for (int i = startItem; i <= endItem; i++) {
            auto widget = itemList.value(i)->widget();
            auto width = itemLayoutData(i).radio * lineHeight;
            if (!dryRun) {
                widget->setGeometry(x, y, width, lineHeight);
            }
            x += width + spacing();
        }
        y += lineHeight + spacing();
        startItem = endItem + 1;
    }

    // return content height for other function
    return y - spacing() + bottom - rect.top();
}

MediaFlexLayout::LayoutData MediaFlexLayout::itemLayoutData(qsizetype i) const {
    auto itemSize = itemList.at(i)->sizeHint();
    return {.radio = double(itemSize.width()) / double(itemSize.height())};
}
