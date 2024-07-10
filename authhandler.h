#ifndef AUTHHANDLER_H
#define AUTHHANDLER_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>

QT_BEGIN_NAMESPACE
namespace Ui { class AuthHandler; }
QT_END_NAMESPACE

class AuthHandler : public QMainWindow
{
    Q_OBJECT

public:
    AuthHandler(QWidget *parent = nullptr);
    ~AuthHandler();

private slots:
    void on_addExpensesButton_clicked();
    void on_viewExpensesButton_clicked();

    void networkReplyReadyRead();
    void performAuthenticatedDatabaseCall();

private:
    Ui::AuthHandler *ui;
    QNetworkAccessManager *m_networkAccessManager;
    QNetworkReply *m_networkReply;
    QString m_apiKey;
    QString m_idToken;

    void performPOST(const QString &url, const QJsonDocument &payload);
    void parseResponse(const QByteArray &response);
    void fetchUserData();
    void signInUser(const QString &email, const QString &password);
    void setAPIKey(const QString &apiKey);
    void addExpense(const QString &category, double amount, const QString &date, const QString &details);
    void fetchExpenses();
};

#endif // AUTHHANDLER_H
