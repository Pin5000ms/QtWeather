#include "widget.h"
#include "ui_widget.h"

// 日出日落底線
const QPoint Widget::sun[2] = {
    QPoint(20, 75),
    QPoint(130, 75)
};


// 日出日落圓弧
const QRect Widget::rect[2] = {
    QRect(25, 25, 100, 100), // 虛線圓弧
    QRect(50, 80, 50, 20) // “日出日落”文本
};

#define SPAN_INDEX 3                       // 温度曲线间隔指数
#define ORIGIN_SIZE 3                      // 温度曲线原点大小
#define TEMPERATURE_STARTING_COORDINATE 45 // 高温平均值起始坐标

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);

    exitMenu = new QMenu(this);
    exitAct = new QAction;
    exitAct->setText("Exit");
    exitAct->setIcon(QIcon(":/icon/close.ico"));

    exitMenu->addAction(exitAct);
    connect(exitAct, &QAction::triggered, this, []() {
        qApp->exit(0);
    });


    forecast_week_list << ui->day1 << ui->day2 << ui->day3 << ui->day4 << ui->day5 << ui->day6;
    forecast_date_list << ui->date1 << ui->date2 << ui->date3 << ui->date4 << ui->date5 << ui->date6;
    forecast_aqi_list << ui->quality1 << ui->quality2 << ui->quality3 << ui->quality4 << ui->quality5 << ui->quality6;
    forecast_type_list << ui->weather1 << ui->weather2 << ui->weather3 << ui->weather4 << ui->weather5 << ui->weather6;
    forecast_typeIco_list << ui->icon1 << ui->icon2 << ui->icon3 << ui->icon4 << ui->icon5 << ui->icon6;
    forecast_high_list << ui->deg1h << ui->deg2h << ui->deg3h << ui->deg4h << ui->deg5h << ui->deg6h;
    forecast_low_list << ui->deg1l << ui->deg2l << ui->deg3l << ui->deg4l << ui->deg5l << ui->deg6l;

    // 請求天氣API
    url = "http://t.weather.itboy.net/api/weather/city/";
    city = u8"长沙";
    cityTmp = city;
    qDebug() << city;
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    getWeatherInfo(manager);


    /* 事件過濾 */
    ui->sunriseset->installEventFilter(this); // 啟用事件過濾器
    ui->curve->installEventFilter(this);
    //ui->cityLineEdit->installEventFilter(this);

    sunTimer = new QTimer(ui->sunriseset);
    connect(sunTimer, &QTimer::timeout, this, &Widget::paintSunRiseSet);    //connect(sunTimer, SIGNAL(timeout()), ui->sunriseset, SLOT(update()));
    connect(sunTimer, &QTimer::timeout, this, &Widget::paintCurve);
    sunTimer->start(1000);//1000毫秒觸發一次


}

Widget::~Widget()
{
    delete ui;
}


void Widget::contextMenuEvent(QContextMenuEvent *event){
    exitMenu->exec(QCursor::pos());
    event->accept();
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    move(event->globalPos()-mPos);
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    mPos = event->globalPos()-this->pos();
}



void Widget::getWeatherInfo(QNetworkAccessManager *manager)
{
    QString citycode = "101010100";//tool[city];
    if(citycode=="000000000"){
        QMessageBox::warning(this, "Error", "No City Data!", QMessageBox::Ok);
        return;
    }
    QUrl jsonUrl(url + citycode);
    manager->get(QNetworkRequest(jsonUrl));
}

void Widget::replyFinished(QNetworkReply *reply)
{

    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(reply->error() != QNetworkReply::NoError || status_code != 200)
    {
        QMessageBox::warning(this, u8"错误", u8"天气：请求数据错误，检查网络连接！", QMessageBox::Ok);
        return;
    }

    QByteArray bytes = reply->readAll();
    //QString result = QString::fromLocal8Bit(bytes);
    parseJson(bytes);
}


