/********************************
 * mainwindow.cpp
 * 功能：主窗口cpp
 * *****************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "Bpublic.h"
#include "Bffmpeg.h"
#include "Bwidget.h"
#include <QFileDialog>
#include <QDebug>

/********************************
 * MainWindow::MainWindow(QWidget *parent)
 * 功能：主窗口构造函数
 * *****************************/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* BPlay Logo */
    this->setWindowIcon(QIcon(":/BPlay/Image/WindowIcon.png"));
    this->setWindowTitle(QString("BPlay2.0"));

    /* 初始化开始/停止播放按钮 */
    ui->BOnOffButton->setCheckable(true);  //new

    /* 进度条和播放时长 */
    ui->BPlaySlider->setValue(0);
    ui->MediaAllTime->setText("00:00");
    ui->MediaPlayTime->setText("00:00");

    /* 为进度条注册事件过滤器(鼠标点击进度条任意处可准确修改滑块位置) */
    ui->BPlaySlider->installEventFilter(this);

    /* 音量控制 */
    ui->BplayVolume->setMaximum(100);
    ui->BplayVolume->setValue(100);

    /* 为音量条注册事件过滤器(鼠标点击进度条任意处可准确修改滑块位置) */
    ui->BplayVolume->installEventFilter(this);

    /* 倍速按钮 */
    QMenu* menu = new QMenu(this);

    /* 0.5~2.0倍速 */
    QAction *Speed_05 = new QAction("X0.5", this);
    QAction *Speed_08 = new QAction("X0.8", this);
    QAction *Speed_10 = new QAction("X1.0", this);
    QAction *Speed_12 = new QAction("X1.2", this);
    QAction *Speed_15 = new QAction("X1.5", this);
    QAction *Speed_20 = new QAction("X2.0", this);
    menu->addAction(Speed_05);
    menu->addAction(Speed_08);
    menu->addAction(Speed_10);
    menu->addAction(Speed_12);
    menu->addAction(Speed_15);
    menu->addAction(Speed_20);
    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint);   //无边角
    menu->setAttribute(Qt::WA_TranslucentBackground);
    menu->setStyleSheet(" QMenu {border-radius:5px;font-family:'Microsoft Yahei';font-size:14px;color:#fff;}"
                            " QMenu::item {height:30px; width:100px;padding-left:20px;border: 1px solid none;}"
                            "QMenu::item:selected {background-color:rgb(0,120,215); padding-left:20px;border: 1px solid rgb(65,173,255);}");    //设置背景
    ui->BplaySpeed->setMenu(menu);
    connect(menu, &QMenu::triggered, this, [=](QAction *act) {
        if (act == Speed_05) {
            ui->BplaySpeed->setText("X0.5");
            Baudio::GetInstance()->SetSpeed(X05);
            BLOG("Speed_05");
        } else if (act == Speed_08) {
            ui->BplaySpeed->setText("X0.8");
            Baudio::GetInstance()->SetSpeed(X08);
            BLOG("Speed_08");
        } else if (act == Speed_10) {
            ui->BplaySpeed->setText("倍速");
            Baudio::GetInstance()->SetSpeed(X10);
            BLOG("Speed_10");
        } else if (act == Speed_12) {
            ui->BplaySpeed->setText("X1.2");
            Baudio::GetInstance()->SetSpeed(X12);
            BLOG("Speed_12");
        } else if (act == Speed_15) {
            ui->BplaySpeed->setText("X1.5");
            Baudio::GetInstance()->SetSpeed(X15);
            BLOG("Speed_15");
        } else if (act == Speed_20) {
            ui->BplaySpeed->setText("X2.0");
            Baudio::GetInstance()->SetSpeed(X20);
            BLOG("Speed_20");
        }
    });

    /* BPlay2.0图标 */
    BPlayLable = new QLabel();
    BPlayLable->setParent(this);
    BPlayLable->resize(QPixmap(":/BPlay/Image/BPlay.png").width(), QPixmap(":/BPlay/Image/BPlay.png").height());
    BPlayLable->setPixmap(QPixmap(":/BPlay/Image/BPlay.png"));

    /* 主窗口通用事件处理 */
    connect(&UpdateTimer, &QTimer::timeout, [=](){
        /* 1、进度条刷新 */
        int TimeAll = (int)Bffmpeg::GetInstance()->GetTimeAll();
        int Pts = (int)Bffmpeg::GetInstance()->GetAudioPts();

        ui->MediaAllTime->setText(QString("%1:%2:%3").arg(TimeAll / 3600).arg((TimeAll % 3600) / 60).arg(TimeAll % 60));
        ui->MediaPlayTime->setText(QString("%1:%2:%3").arg(Pts / 3600).arg((Pts % 3600) / 60).arg(Pts % 60));

        if (!BPlaySliderPress) {
            ui->BPlaySlider->setMaximum(TimeAll);
            ui->BPlaySlider->setValue(Pts);
        }
    });

    UpdateTimer.start(10);  //定视频没有播放时时器可以关闭，后期优化

    /* 播放区鼠标单击事件处理 */
    connect(ui->BPlayopenGLWidget, &Bwidget::widgetclick, this, [=](){
        if (Bffmpeg::GetInstance()->GetFormatContext() == NULL) {
            return;
        }

        if (ui->BOnOffButton->isChecked()) {
            ui->BOnOffButton->setChecked(false);
            Bffmpeg::GetInstance()->StopPlay();
        } else {
            ui->BOnOffButton->setChecked(true);
            Bffmpeg::GetInstance()->StartPlay();
        }
    });

    /* 播放区鼠标双击事件处理 */
    connect(ui->BPlayopenGLWidget, &Bwidget::widgetDoubleclick, this, [=](){
        isFull = !isFull;
        if (isFull) {
             ui->widget_2->hide();
             ui->widget->hide();
             showFullScreen();
        } else {
            ui->widget_2->show();
            ui->widget->show();
            showNormal();
        }
    });

    /* 播放区文件拖入事件处理 */
    connect(ui->BPlayopenGLWidget, &Bwidget::GetFile, this, &MainWindow::LoadMediaFileAndPlay);

    /* 允许捕捉鼠标移动 */
    setMouseTracking(true);
    ui->centralwidget->setMouseTracking(true);
    ui->widget_3->setMouseTracking(true);
    ui->BPlayopenGLWidget->setMouseTracking(true);
}

