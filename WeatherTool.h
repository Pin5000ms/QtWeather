#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <map>
#include <QFile>
#include <QCoreApplication>
#include <QtDebug>

class WeatherTool{
private:
    std::map<QString, QString> map_city2code;
public:

    WeatherTool(){
        QString fileName = QCoreApplication::applicationDirPath();
        fileName +="/citycode-2019-08-23.json";
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Unable to open the file";
            return;
        }

        file.open(QIODevice::ReadOnly| QIODevice::Text);
        QByteArray json = file.readAll();
        file.close();
        QJsonParseError err;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(json,&err);
        QJsonArray citys = jsonDoc.array();
        for(int i = 0; i < citys.size(); i++){
            QString code = citys.at(i).toObject().value("city_code").toString();
            QString city = citys.at(i).toObject().value("city_name").toString();
            //qDebug()<<city;
            if(code.size() > 0){
                map_city2code.insert(std::pair<QString, QString>(city, code));
            }
        }
    }


    QString operator[](const QString& city){
        std::map<QString,QString>::iterator it = map_city2code.find(city);
        if(it == map_city2code.end()){
            //it = map_city2code.find(city + u8"市");
        }
        if(it != map_city2code.end()){
            return it->second;
        }
        return "000000000";
    }

};

#endif // WEATHERTOOL_H
