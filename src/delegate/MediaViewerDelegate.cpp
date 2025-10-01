#include "MediaViewerDelegate.h"
#include <ElaContentDialog.h>
#include <ElaMessageBar.h>
#include <ElaText.h>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QFile>
#include <QFileDialog>
#include <QGuiApplication>
#include <QImageReader>
#include <QPaintDevice>
#include <QScreen>
#include <QtConcurrent>
#include <model/MediaListModel.h>
#include <utils/Settings.hpp>
#include <utils/Tools.h>
#include <view/MediaViewer.h>

MediaViewerDelegate::MediaViewerDelegate(QAbstractItemModel* model,
                                         int index,
                                         MediaViewer* viewer,
                                         QObject* parent)
    : QObject(parent)
    , mediaListModel(model)
    , mediaIndex(model->index(index, MediaListModel::Path))
    , view(viewer) {
    filepath = mediaIndex.data().value<QString>();
    loadImage(filepath);
}

void MediaViewerDelegate::initConnections() {
    // connect to actions
    connect(mediaListModel,
            &QAbstractItemModel::rowsAboutToBeRemoved,
            this,
            &MediaViewerDelegate::onModelRowsToBeRemoved);

    connect(this, &MediaViewerDelegate::imageChanged, this, &MediaViewerDelegate::onImageChanged);

    connect(view->openFileAction, &QAction::triggered, this, [this]() {
        openImageFileDialog();
        if (this->image.isNull()) {
            ElaMessageBar::error(ElaMessageBarType::PositionPolicy::TopRight,
                                 "Error",
                                 "Image failed to load",
                                 3000,
                                 view->imageViewer);
        }
        loadImage(this->image);
        QScreen* screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int screenWidth = screenGeometry.width();
        int screenHeight = screenGeometry.height();

        int windowWidth = screenWidth / 2;
        int windowHeight = screenHeight / 2;
        view->resize(windowWidth, windowHeight);

        // if you wanna adapt to your monitor resolution, use adaptiveResize();
    });
    connect(view->copyFileAction,
            &QAction::triggered,
            this,
            &MediaViewerDelegate::copyImageToClipboard);
    connect(view->saveFileAction,
            &QAction::triggered,
            this,
            &MediaViewerDelegate::saveImageFileDialog);

    //TODO(must): implement the openInFileExplorer functionality
    //connect(openInFileExplorerAction,......)

    // Image rotation functionality removed

    connect(view->deleteAction, &QAction::triggered, this, &MediaViewerDelegate::deleteImage);

    connect(view->printAction, &QAction::triggered, this, [=]() {
        qDebug() << "Print action clicked";
    });

    connect(view->editAction, &QAction::triggered, this, [=]() {
        //TODO(optional): implement the edit functionality
    });

    connect(view->prevAction, &QAction::triggered, this, &MediaViewerDelegate::prevImage);

    connect(view->nextAction, &QAction::triggered, this, &MediaViewerDelegate::nextImage);

    // Rotation
    connect(view->rotateAction, &QAction::triggered, this, &MediaViewerDelegate::rotateImage);

    connect(view->likeButton, &ElaIconButton::clicked, this, [=]() {
        //TODO(must): implement the like functionality
        // add the image to Favorite Page
    });

    connect(view->fileInfoButton,
        &ElaIconButton::clicked,
        this,
        &MediaViewerDelegate::onFileInfoClicked);

    // Zoom: connect buttons and slider
    connect(view->zoomInButton, &ElaIconButton::clicked, this, &MediaViewerDelegate::zoomIn);
    connect(view->zoomOutButton, &ElaIconButton::clicked, this, &MediaViewerDelegate::zoomOut);
    connect(view->zoom2originalButton, &ElaIconButton::clicked, this, &MediaViewerDelegate::zoomToOriginal);
    connect(view->zoomSlider, &QSlider::valueChanged, this, &MediaViewerDelegate::scaleTo);

    // Zoom removed

    // Zoom functionality removed

    connect(view->maximizeButton, &ElaIconButton::clicked, [this]() {
        this->view->showMaximized();
    });

    // Zoom functionality removed

    connect(this, &MediaViewerDelegate::scaledByWheel, [this]() {
        view->zoomSlider->setValue(getScale());
    });

    connect(view->imageViewer,
            &ImageViewer::wheelScrolled,
            this,
            &MediaViewerDelegate::onWheelScrolled);
}

void MediaViewerDelegate::onModelRowsToBeRemoved(const QModelIndex& parent, int first, int last) {
    if (mediaIndex.row() >= first && mediaIndex.row() <= last) {
        if (last < mediaListModel->rowCount() - 1) {
            mediaIndex = mediaListModel->index(last + 1, MediaListModel::Path);
        } else if (first > 0) {
            mediaIndex = mediaListModel->index(first - 1, MediaListModel::Path);
        } else {
            view->close();
            return;
        }
        filepath = mediaIndex.data().value<QString>();
        loadImage(filepath);
    }
}