/********************************
 * MainWindow::~MainWindow()
 * 功能：主窗口析构函数
 * *****************************/
MainWindow::~MainWindow()
{
    delete ui;
}

/********************************
 * void MainWindow::resizeEvent(QResizeEvent *event)
 * 功能：主窗口大小变化回调
 * *****************************/
void MainWindow::resizeEvent(QResizeEvent *event)
{
    BPlayLable->move(QPoint(this->width() / 2 - QPixmap(":/BPlay/Image/BPlay.png").width() / 2,
                            this->height() / 2 - 35 - QPixmap(":/BPlay/Image/BPlay.png").height() / 2));
    QMainWindow::resizeEvent(event);
    return;
}

/********************************
 * bool MainWindow::eventFilter(QObject *obj, QEvent *event)
 * 功能：事件过滤器
 * *****************************/
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->BPlaySlider) {
        if (event->type() == QEvent::MouseButtonPress) {
            /* 滑动条事件过滤 */
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            int value = QStyle::sliderValueFromPosition(ui->BPlaySlider->minimum(), ui->BPlaySlider->maximum(), mouseEvent->pos().x(), ui->BPlaySlider->width());
            ui->BPlaySlider->setValue(value);
            Bffmpeg::GetInstance()->ResetTime(ui->BPlaySlider->value());
            if (ui->BOnOffButton->isChecked()) {
                Bffmpeg::GetInstance()->StartPlay();
            }
        }
    } else if (obj == ui->BplayVolume) {
        if ((event->type() == QEvent::MouseButtonPress) || (event->type() == QEvent::MouseMove)) {
            /* 音量条事件过滤 */
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            int value = QStyle::sliderValueFromPosition(ui->BplayVolume->minimum(), ui->BplayVolume->maximum(), mouseEvent->pos().x(), ui->BplayVolume->width());
            ui->BplayVolume->setValue(value);
            BLOG("%d", value);
            Baudio::GetInstance()->SetVolum(value / 100.0);
        }
    }
    
    return QObject::eventFilter(obj,event);
}

/********************************
 * void MainWindow::on_BPlaySlider_valueChanged()
 * 功能：滑块的值发生改变回调函数(先保留)
 * *****************************/
void MainWindow::on_BPlaySlider_valueChanged()
{
    return;
}

/********************************
 * void MainWindow::on_BPlaySlider_sliderPressed()
 * 功能：滑动条点击回调函数
 * *****************************/
void MainWindow::on_BPlaySlider_sliderPressed()
{
    BPlaySliderPress = true;
    return;
}