void Widget::parseJson(QByteArray bytes)
{
    QJsonParseError err;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(bytes, &err); // 检测json格式
    if (err.error != QJsonParseError::NoError) // Json格式错误
    {
        return;
    }

    QJsonObject jsObj = jsonDoc.object();
    QString message = jsObj.value("message").toString();
    if (message.contains("success")==false)
    {
        QMessageBox::information(this, tr("The information of Json_desc"),
                                 u8"天气：城市错误！", QMessageBox::Ok );
        city = cityTmp;
        return;
    }

    QString dateStr = jsObj.value("date").toString();
    today.date = QDate::fromString(dateStr, "yyyyMMdd").toString("yyyy-MM-dd");
    today.city = jsObj.value("cityInfo").toObject().value("city").toString();

    // 解析data
    QJsonObject dataObj = jsObj.value("data").toObject();
    today.shidu = dataObj.value("shidu").toString();
    today.pm25 = QString::number( dataObj.value("pm25").toDouble() );
    today.quality = dataObj.value("quality").toString();
    today.wendu = dataObj.value("wendu").toString();//u8"°"
    today.ganmao = dataObj.value("ganmao").toString();

    // 解析data中的yesterday
    QJsonObject yestObj = dataObj.value("yesterday").toObject();
    forecast[0].date = yestObj.value("date").toString();
    forecast[0].high = yestObj.value("high").toString();
    forecast[0].low = yestObj.value("low").toString();
    forecast[0].aqi = QString::number( yestObj.value("aqi").toDouble() );
    forecast[0].type = yestObj.value("type").toString();

    // 解析data中的forecast
    QJsonArray forecastArr = dataObj.value("forecast").toArray();
    int j = 0;
    for (int i = 1; i < 6; i++)
    {
        QJsonObject dateObj = forecastArr.at(j).toObject();
        forecast[i].date = dateObj.value("date").toString();
        forecast[i].aqi = QString::number( dateObj.value("aqi").toDouble() );
        forecast[i].high = dateObj.value("high").toString();
        forecast[i].low = dateObj.value("low").toString();
        forecast[i].type = dateObj.value("type").toString();
        j++;
    }

    //更新今日數據
    QJsonObject todayObj = forecastArr.at(0).toObject();
    today.fx = todayObj.value("fx").toString();
    today.fl = todayObj.value("fl").toString();
    today.type = todayObj.value("type").toString();
    today.sunrise = todayObj.value("sunrise").toString();
    today.sunset = todayObj.value("sunset").toString();
    today.notice = todayObj.value("notice").toString();

    setLabelContent();
}

/* 设置控件文本 */
void Widget::setLabelContent()
{
    // 今日数据
    ui->date->setText(today.date);
    ui->temp->setText(today.wendu);
    ui->city->setText(today.city);
    ui->weather->setText(today.type);
    //ui->noticeLb->setText(today.notice);
    ui->humidval->setText(today.shidu);
    ui->pm25val->setText(today.pm25);
    ui->winddval->setText(today.fx);
    ui->windsval->setText(today.fl);
    //ui->ganmaoBrowser->setText(today.ganmao);

    // 判断白天还是夜晚图标
    QString sunsetTime = today.date + " " + today.sunset;
    if (QDateTime::currentDateTime() <= QDateTime::fromString(sunsetTime, "yyyy-MM-dd hh:mm"))
    {
        ui->typeIcoLb->setStyleSheet( tr("border-image: url(:/day/day/%1.png); background-color: argb(60, 60, 60, 0);").arg(today.type) );
    }
    else
    {
        ui->typeIcoLb->setStyleSheet( tr("border-image: url(:/night/night/%1.png); background-color: argb(60, 60, 60, 0);").arg(today.type) );
    }

    ui->quality->setText(today.quality == "" ? "None" : today.quality);

    if (today.quality == "Excellent")
    {
        ui->quality1->setStyleSheet("color: rgb(0, 255, 0); background-color: argb(255, 255, 255, 0);");
    }
    else if (today.quality == "Good")
    {
        ui->quality1->setStyleSheet("color: rgb(255, 255, 0); background-color: argb(255, 255, 255, 0);");
    }
    else if (today.quality == "No Good")
    {
        ui->quality1->setStyleSheet("color: rgb(255, 170, 0); background-color: argb(255, 255, 255, 0);");
    }
    else if (today.quality == "Bad")
    {
        ui->quality1->setStyleSheet("color: rgb(255, 0, 0); background-color: argb(255, 255, 255, 0);");
    }
    else if (today.quality == "Worst")
    {
        ui->quality1->setStyleSheet("color: rgb(170, 0, 0); background-color: argb(255, 255, 255, 0);");
    }
    else
    {
        ui->quality1->setStyleSheet("color: rgb(255, 255, 255); background-color: argb(255, 255, 255, 0);");
    }

    // 六天數據
    for (int i = 0; i < 6; i++)
    {
        //forecast_week_list[i]->setText(forecast[i].date.right(3));
        forecast_date_list[i]->setText(forecast[i].date.left(3));
        forecast_type_list[i]->setText(forecast[i].type);
        forecast_high_list[i]->setText(forecast[i].high.split(" ").at(1));
        forecast_low_list[i]->setText(forecast[i].low.split(" ").at(1));
        forecast_typeIco_list[i]->setStyleSheet( tr("image: url(:/day/day/%1.png);").arg(forecast[i].type) );

        if (forecast[i].aqi.toInt() >= 0 && forecast[i].aqi.toInt() <= 50)
        {
            forecast_aqi_list[i]->setText("Excellent");
            forecast_aqi_list[i]->setStyleSheet("color: rgb(0, 255, 0);");
        }
        else if (forecast[i].aqi.toInt() > 50 && forecast[i].aqi.toInt() <= 100)
        {
            forecast_aqi_list[i]->setText("Good");
            forecast_aqi_list[i]->setStyleSheet("color: rgb(255, 255, 0);");
        }
        else if (forecast[i].aqi.toInt() > 100 && forecast[i].aqi.toInt() <= 150)
        {
            forecast_aqi_list[i]->setText("No Good");
            forecast_aqi_list[i]->setStyleSheet("color: rgb(255, 170, 0);");
        }
        else if (forecast[i].aqi.toInt() > 150 && forecast[i].aqi.toInt() <= 200)
        {
            forecast_aqi_list[i]->setText("Bad");
            forecast_aqi_list[i]->setStyleSheet("color: rgb(255, 0, 0);");
        }
        else
        {
            forecast_aqi_list[i]->setText("Worst");
            forecast_aqi_list[i]->setStyleSheet("color: rgb(170, 0, 0);");
        }
    }
}