void MediaViewerDelegate::onImageChanged(bool fadeAnimation) {
    view->imageViewer->setContent(image, fadeAnimation);
    view->fileInfoBriefText->setText(QString("%1 x %2 %3")
                                         .arg(QString::number(image.width()))
                                         .arg(QString::number(QImage(image).height()))
                                         .arg(Tools::fileSizeString(filepath)));
    view->setWindowTitle(QFileInfo(filepath).fileName());
    view->zoomSlider->setValue(view->imageViewer->getScale());
    view->fileInfoWidget->loadInfo(filepath);
}

void MediaViewerDelegate::onWheelScrolled(int delta) {
    if (settings.value("wheelBehavior").toInt() == 0) {
        const double scaleFactor = std::abs(delta) / 100.0;
        // scaleTo(view->imageViewer->getScale() + delta / 10); // Zoom removed
        emit scaledByWheel();
    } else {
        if (delta > 0) {
            prevImage();
        } else {
            nextImage();
        }
    }
}

bool MediaViewerDelegate::copyImageToClipboard() {
    if (this->image.isNull()) {
        ElaMessageBar::error(ElaMessageBarType::Bottom,
                             "Null Image!",
                             nullptr,
                             2000,
                             view->imageViewer);
        return false;
    } else {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setImage(this->image);
        ElaMessageBar::success(ElaMessageBarType::Bottom,
                               "Copied!",
                               nullptr,
                               2000,
                               view->imageViewer);
        return true;
    }
}

void MediaViewerDelegate::openImageFileDialog() {
    QString initFilePath = filepath;
    filepath = QFileDialog::getOpenFileName(nullptr,
                                            "Choose Image File",
                                            initFilePath,
                                            "Image Files (*.png *.jpg *.bmp *.jpeg *.gif)");
    if (!filepath.isEmpty()) {
        loadImage(filepath);
    }
}

void MediaViewerDelegate::saveImageFileDialog() {
    filepath = QFileDialog::getSaveFileName(nullptr,
                                            "Save Image File",
                                            "",
                                            "Image Files (*.png *.jpg *.bmp *.jpeg *.gif)");
    if (!filepath.isEmpty()) {
        this->image.save(filepath);
    }
}

void MediaViewerDelegate::onFileInfoClicked() {
    auto* fileInfoAnimation = new QPropertyAnimation(view->fileInfoWidget, "width");
    connect(fileInfoAnimation, &QPropertyAnimation::valueChanged, [=](const QVariant& value) {
        view->fileInfoWidget->setFixedWidth(value.toInt());
    });
    connect(fileInfoAnimation,
            &QPropertyAnimation::finished,
            fileInfoAnimation,
            &QObject::deleteLater);
    fileInfoAnimation->setEasingCurve(QEasingCurve::OutCubic);
    fileInfoAnimation->setDuration(150);
    if (view->fileInfoWidget->isVisible()) {
        fileInfoAnimation->setStartValue(view->fileInfoWidget->width());
        fileInfoAnimation->setEndValue(0);
        connect(fileInfoAnimation,
                &QPropertyAnimation::finished,
                view->fileInfoWidget,
                &QWidget::hide);
    } else {
        view->fileInfoWidget->show();
        fileInfoAnimation->setStartValue(view->fileInfoWidget->width());
        fileInfoAnimation->setEndValue(350);
    }
    fileInfoAnimation->start();
}

void MediaViewerDelegate::adaptiveResize() {
    // get the resolution of user's monitor
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();
    //get the size of the image

    QImage loadedImage = this->image;
    int imageWidth = loadedImage.width();
    int imageHeight = loadedImage.height();

    //calculate the size of the Window
    int windowWidth = imageWidth;
    int windowHeight = imageHeight;
    if (imageWidth > screenWidth || imageHeight > screenHeight) {
        qreal widthRatio = static_cast<qreal>(screenWidth) / windowWidth;
        qreal heightRatio = static_cast<qreal>(screenHeight) / windowHeight;
        qreal scaleFactor = qMin(widthRatio, heightRatio);
        windowWidth = static_cast<int>(windowWidth * scaleFactor);
        windowHeight = static_cast<int>(windowHeight * scaleFactor);
    }
    view->resize(windowWidth, windowHeight);
}