/********************************
 * void MainWindow::on_BPlaySlider_sliderPressed()
 * 功能：滑动条释放回调函数
 * *****************************/
void MainWindow::on_BPlaySlider_sliderReleased()
{
    BPlaySliderPress = false;
    Bffmpeg::GetInstance()->ResetTime(ui->BPlaySlider->value());
    if (ui->BOnOffButton->isChecked()) {
        Bffmpeg::GetInstance()->StartPlay();
    }

    return;
}

/********************************
 * void MainWindow::LoadMediaFileAndPlay(QString filepath)
 * 功能：打开文件并播放
 * *****************************/
void MainWindow::LoadMediaFileAndPlay(QString filepath)
{
    if (0 != Bffmpeg::GetInstance()->BLoadMediaFile(filepath)) {
        /* 媒体文件获取失败 */
        BLOG("Media File illegal");
        QMessageBox::information(this, QString("错误"), QString("媒体文件非法！"));
        return;
    }

    /* 初始化画布 */
    ui->BPlayopenGLWidget->InitMedia(true);

    /* 更新按钮状态,开启媒体播放 */
    ui->BOnOffButton->setChecked(true);
    Bffmpeg::GetInstance()->StartPlay();

    /* 隐藏Bplay2.0图标 */
    BPlayLable->hide();

    return;
}

/********************************
 * void MainWindow::wheelEvent(QWheelEvent *event)
 * 功能：滚轮事件回调函数
 * *****************************/
void MainWindow::wheelEvent(QWheelEvent *event)
{
    if (event->delta() > 0) {
        /* 增大音量 */
        if (ui->BplayVolume->value() < 99) {
            ui->BplayVolume->setValue(ui->BplayVolume->value() + 2);
        } else if (ui->BplayVolume->value() == 99) {
            ui->BplayVolume->setValue(100);
        }
    } else{
        /* 降低音量 */
        if (ui->BplayVolume->value() > 1) {
            ui->BplayVolume->setValue(ui->BplayVolume->value() - 2);
        } else if (ui->BplayVolume->value() == 1) {
            ui->BplayVolume->setValue(0);
        }
    }

    Baudio::GetInstance()->SetVolum(ui->BplayVolume->value() / 100.0);
    BLOG("BplayVolume: %d", ui->BplayVolume->value());

    return;
}

/********************************
 * void MainWindow::mouseMoveEvent(QMouseEvent *event)
 * 功能：鼠标移动事件回调函数(未点击也可感知)
 * *****************************/
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (isFull) {
        ui->widget_2->show();
        ui->widget->show();
        static QTimer *timer = NULL;

        if (timer == NULL) {
            timer = new QTimer;
            connect(timer, &QTimer::timeout, this, [=](){
                if (isFull) {
                    ui->widget_2->hide();
                    ui->widget->hide();
                }
                timer->stop();
            });
        } else {
            timer->stop();
        }

        timer->start(3000);
    }

    return;
}

/********************************
 * void MainWindow::on_Bopenfile_btn_clicked()
 * 功能：打开文件按钮点击回调函数
 * *****************************/
void MainWindow::on_Bopenfile_btn_clicked()
{
    QString FilePath = QFileDialog::getOpenFileName(this, QString("媒体文件"), QString("."), QString("视频文件(*.mp4 *.flv *.avi);;所有文件(*.*)"));
    if (FilePath.isEmpty()) {
        BLOG("Media File empty");
        return;
    }

    LoadMediaFileAndPlay(FilePath);

    return;
}

/********************************
 * void MainWindow::on_BFullScreen_btn_clicked()
 * 功能：全屏按钮点击回调函数
 * *****************************/
void MainWindow::on_BFullScreen_btn_clicked()
{
    isFull = !isFull;
    if (isFull) {
         ui->widget_2->hide();
         ui->widget->hide();
         showFullScreen();
    } else {
        ui->widget_2->show();
        ui->widget->show();
        showNormal();
    }

    return;
}

/********************************
 * void MainWindow::on_BOnOffButton_clicked()
 * 功能：开始/暂停按钮点击回调函数
 * *****************************/
void MainWindow::on_BOnOffButton_clicked()
{
    if (Bffmpeg::GetInstance()->GetFormatContext() == NULL) {
        return;
    }

    if (ui->BOnOffButton->isChecked()) {
        Bffmpeg::GetInstance()->StartPlay();
    } else {
        Bffmpeg::GetInstance()->StopPlay();
    }

    return;
}
