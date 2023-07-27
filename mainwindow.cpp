#include "mainwindow.h"
#include "ui_mainwindow.h"

#define UNUSED(X) (void)X

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    imageprocessthread = new MajorImageProcessingThread;

    connect(ui->quitButton, SIGNAL(clicked()), this, SLOT(clickQuitButton()));
    connect(ui->photoButton, SIGNAL(clicked()), this, SLOT(clickPhotoButton()));
    connect(imageprocessthread, SIGNAL(SendMajorImageProcessing(QImage)),
            this, SLOT(new_frame_display_4_rgb(QImage)));
}

MainWindow::~MainWindow()
{
    delete imageprocessthread;
    delete ui;
}
void MainWindow::clickQuitButton(void)
{
    this->close();
}

/* QString 转 char* */
char * MainWindow::qstringToChar(QString srcString)
{
    if (srcString.isEmpty()) {
        return NULL;
    }

    QByteArray      ba  = srcString.toLatin1();
    char *          destCharArray;

    destCharArray           = ba.data();
    return destCharArray;
}


bool MainWindow::new_frame_display_4_rgb(QImage image)
{
    ui->mainlabel->clear();
    if(image.isNull())
    {
        ui->mainlabel->setText("画面丢失！");
    }
    else
    {
        QPixmap pix = QPixmap::fromImage(image,Qt::AutoColor);
//        ui->mainlabel->setPixmap(QPixmap::fromImage(img.scaled(ui->mainlabel->resize(width,height))));
//        ui->mainlabel->setPixmap(QPixmap::fromImage(img.scaled(img.size())));
        ui->mainlabel->setPixmap(pix);
//        ui->mainlabel->setPixmap(QPixmap::fromImage(img.scaled(ui->mainlabel->size())));
    }

    return true;
}

void MainWindow::clickPhotoButton(void)
{
    imageprocessthread->init(0);
    imageprocessthread->start();
}