void Widget::paintSunRiseSet()
{
    QPainter painter(ui->sunriseset);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.save();

    QPen pen = painter.pen();
    pen.setWidthF(0.5);
    pen.setColor(Qt::yellow);
    painter.setPen(pen);
    painter.drawLine(sun[0], sun[1]);
    painter.restore();


    painter.save();
    painter.setFont(QFont("Microsoft Yahei", 8, QFont::Normal)); // 字体、大小、正常粗细
    painter.setPen(Qt::white);

    if (today.sunrise != "" && today.sunset != "")
    {
        ui->sunrise->setText(today.sunrise);
        ui->sunset->setText(today.sunset);
    }
    painter.drawText(rect[1], Qt::AlignHCenter, u8"日出日落");
    painter.restore();


    // 绘制圆弧
    painter.save();
    //    pen.setWidth(1);
    pen.setWidthF(0.5);
    pen.setStyle(Qt::DotLine); // 虚线
    pen.setColor(Qt::green);
    painter.setPen(pen);
    painter.drawArc(rect[0], 0 * 16, 180 * 16);//Qt的角度單位是1/16度
    painter.restore();


    // 绘制日出日落占比
    if (today.sunrise != "" && today.sunset != "")
    {
        painter.setPen(Qt::NoPen);//關閉筆
        painter.setBrush(QColor(255, 85, 0, 100));//設定筆刷顏色

        int startAngle, spanAngle;
        QString sunsetTime = today.date + " " + today.sunset;

        if (QDateTime::currentDateTime() > QDateTime::fromString(sunsetTime, "yyyy-MM-dd hh:mm"))
        {
            startAngle = 0 * 16;
            spanAngle = 180 * 16;
        }
        else
        {
            // 计算起始角度和跨越角度
            static QStringList sunSetTime = today.sunset.split(":");
            static QStringList sunRiseTime = today.sunrise.split(":");

            static QString sunsetHour = sunSetTime.at(0);
            static QString sunsetMint = sunSetTime.at(1);
            static QString sunriseHour = sunRiseTime.at(0);
            static QString sunriseMint = sunRiseTime.at(1);

            static int sunrise = sunriseHour.toInt() * 60 + sunriseMint.toInt();
            static int sunset = sunsetHour.toInt() * 60 + sunsetMint.toInt();
            int now = QTime::currentTime().hour() * 60 + QTime::currentTime().minute();

            startAngle = ((double)(sunset - now) / (sunset - sunrise)) * 180 * 16;
            spanAngle = ((double)(now - sunrise) / (sunset - sunrise)) * 180 * 16;
        }

        if (startAngle >= 0 && spanAngle >= 0)
        {
            painter.drawPie(rect[0], startAngle, spanAngle); // 繪製扇形
        }
    }

    ui->sunriseset->update();
}

