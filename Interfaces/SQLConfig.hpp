#pragma once
#include <iostream>

struct SQLConfig
{
    static std::string ServerName;
    static std::string DatabaseName;
    static std::string UserName;
    static std::string Password;
    static bool TrustServerCertificate;
};
