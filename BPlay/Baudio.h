/********************************
 * Baudio.h
 * 功能：音频处理头文件
 * *****************************/

#ifndef BAUDIO_H
#define BAUDIO_H

#include <QAudioOutput>
#include <QThread>
#include <QMutex>
#include "Bffmpeg.h"

typedef enum {
    X05 = 5,
    X08 = 8,
    X10 = 10,
    X12 = 12,
    X15 = 15,
    X20 = 20
}SPEED;

class Baudio : public QThread
{
public:
    static Baudio* GetInstance();
    void run();
    void start();
    void stop();
    void OpenAudioOutput();
    void AudioReset();
    void SetVolum(qreal volum);
    void SetSpeed(SPEED speed);
private:
    Baudio();
    static QAtomicPointer<Baudio> Instance;
    static QMutex mtx;

    bool audiorun;                              /* 音频解码线程是否启动 */
    QIODevice *io = NULL;                       /* 音频设备 */
    QAudioOutput *output = NULL;                /* 音频输出 */
    qreal Volume = 1.0;                         /* 音量(0.0~1.0) */
    SPEED Speed = X10;                          /* 倍速(0.5~2) */
};


#endif // BAUDIO_H
