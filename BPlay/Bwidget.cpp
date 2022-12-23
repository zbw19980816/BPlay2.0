/********************************
 * Bwidget.cpp
 * 功能：视频显示控件cpp
 * *****************************/

#include "Bwidget.h"
#include <QDragEnterEvent>
#include <QMimeData>
#include <QTimer>

/********************************
 * Bwidget::Bwidget(QWidget *parent) : QOpenGLWidget(parent)
 * 功能：Bwidget构造 
 * *****************************/
Bwidget::Bwidget(QWidget *parent) : QOpenGLWidget(parent)
{
    TimerID = startTimer(1);
    setAcceptDrops(true);    /* 允许拖拽事件 */
}

/********************************
 * void Bwidget::resizeEvent(QResizeEvent *event)
 * 功能：Bwidget窗口大小变化事件
 * *****************************/
void Bwidget::resizeEvent(QResizeEvent *event)
{
    QOpenGLWidget::resizeEvent(event);
    return;
}

/********************************
 * void Bwidget::mousePressEvent(QMouseEvent *event)
 * 功能：Bwidget鼠标按下事件
 * *****************************/
void Bwidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        /* 只处理左键 */
        static QTimer *timer = NULL;

        if (timer == NULL) {
            timer = new QTimer;
            isDoubleClick = false;
            connect(timer, &QTimer::timeout, [=](){
                if (!isDoubleClick) {
                    emit widgetclick();
                }
                isDoubleClick = false;
                timer->stop();
            });
        } else {
            timer->stop();
        }

        timer->start(300);
    }
    return;
}

/********************************
 * void Bwidget::mouseDoubleClickEvent(QMouseEvent *event)
 * 功能：Bwidget鼠标双击事件
 * *****************************/
void Bwidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        /* 触发自定义鼠标双击信号 */
        isDoubleClick = true;
        emit widgetDoubleclick();
        return;
    }
}

/********************************
 * void Bwidget::dragEnterEvent(QDragEnterEvent* event)
 * 功能：拖拽进入事件
 * *****************************/
void Bwidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        /* 有拖拽文件时设置接受 */
        event->acceptProposedAction();
    }

    return;
}

/********************************
 * void Bwidget::dropEvent(QDropEvent *event)
 * 功能：件拖拽放下事件
 * tip:该方法需要和dragEnterEvent、setAcceptDrops(true)配合才可以生效
 * *****************************/
void Bwidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if(!urls.isEmpty()) {
            /* 触发获取到文件信号 */
            emit GetFile(urls.first().toLocalFile());
        }
    }

    return;
}

/********************************
 * void Bwidget::timerEvent(QTimerEvent *event)
 * 功能：刷新widget
 * *****************************/
void Bwidget::timerEvent(QTimerEvent *event)
{
    /* widget宽高变化后重设视频显示区域 */
    if ((WidgetWidth != width()) || (WidgetHeight != height())) {
        WidgetWidth = width();
        WidgetHeight = height();
        InitMedia(false);
    }

    /* update会触发paintEvent */
    this->update();
}

/********************************
 * void Bwidget::paintPic(AVFrame* frame)
 * 功能：绘制图像
 * *****************************/
void Bwidget::paintPic(AVFrame* frame)
{
    if (NULL == frame) {
        return;
    }

    SwsContext *context = NULL;
    AVCodecContext *codec = Bffmpeg::GetInstance()->GetFormatContext()->streams[Bffmpeg::GetInstance()->GetVideoIndex()]->codec;

    /* 解码数据转RGB32 */
    //AV_PIX_FMT_BGRA ?
    context = sws_getCachedContext(context,                                         /* 转化上下文结构体 */
                                   codec->width, codec->height, codec->pix_fmt,     /* 源图像格式和宽高 */
                                   Width, Height, AV_PIX_FMT_BGRA,                  /* 目标图像格式和宽高 */
                                   SWS_BICUBIC, NULL, NULL, NULL);

    if (NULL == context) {
        BLOG("sws_getCachedContext fail");
        return;
    }

    uint8_t *data[AV_NUM_DATA_POINTERS] = { 0 };
    data[0] = (uint8_t *)Image->bits();
    int linesize[AV_NUM_DATA_POINTERS] = { 0 };
    linesize[0] = Width * 4;                /* 实际显示一行的宽度，32位4个字节 */
    sws_scale(context, frame->data,         /* 源视频解码数据 */
              frame->linesize,              /* 每行大小 */
              0,                            /* 用不到 */
              frame->height,                /* 图像高度 */
              data,                         /* 输出的每个通道数据指针 */
              linesize);                    /* 每个通道行字节数 */
    sws_freeContext(context);

    /* 绘制图像 */
    QPainter painter;
    painter.begin(this);
    painter.drawImage(QPoint(X, Y), *Image);
    painter.end();

    return;
}