void MediaViewerDelegate::deleteImage() {
    if (settings.value("confirmDeletion").toBool()) {
        // ask before deletion
        auto* confirmDialog = new ElaContentDialog(view);
        confirmDialog->setWindowTitle("Confirm Deletion");
        auto* centralWidget = new QWidget(view);
        auto* centralVLayout = new QVBoxLayout(centralWidget);
        centralVLayout->setContentsMargins(15, 25, 15, 10);
        auto* title = new ElaText("Delete File", view);
        title->setTextStyle(ElaTextType::Title);
        auto* subTitle = new ElaText("Are you sure you want to delete this file?", view);
        subTitle->setTextStyle(ElaTextType::Body);
        centralVLayout->addWidget(title);
        centralVLayout->addSpacing(2);
        centralVLayout->addWidget(subTitle);
        centralVLayout->addStretch();
        confirmDialog->setCentralWidget(centralWidget);
        confirmDialog->setRightButtonText("Yes");
        confirmDialog->setMiddleButtonText("No");
        confirmDialog->setLeftButtonText("Cancel");
        connect(confirmDialog, &ElaContentDialog::middleButtonClicked, this, [=, this]() {
            confirmDialog->close();
        });
        connect(confirmDialog, &ElaContentDialog::rightButtonClicked, this, [=, this]() {
            if (!QFile(filepath).remove()) {
                ElaMessageBar::error(ElaMessageBarType::Bottom,
                                     "Delete failed!",
                                     nullptr,
                                     2000,
                                     view->imageViewer);
            } else {
                ElaMessageBar::success(ElaMessageBarType::Bottom,
                                       "Deleted!",
                                       nullptr,
                                       2000,
                                       view->imageViewer);
            }
        });
        confirmDialog->exec();
    } else {
        if (!QFile(filepath).remove()) {
            ElaMessageBar::error(ElaMessageBarType::Bottom,
                                 "Delete failed!",
                                 nullptr,
                                 2000,
                                 view->imageViewer);
        } else {
            ElaMessageBar::success(ElaMessageBarType::Bottom,
                                   "Deleted!",
                                   nullptr,
                                   2000,
                                   view->imageViewer);
        }
    }
}

void MediaViewerDelegate::prevImage() {
    if (mediaIndex.row() > 0) {
        mediaIndex = mediaListModel->index(mediaIndex.row() - 1, MediaListModel::Path);
    } else {
        mediaIndex = mediaListModel->index(mediaListModel->rowCount() - 1, MediaListModel::Path);
    }
    filepath = mediaIndex.data().value<QString>();
    loadImage(filepath);
}

void MediaViewerDelegate::nextImage() {
    if (mediaIndex.row() < mediaListModel->rowCount() - 1) {
        mediaIndex = mediaListModel->index(mediaIndex.row() + 1, MediaListModel::Path);
    } else {
        mediaIndex = mediaListModel->index(0, MediaListModel::Path);
    }
    filepath = mediaIndex.data().value<QString>();
    loadImage(filepath);
}

// Image rotation function removed to eliminate image editing functionality

bool MediaViewerDelegate::loadImage(const QString& path, bool fadeAnimation) {
    if (path.isEmpty()) {
        return false;
    }
    try {
        QImage loaded(path);
        if (loaded.isNull()) {
            return false;
        }
        if (this->image.isNull() || this->image != loaded) {
            this->image = loaded;
            emit imageChanged(fadeAnimation);
            return true;
        }
    } catch (...) {
    }
    return false;
}

bool MediaViewerDelegate::loadImage(const QImage& image, bool fadeAnimation) {
    if (image.isNull()) {
        return false;
    }
    try {
        if (this->image.isNull() || this->image != image) {
            this->image = image;
            emit imageChanged(fadeAnimation);
            return true;
        }
    } catch (...) {
    }
    return false;
}

// Image scaling function removed to eliminate image editing functionality

void MediaViewerDelegate::rotateImage() {
    // rotate 90 degrees clockwise visually (view-only)
    view->imageViewer->rotateBy(90);
}

void MediaViewerDelegate::zoomIn() {
    int s = view->imageViewer->getScale();
    int maxS = view->imageViewer->getMaxScale();
    int next = qMin(maxS, s + 10);
    scaleTo(next);
}

void MediaViewerDelegate::zoomOut() {
    int s = view->imageViewer->getScale();
    int minS = view->imageViewer->getMinScale();
    int next = qMax(minS, s - 10);
    scaleTo(next);
}

void MediaViewerDelegate::zoomToOriginal() {
    scaleTo(100);
}

void MediaViewerDelegate::scaleTo(int percent) {
    // percent is in [1..800], where 100 is original size
    if (this->image.isNull()) return;
    view->imageViewer->scaleToPercent(percent);
}

int MediaViewerDelegate::getScale() const {
    return view->imageViewer->getScale();
}

void MediaViewerDelegate::openInFileExplorer() {
    if (filepath.isEmpty()) {
        ElaMessageBar::error(ElaMessageBarType::Bottom,
                             "No file selected!",
                             nullptr,
                             2000,
                             view->imageViewer);
        return;
    }
    QString directoryPath = QFileInfo(filepath).absolutePath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(directoryPath));
}
