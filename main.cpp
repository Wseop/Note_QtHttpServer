#include <QCoreApplication>
#include <QHttpServer>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

bool authorize(const QHttpServerRequest& request)
{
    for (const auto&[key, value] : request.headers())
    {
        if (key == "SECRET_KEY" && value == "SECRET_VALUE")
            return true;
    }
    return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QHttpServer httpServer;
    const auto port = httpServer.listen(QHostAddress::Any);

    if (!port)
        return 0;

    qDebug() << QCoreApplication::translate("QHttpServerExample",
                "Running on http://127.0.0.1:%1/ (Press CTRL+C to quit)").arg(port);

    QJsonArray array;
    for (int i = 0; i < 5; i++)
    {
        QJsonObject obj;
        QJsonValue value;

        value = i + 10000;
        obj.insert(QString("%1").arg(i), value);
        array.append(obj);
    }

    // GET route
    httpServer.route("/exampleApi", QHttpServerRequest::Method::Get,
                     [&array](const QHttpServerRequest& request) {
        return QHttpServerResponse(array);
    });

    // GET route with dynamic parameters
    httpServer.route("/exampleApi/<arg>", QHttpServerRequest::Method::Get,
                     [&array](int id, const QHttpServerRequest& request) {
        QJsonObject obj = array[id].toObject();
        return QHttpServerResponse(obj);
    });

    // GET route with dynamic parameters
    httpServer.route("/exampleApi/<arg>/<arg>", QHttpServerRequest::Method::Get,
                     [&array](int id, QString msg, const QHttpServerRequest& request) {
        qDebug() << msg;

        QJsonObject obj = array[id].toObject();
        return QHttpServerResponse(obj);
    });

    // GET route with parameter pattern & hosting files
    httpServer.route("/exampleApi/<arg>-png", QHttpServerRequest::Method::Get,
                     [](int id, const QHttpServerRequest& request) {
        return QHttpServerResponse::fromFile(QString(":/resources/%1.png").arg(id));
    });

    // POST route with authorization
    httpServer.route("/exampleApi", QHttpServerRequest::Method::Post,
                     [&array](const QHttpServerRequest& request) {
        // 인증 진행 - 실패 시 401(Unauthorized) return
        if (!authorize(request))
            return QHttpServerResponse(QHttpServerResponder::StatusCode::Unauthorized);

        // 데이터 생성 및 추가 - 성공 시 201(Created) return
        QJsonObject newObj = QJsonDocument::fromJson(request.body()).object();
        array.append(newObj);
        return QHttpServerResponse(newObj, QHttpServerResponder::StatusCode::Created);
    });

    return a.exec();
}
