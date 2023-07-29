#include "mainwindow.h"
#include "majorimageprocessingthread.h"
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
            this, SLOT(new_frame_display(QImage)));
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


bool MainWindow::new_frame_display(QImage image)
{
    ui->mainlabel->clear();
    if(image.isNull())
    {
        ui->mainlabel->setText("画面丢失！");
    }
    else
    {
        image = image.scaled(ui->mainlabel->width(),ui->mainlabel->height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        QPixmap pix = QPixmap::fromImage(image,Qt::AutoColor);
        ui->mainlabel->setPixmap(pix);

    }

    return true;
}

void MainWindow::clickPhotoButton(void)
{
//    ui->photoButton->setDisabled(1);
    imageprocessthread->init(0);
    imageprocessthread->start();
}

void MainWindow::on_stopButton_clicked()
{
    imageprocessthread->stop();
    ui->mainlabel->setText("Device is ready");
    ui->photoButton->setDisabled(0);
}



