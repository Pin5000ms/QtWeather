#ifndef WIDGET_H
#define WIDGET_H

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

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QByteArray>
#include <QPainter>

#include "WeatherTool.h"

#include <iostream>
#include <string>
#include <regex>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE


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

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    bool eventFilter(QObject* watched, QEvent* event);
    void contextMenuEvent(QContextMenuEvent *event);//重寫右鍵點擊事件
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

private slots:
    void replyFinished(QNetworkReply *reply);
    //void on_searchBt_clicked();
    //void on_refreshBt_clicked();
private:
    Ui::Widget *ui;
    QMenu *exitMenu;
    QAction *exitAct;
    QPoint mPos;

    QList<QLabel *> forecast_week_list;//星期幾
    QList<QLabel *> forecast_date_list;//日期
    QList<QLabel *> forecast_aqi_list;//空氣品質
    QList<QLabel *> forecast_type_list;//天氣
    QList<QLabel *> forecast_typeIco_list;//天氣Icon
    QList<QLabel *> forecast_high_list;//當日高溫
    QList<QLabel *> forecast_low_list;//當日低溫

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
    WeatherTool tool = WeatherTool();

    void getWeatherInfo(QNetworkAccessManager *manager);
    void parseJson(QByteArray bytes);
    void setLabelContent();
    void paintSunRiseSet();
    void paintCurve();
    void callKeyBoard();
};
#endif // WIDGET_H
