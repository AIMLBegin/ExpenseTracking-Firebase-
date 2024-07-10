#include "authhandler.h"
#include "ui_authhandler.h"
#include <QDebug>
#include <QVariantMap>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonDocument>

AuthHandler::AuthHandler(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AuthHandler)
{
    ui->setupUi(this);
    m_networkAccessManager = new QNetworkAccessManager(this); //acess with network
    setAPIKey("AIzaSyBAEkRyYjCQPTReXdxKTcnPkOpz_5D2e9g"); //api key link
    //defining the category according to the db
    ui->categoryComboBox->addItem("Food");
    ui->categoryComboBox->addItem("Transportation");
    ui->categoryComboBox->addItem("Entertainment");
    ui->categoryComboBox->addItem("Groceries");
//connect with the buttons
    connect(ui->addExpensesButton, &QPushButton::clicked, this, &AuthHandler::on_addExpensesButton_clicked);
    connect(ui->viewExpensesButton, &QPushButton::clicked, this, &AuthHandler::on_viewExpensesButton_clicked);

    // Sign in the user to get the ID token
    signInUser("user1@example.com", "password@65");
     signInUser("user2@example.com", "password@75");
}

AuthHandler::~AuthHandler()
{
    delete ui;
    m_networkAccessManager->deleteLater();
}

void AuthHandler::setAPIKey(const QString &apiKey)
{
    m_apiKey = apiKey;
}

void AuthHandler::on_addExpensesButton_clicked()
{
    QString category = ui->categoryComboBox->currentText();
    double amount = ui->amountlineEdit->text().toDouble();
    QString date = ui->dateEdit->date().toString();
    QString details = ui->detailslineEdit->text();

    addExpense(category, amount, date, details);
}

void AuthHandler::on_viewExpensesButton_clicked()
{
    fetchExpenses();
}

void AuthHandler::networkReplyReadyRead()
{
    QByteArray response = m_networkReply->readAll();
    m_networkReply->deleteLater();
    qDebug()<<"network response:" << response;
    parseResponse(response);
}

void AuthHandler::performAuthenticatedDatabaseCall()
{
    if (m_idToken.isEmpty()) {
        qDebug() << "Error: ID token is empty. Cannot proceed with the request.";
        return;
    }

    QString endPoint = "https://expensetracker-b24c2-default-rtdb.firebaseio.com/expenses.json?auth=" + m_idToken;
    qDebug() << "Fetching URL:" << endPoint;
    QNetworkRequest request(QUrl(endPoint));
    m_networkReply = m_networkAccessManager->get(QNetworkRequest(QUrl(endPoint)));
    connect(m_networkReply, &QNetworkReply::readyRead, this, &AuthHandler::networkReplyReadyRead);
}

void AuthHandler::performPOST(const QString &url, const QJsonDocument &payload)
{
    QNetworkRequest newRequest((QUrl(url)));
    newRequest.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));
    m_networkReply = m_networkAccessManager->post(newRequest, payload.toJson());
    connect(m_networkReply, &QNetworkReply::readyRead, this, &AuthHandler::networkReplyReadyRead);
}

void AuthHandler::parseResponse(const QByteArray &response)
{
    QJsonDocument jsonDocument = QJsonDocument::fromJson(response);
    if (jsonDocument.isNull() || !jsonDocument.isObject()) {
            qDebug() << "Error: Invalid JSON response";
            return;
        }
    QJsonObject jsonObject = jsonDocument.object();

    if (jsonObject.contains("error")) {
        qDebug() << "Error occurred!" << response;
    } else if (jsonObject.contains("idToken")) {
        QString idToken = jsonObject.value("idToken").toString();
        qDebug() << "User signed in successfully!";
        m_idToken = idToken;
        performAuthenticatedDatabaseCall();
    } else {
        qDebug() << "Full JSON Response: " << QString(response);
    }
}

void AuthHandler::addExpense(const QString &category, double amount, const QString &date, const QString &details)
{
    if (m_idToken.isEmpty()) {
       qDebug() << "Error: ID token is empty. Cannot proceed with the request.";
        return;
    }

    QString endPoint = "https://expensetracker-b24c2-default-rtdb.firebaseio.com/expenses.json?auth=" + m_idToken;
    qDebug() << "Posting to URL:" << endPoint;

    QVariantMap variantPayload;
    variantPayload["category"] = category;
    variantPayload["amount"] = amount;
    variantPayload["date"] = date;
    variantPayload["details"] = details;

    QJsonDocument jsonPayload = QJsonDocument::fromVariant(variantPayload);
    performPOST(endPoint, jsonPayload);
}

void AuthHandler::fetchExpenses()
{
    performAuthenticatedDatabaseCall();
}

void AuthHandler::fetchUserData()
{
    if (m_idToken.isEmpty()) {
        qDebug() << "Error: ID token is empty. Cannot proceed with the request.";
        return;
    }
    qDebug() << "Fetching user data...";
    performAuthenticatedDatabaseCall();
}

void AuthHandler::signInUser(const QString &email, const QString &password)
{
    QUrl url("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + m_apiKey);
    QVariantMap payload;
    payload["email"] = email;
    payload["password"] = password;
    payload["returnSecureToken"] = true;

    QJsonDocument doc = QJsonDocument::fromVariant(payload);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json"); //contentType is a header is set to application/json to indicate that the request payload is in JSON format

    QNetworkReply* reply = m_networkAccessManager->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QByteArray response = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
        QJsonObject jsonObj = jsonDoc.object();

        if (jsonObj.contains("idToken")) {
            m_idToken = jsonObj["idToken"].toString();
            qDebug() << "Successfully signed in. ID Token:" << m_idToken;
        } else if (jsonObj.contains("error")) {
            qDebug() << "Sign in error:" << jsonObj["error"].toObject()["message"].toString();
        }

        reply->deleteLater();
    });
}
