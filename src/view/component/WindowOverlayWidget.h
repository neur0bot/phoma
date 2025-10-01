#pragma once
#include <QEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>

// from: https://stackoverflow.com/questions/19362455/dark-transparent-layer-over-a-qmainwindow-in-qt/19367454#19367454\
// with some changes for ElaWindow

// show a overlay over whole ElaWindow except ElaAppBar
class WindowOverlayWidget : public QWidget {
public:
    explicit WindowOverlayWidget(QWidget* parent = nullptr);

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;
    bool event(QEvent* ev) override;

private:
    void newParent();
};