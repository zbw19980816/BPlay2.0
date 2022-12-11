/********************************
 * Bwidget.h
 * 功能：视频显示控件头文件
 * *****************************/

#ifndef BWIDGET_H
#define BWIDGET_H

#include <QOpenGLWidget>
#include <QPainter>
#include "Bffmpeg.h"
#include "Bvideo.h"

class Bwidget : public QOpenGLWidget
{
public:
    Bwidget(QWidget *parent = NULL);
    void paintEvent(QPaintEvent *event);
    void InitMedia(bool isFirst);
    void timerEvent(QTimerEvent *event);
private:
    int WidgetWidth;    /* Widget宽 */
    int WidgetHeight;   /* Widget高 */
    int Width;          /* 视频显示实际宽 */   
    int Height;         /* 视频显示实际高 */  
    int X;              /* 视频显示实际左上角X坐标 */ 
    int Y;              /* 视频显示实际左上角Y坐标 */ 
    AVFrame *LastFrame = NULL;  /* 最后一帧数据 */
    uchar *ImageData = NULL;    /* 视频数据 */
    QImage *Image = NULL;       /* 视频信息 */
    QMutex BwidgetMtx;          /* 送显锁 */
    int TimerID;                /* 定时刷新widget定时器ID */

    void SetSize();             /* 设置显示区域 */
    void paintPic(AVFrame* frame);
    void resizeEvent(QResizeEvent *event);
};

#endif // BWIDGET_H
