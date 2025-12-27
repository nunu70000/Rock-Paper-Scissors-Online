#include "loginconfiguration.h"

#include <utility>

class LoginConfigurationData : public QSharedData
{
public:
};

LoginConfiguration::LoginConfiguration(QObject *parent)
    : QObject{parent}
    , data(new LoginConfigurationData)
{}

LoginConfiguration::LoginConfiguration(const LoginConfiguration &rhs)
    : data{rhs.data}
{}

LoginConfiguration::LoginConfiguration(LoginConfiguration &&rhs)
    : data{std::move(rhs.data)}
{}

LoginConfiguration &LoginConfiguration::operator=(const LoginConfiguration &rhs)
{
    if (this != &rhs)
        data = rhs.data;
    return *this;
}

LoginConfiguration &LoginConfiguration::operator=(LoginConfiguration &&rhs)
{
    if (this != &rhs)
        data = std::move(rhs.data);
    return *this;
}

LoginConfiguration::~LoginConfiguration() {}
