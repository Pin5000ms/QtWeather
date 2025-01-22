#include "weatherdata.h"


Weather::Weather(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Weather)
{
    ui->setupUi(this);

    this->setWindowFlag(Qt::FramelessWindowHint);
}

//void Weather::parseJson(QByteArray bytes)
//{
//    QJsonParseError err;
//    QJsonDocument jsonDoc = QJsonDocument::fromJson(bytes, &err); // 检测json格式
//    if (err.error != QJsonParseError::NoError) // Json格式错误
//    {
//        return;
//    }

//    QJsonObject jsObj = jsonDoc.object();
//    QString message = jsObj.value("message").toString();
//    if (message.contains("success")==false)
//    {
//        QMessageBox::information(this, tr("The information of Json_desc"),
//                                 u8"天气：城市错误！", QMessageBox::Ok );
//        city = cityTmp;
//        return;
//    }

//    QString dateStr = jsObj.value("date").toString();
//    today.date = QDate::fromString(dateStr, "yyyyMMdd").toString("yyyy-MM-dd");
//    today.city = jsObj.value("cityInfo").toObject().value("city").toString();

//    // 解析data
//    QJsonObject dataObj = jsObj.value("data").toObject();
//    today.shidu = dataObj.value("shidu").toString();
//    today.pm25 = QString::number( dataObj.value("pm25").toDouble() );
//    today.quality = dataObj.value("quality").toString();
//    today.wendu = dataObj.value("wendu").toString() + u8"°";
//    today.ganmao = dataObj.value("ganmao").toString();

//    // 解析data中的yesterday
//    QJsonObject yestObj = dataObj.value("yesterday").toObject();
//    forecast[0].date = yestObj.value("date").toString();
//    forecast[0].high = yestObj.value("high").toString();
//    forecast[0].low = yestObj.value("low").toString();
//    forecast[0].aqi = QString::number( yestObj.value("aqi").toDouble() );
//    forecast[0].type = yestObj.value("type").toString();

//    // 解析data中的forecast
//    QJsonArray forecastArr = dataObj.value("forecast").toArray();
//    int j = 0;
//    for (int i = 1; i < 6; i++)
//    {
//        QJsonObject dateObj = forecastArr.at(j).toObject();
//        forecast[i].date = dateObj.value("date").toString();
//        forecast[i].aqi = QString::number( dateObj.value("aqi").toDouble() );
//        forecast[i].high = dateObj.value("high").toString();
//        forecast[i].low = dateObj.value("low").toString();
//        forecast[i].type = dateObj.value("type").toString();
//        j++;
//    }

//    // 取得今日数据
//    QJsonObject todayObj = forecastArr.at(0).toObject();
//    today.fx = todayObj.value("fx").toString();
//    today.fl = todayObj.value("fl").toString();
//    today.type = todayObj.value("type").toString();
//    today.sunrise = todayObj.value("sunrise").toString();
//    today.sunset = todayObj.value("sunset").toString();
//    today.notice = todayObj.value("notice").toString();

//    //setLabelContent();
//}

void Weather::replayFinished(QNetworkReply *reply)
{
    /* 获取响应的信息，状态码为200表示正常 --comment by wsg 2017/12/11 */
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(reply->error() != QNetworkReply::NoError || status_code != 200)
    {
        QMessageBox::warning(this, u8"错误", u8"天气：请求数据错误，检查网络连接！", QMessageBox::Ok);
        return;
    }

    QByteArray bytes = reply->readAll();
    //    QString result = QString::fromLocal8Bit(bytes);
    parseJson(bytes);
}

//void Weather::getWeatherInfo(QNetworkAccessManager *manager)
//{
//    url = "http://t.weather.itboy.net/api/weather/city/";

//    QString citycode = tool[city];
//    if(citycode=="000000000"){
//        QMessageBox::warning(this, "Error", "", QMessageBox::Ok);
//        return;
//    }
//    QUrl jsonUrl(url + citycode);
//    manager->get( QNetworkRequest(jsonUrl) );
//}