/* 事件過濾 */
bool Widget::eventFilter(QObject *watched, QEvent *event)//過濾的對象, 過濾的事件
{
    if (watched == ui->sunriseset && event->type() == QEvent::Paint)
    {
        paintSunRiseSet();
    }
    else if (watched == ui->curve && event->type() == QEvent::Paint)
    {
        paintCurve();
    }
//    else if (watched == ui->cityLineEdit && event->type() == QEvent::MouseButtonPress)
//    {
//        callKeyBoard(); // 调用软键盘
//    }

    return QWidget::eventFilter(watched, event);
}


void Widget::paintCurve()
{
    QPainter painter(ui->curve);
    painter.setRenderHint(QPainter::Antialiasing, true); // 反鋸齒

    // 将温度转换为int类型，并计算平均值，平均值作为curveLb曲线的参考值，参考Y坐标为45
    int tempTotal = 0;
    int high[6] = {};
    int low[6] = {};

    QString h, l;


    for (int i = 0; i < 6; i++)
    {
        QStringList highParts = forecast[i].high.split(" ");
        if (highParts.size() > 1) {
            h = highParts.at(1);
            h = h.left(h.length() - 1);
            high[i] = static_cast<int>(h.toDouble());
            tempTotal += high[i];
        } else {
            return;
            // 處理錯誤情況，可能記錄日誌或設置默認值
            qDebug() << "Unexpected format in forecast high:" << forecast[i].high;
        }

        QStringList lowParts = forecast[i].low.split(" ");
        if (lowParts.size() > 1) {
            l = lowParts.at(1);
            l = l.left(l.length() - 1);
            low[i] = static_cast<int>(l.toDouble());
        } else {
            return;
            // 處理錯誤情況，可能記錄日誌或設置默認值
            qDebug() << "Unexpected format in forecast low:" << forecast[i].low;
        }
//        h = forecast[i].high.split(" ").at(1);
//        h = h.left(h.length() - 1);
//        high[i] = (int)(h.toDouble());
//        tempTotal += high[i];

//        l = forecast[i].low.split(" ").at(1);
//        l = l.left(h.length() - 1);
//        low[i] = (int)(l.toDouble());
    }
    int tempAverage = (int)(tempTotal / 6); // 最高温平均值



    // 算出温度对应坐标
    int pointX[6] = {35, 110, 185, 260, 335, 410}; // 点的X坐标
    int pointHY[6] = {0};
    int pointLY[6] = {0};
    for (int i = 0; i < 6; i++)
    {
        pointHY[i] = TEMPERATURE_STARTING_COORDINATE - ((high[i] - tempAverage) * SPAN_INDEX);
        pointLY[i] = TEMPERATURE_STARTING_COORDINATE + ((tempAverage - low[i]) * SPAN_INDEX);
    }

    QPen pen = painter.pen();
    pen.setWidth(1);

    // 高温曲线绘制
    painter.save();
    pen.setColor(QColor(255, 170, 0));
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);
    painter.setBrush(QColor(255, 170, 0));
    painter.drawEllipse(QPoint(pointX[0], pointHY[0]), ORIGIN_SIZE, ORIGIN_SIZE);
    painter.drawEllipse(QPoint(pointX[1], pointHY[1]), ORIGIN_SIZE, ORIGIN_SIZE);
    painter.drawLine(pointX[0], pointHY[0], pointX[1], pointHY[1]);

    pen.setStyle(Qt::SolidLine);
    pen.setWidth(1);
    painter.setPen(pen);

    for (int i = 1; i < 5; i++)
    {
        painter.drawEllipse(QPoint(pointX[i + 1], pointHY[i + 1]), ORIGIN_SIZE, ORIGIN_SIZE);
        painter.drawLine(pointX[i], pointHY[i], pointX[i + 1], pointHY[i + 1]);
    }
    painter.restore();

    // 低温曲线绘制
    pen.setColor(QColor(0, 255, 255));
    pen.setStyle(Qt::DotLine);
    painter.setPen(pen);
    painter.setBrush(QColor(0, 255, 255));
    painter.drawEllipse(QPoint(pointX[0], pointLY[0]), ORIGIN_SIZE, ORIGIN_SIZE);
    painter.drawEllipse(QPoint(pointX[1], pointLY[1]), ORIGIN_SIZE, ORIGIN_SIZE);
    painter.drawLine(pointX[0], pointLY[0], pointX[1], pointLY[1]);

    pen.setColor(QColor(0, 255, 255));
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    for (int i = 1; i < 5; i++)
    {
        painter.drawEllipse(QPoint(pointX[i + 1], pointLY[i + 1]), ORIGIN_SIZE, ORIGIN_SIZE);
        painter.drawLine(pointX[i], pointLY[i], pointX[i + 1], pointLY[i + 1]);
    }

    ui->curve->update();
}

