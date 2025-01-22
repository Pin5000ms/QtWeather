#ifndef WEATHERDATA_H
#define WEATHERDATA_H
#include <QWidget>
#include <QList>
#include <QLabel>
#include <QPoint>
#include <QRect>
#include <QTimer>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QByteArray>
#include "WeatherTool.h"

namespace Ui {
    class Weather;
}

struct Today
{
    QString date;
    QString wendu;
    QString city;
    QString shidu;
    QString pm25;
    QString quality;
    QString ganmao;
    QString fx;
    QString fl;
    QString type;
    QString sunrise;
    QString sunset;
    QString notice;
};

struct Forecast
{
    QString date;
    QString high;
    QString low;
    QString aqi;
    QString type;
};

class Weather : public QWidget
{
    Q_OBJECT

public:
    explicit Weather(QWidget *parent = 0);
    ~Weather();

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *menuEvent);

public slots:
    void replayFinished(QNetworkReply *reply);
    //void on_searchBt_clicked();
    //void on_refreshBt_clicked();
    //void slot_exitApp();

private:
    Ui::Weather *ui;

    QPoint mPos; // 窗口移动

    QList<QLabel *> forecast_week_list;
    QList<QLabel *> forecast_date_list;
    QList<QLabel *> forecast_aqi_list;
    QList<QLabel *> forecast_type_list;
    QList<QLabel *> forecast_typeIco_list;
    QList<QLabel *> forecast_high_list;
    QList<QLabel *> forecast_low_list;

    Today today;
    Forecast forecast[6];

    static const QPoint sun[2];
    static const QRect sunRizeSet[2];
    static const QRect rect[2];

    QTimer *sunTimer;

    QNetworkAccessManager *manager;
    QString url;
    QString city;
    QString cityTmp;
    WeatherTool tool;

    /* 右键菜单退出程序 */
    QMenu *m_pMenu;
    QAction *m_pExitAct;

    void getWeatherInfo(QNetworkAccessManager *manager);
    void parseJson(QByteArray bytes);
    void setLabelContent();
    void paintSunRiseSet();
    void paintCurve();
    void callKeyBoard();
};
#endif // WEATHERDATA_H
