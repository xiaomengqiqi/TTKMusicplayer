#include "musicapplicationobject.h"
#include <QPropertyAnimation>
#include <Windows.h>
#include <Dbt.h>
#include "musicmobiledeviceswidget.h"
#include "musicaudiorecorderwidget.h"
#include "musictimerwidget.h"
#include "musictimerautoobject.h"

MusicApplicationObject::MusicApplicationObject(QObject *parent)
    : QObject(parent), m_mobileDevices(NULL)
{
    m_supperClass = static_cast<QWidget*>(parent);

    m_animation = new QPropertyAnimation(parent, "windowOpacity");
    m_animation->setDuration(1000);
    m_animation->setStartValue(0);
    m_animation->setEndValue(1);
    m_animation->start();

    m_musicTimerAutoObj = new MusicTimerAutoObject(this);
    connect(m_musicTimerAutoObj, SIGNAL(setPlaySong(int)), parent,
                                 SLOT(setPlaySongChanged(int)));
    connect(m_musicTimerAutoObj, SIGNAL(setStopSong()), parent,
                                 SLOT(setStopSongChanged()));

    m_setWindowToTop = false;
    connect(this, SIGNAL(getCurrentPlayList(QStringList&)), parent,
                  SLOT(getCurrentPlayList(QStringList&)));
    musicToolSetsParameter();
}

MusicApplicationObject::~MusicApplicationObject()
{
    delete m_mobileDevices;
    delete m_musicTimerAutoObj;
    delete m_animation;
}

void MusicApplicationObject::nativeEvent(const QByteArray &,
                                         void *message, long *)
{
    MSG* msg = reinterpret_cast<MSG*>(message);
    if(msg->message == WM_DEVICECHANGE)
    {
        PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)msg->lParam;
        qDebug()<<msg->wParam;
        switch(msg->wParam)
        {
            case DBT_DEVICETYPESPECIFIC:
                break;
            case DBT_DEVICEARRIVAL:
                if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
                {
                    PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                    if (lpdbv->dbcv_flags == 0)
                    {
                        DWORD unitmask = lpdbv ->dbcv_unitmask;
                        int i;
                        for(i = 0; i < 26; ++i)
                        {
                            if(unitmask & 0x1)
                                break;
                            unitmask = unitmask >> 1;
                        }
                        qDebug() << "USB_Arrived and The USBDisk is: "<<(char)(i + 'A');
                        delete m_mobileDevices;
                        m_mobileDevices = new MusicMobileDevicesWidget;
                        m_mobileDevices->show();
                    }
                }
                break;
            case DBT_DEVICEREMOVECOMPLETE:
                if(lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)
                {
                    PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
                    if (lpdbv -> dbcv_flags == 0)
                    {
                        qDebug() << "USB_remove";
                        delete m_mobileDevices;
                        m_mobileDevices = NULL;
                    }
                }
                break;
            default: break;
        }
    }
}

void MusicApplicationObject::musicAboutUs()
{
    QMessageBox::about(NULL, tr("About QMusicPlayer"),
       tr("QMusicPlayer") + QString("\n\n") +
       tr("Directed By Greedysky") +
       QString("\nCopyright© 2014-2015") +
       QString("\nMail:Greedysky@163.com"));
}

void MusicApplicationObject::musicAudioRecorder()
{
    MusicAudioRecorderWidget recorder;
    recorder.exec();
}

void MusicApplicationObject::musicTimerWidget()
{
    MusicTimerWidget timer;
    QStringList list;
    emit getCurrentPlayList(list);
    timer.setSongStringList(list);
    connect(&timer, SIGNAL(timerParameterChanged()),
                    SLOT(musicToolSetsParameter()));
    timer.exec();
}

void MusicApplicationObject::musicSetWindowToTop()
{
    m_setWindowToTop = !m_setWindowToTop;
    Qt::WindowFlags flags = m_supperClass->windowFlags();
    m_supperClass->setWindowFlags( m_setWindowToTop ?
                  (flags | Qt::WindowStaysOnTopHint) :
                  (flags & ~Qt::WindowStaysOnTopHint) );
    m_supperClass->show();
}

void MusicApplicationObject::musicToolSetsParameter()
{
    m_musicTimerAutoObj->runTimerAutoConfig();
}
