#ifndef LOGINCONFIGURATION_H
#define LOGINCONFIGURATION_H

#include <QObject>
#include <QSharedDataPointer>

class LoginConfigurationData;

class LoginConfiguration : public QObject
{
    Q_OBJECT
public:
    explicit LoginConfiguration(QObject *parent = nullptr);
    LoginConfiguration(const LoginConfiguration &);
    LoginConfiguration(LoginConfiguration &&);
    LoginConfiguration &operator=(const LoginConfiguration &);
    LoginConfiguration &operator=(LoginConfiguration &&);
    ~LoginConfiguration();

signals:

private:
    QSharedDataPointer<LoginConfigurationData> data;
};

#endif // LOGINCONFIGURATION_H