/********************************
 * void Bwidget::paintEvent(QPaintEvent *event)
 * 功能：解码数据送显
 * *****************************/
void Bwidget::paintEvent(QPaintEvent *event)
{
    if (Bvideo::GetInstance()->GetFrameque().frame.size() == 0) {
        return;
    }

    /* 解码数据送显 */
    QMutexLocker Locker1(&Bvideo::GetInstance()->GetFrameque().mtx);
    QMutexLocker Locker2(&BwidgetMtx);
    AVFrame* frame = Bvideo::GetInstance()->GetFrameque().frame.front();
    paintPic(frame);

    /* 出队列，释放上一帧解码数据(保留最后一帧数据，用于窗口大小变化后重绘) */
    Bvideo::GetInstance()->GetFrameque().frame.pop_front();
    if (LastFrame) {
        av_frame_unref(LastFrame);
        av_frame_free(&LastFrame);
    }

    LastFrame = frame;

    return;
}

/********************************
 * void Bwidget::SetSize()
 * 功能：设置显示区域
 * *****************************/
void Bwidget::SetSize()
{
    if (NULL == Bffmpeg::GetInstance()->GetFormatContext()) {
        return;
    }
    int WidgetWidth = width();          /* Bwidget宽 */
    int WidgetHeight = height();        /* Bwidget高 */
    int VideoIndex = Bffmpeg::GetInstance()->GetVideoIndex();
    int MediaWidth = Bffmpeg::GetInstance()->GetFormatContext()->streams[VideoIndex]->codec->width;     /* 视频宽 */
    int MediaHeight = Bffmpeg::GetInstance()->GetFormatContext()->streams[VideoIndex]->codec->height;   /* 视频高 */

    BLOG("WidgetWidth[%d],WidgetHeight[%d], MediaWidth[%d], MediaHeight[%d]", WidgetWidth, WidgetHeight, MediaWidth, MediaHeight);

    if (((float)WidgetWidth / (float)WidgetHeight) > ((float)MediaWidth / (float)MediaHeight)) {
        /* Bwidget宽高比大于视频,左右需要有黑边 */
        Width = (int)((float)WidgetHeight * (float)MediaWidth / (float)MediaHeight);
        Height = WidgetHeight;
        X = (WidgetWidth - Width) / 2;
        Y = 0;
    } else {
        /* Bwidget宽高比小于视频,上下需要有黑边 */
        Width = WidgetWidth;
        Height = ((float)WidgetWidth * (float)MediaHeight / (float)MediaWidth);
        X = 0;
        Y = (WidgetHeight - Height) / 2;
    }

    BLOG("Width[%d], Height[%d], X[%d], Y[%d]", Width, Height, X, Y);

    return;
}

/********************************
 * void Bwidget::InitMedia()
 * 功能：初始化媒体画布相关信息
 * bool isFirst: 是否为第一次初始化画布(暂时没用到)
 * *****************************/
void Bwidget::InitMedia(bool isFirst)
{
    QMutexLocker Locker(&BwidgetMtx);
    SetSize();

    if (ImageData) {
        delete ImageData;
    }
    
    if (Image) {
        delete Image;
    }

    /* 初始化视频区域 */
    ImageData = new uchar[Width * Height * 4];
    Image = new QImage(ImageData, Width, Height, QImage::Format_RGB32);

    /* 初始背景为黑色 */
    QPainter pt;
    pt.begin(this);
    QColor color(0, 0, 0);
    pt.fillRect(0, 0, width(), height(), color);
    pt.end();

    /* 绘制最后一帧画面 */
    paintPic(LastFrame);

    return;
}
