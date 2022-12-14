/********************************
 * mainwindow.h
 * 功能：主窗口头文件
 * *****************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QTimer>
#include <QStyle>
#include <QMouseEvent>
#include <QMenu>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool eventFilter(QObject *obj, QEvent *event);
    
private slots:
    void on_Bopenfile_btn_clicked();
    void on_BFullScreen_btn_clicked();
    void on_BOnOffButton_clicked();
    void on_BPlaySlider_sliderPressed();
    void on_BPlaySlider_sliderReleased();
    void on_BPlaySlider_valueChanged();
    
private:
    Ui::MainWindow *ui;
    QTimer UpdateTimer;              /* 进度条和播放时间刷新定时器 */
    bool BPlaySliderPress = false;   /* 进度条有无按下 */
    bool isFull = false;             /* 是否为全屏状态 */
    QLabel *BPlayLable = NULL;       /* BPlay2.0图标 */

    void LoadMediaFileAndPlay(QString filepath);
    void resizeEvent(QResizeEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
};
#endif // MAINWINDOW_H
