#pragma once
#include <iostream>

struct ChangePasswordDTO
{
    std::optional<std::string> Code;
    std::optional<std::string> Email;
    std::optional<std::string> Phone;
    std::string OldPassword;
    std::string NewPassword;
};
