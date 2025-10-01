#pragma once

#include <QAbstractItemModel>
#include <QDateTime>
#include <QEvent>
#include <QFutureWatcher>
#include <QLabel>
#include <QPixmap>

// display media in thumbnail, supposed to be work with ImageFlexLayout
class MediaPreviewer : public QLabel {
    Q_OBJECT
public:
    explicit MediaPreviewer(QAbstractItemModel* model, int rowIndex, QWidget* parent = nullptr);
    ~MediaPreviewer();

    // load image when show
    void paintEvent(QPaintEvent* event) override;

    QSize sizeHint() const override;

    void setPath(const QString& path);
    void setLastModifiedTime(const QDateTime& time);
    void setIsFavorite(bool isFavorite);
    QString path();
    QDateTime lastModifiedTime();
    bool isFavorite();

signals:
    void doubleClicked();

public slots:
    void loadImageComplete();

private:
    QString filepath;
    QDateTime lastModified;
    bool isFav;

    QSize mediaSize;

    bool requireReloadImage = true;
    QFutureWatcher<QPixmap> imageLoadWatcher;

    QPixmap originalPixmap;

    void initMedia();
    // Removed image editing function
    QPixmap loadImage();

    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    // Image scaling function removed

    // Animation removed
};