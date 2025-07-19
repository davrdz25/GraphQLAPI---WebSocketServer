#pragma once
#include <iostream>

struct UserSessionDTO
{
    std::optional<std::string> Code;
    std::optional<std::string> Email;
    std::optional<std::string> Phone;
    std::string Password;

};
