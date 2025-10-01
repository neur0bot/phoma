#include "WindowOverlayWidget.h"

WindowOverlayWidget::WindowOverlayWidget(QWidget* parent)
    : QWidget{parent} {
    // setAttribute(Qt::WA_NoSystemBackground);
    // setAttribute(Qt::WA_TransparentForMouseEvents);
    newParent();
}

// Catches resize and child events from the parent widget
bool WindowOverlayWidget::eventFilter(QObject* obj, QEvent* ev) {
    if (obj == parent()) {
        if (ev->type() == QEvent::Resize) {
            auto targetSize = dynamic_cast<QResizeEvent*>(ev)->size();
            auto* parentWidget = dynamic_cast<QWidget*>(parent());
            resize(targetSize.width(), targetSize.height() - parentWidget->contentsMargins().top());
        } else if (ev->type() == QEvent::ChildAdded) {
            raise();
        }
    }
    return QWidget::eventFilter(obj, ev);
}

// Tracks parent widget changes
bool WindowOverlayWidget::event(QEvent* ev) {
    if (ev->type() == QEvent::ParentAboutToChange) {
        if (parent() != nullptr) {
            parent()->removeEventFilter(this);
        }
    } else if (ev->type() == QEvent::ParentChange) {
        newParent();
    }
    return QWidget::event(ev);
}

void WindowOverlayWidget::newParent() {
    if (parent() == nullptr) {
        return;
    }
    parent()->installEventFilter(this);
    auto* parentWidget = dynamic_cast<QWidget*>(parent());
    move(0, parentWidget->contentsMargins().top());
    raise();
}
