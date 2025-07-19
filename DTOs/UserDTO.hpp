#pragma once
#include <iostream>

struct UserDTO {
    int Entry;
    std::optional<std::string> Code;
    std::optional<std::string> FirstName;
    std::optional<std::string> MiddleName;
    std::optional<std::string> LastName;
    std::optional<std::string> SecondLastName;
    std::optional<std::string> Email;
    std::optional<std::string> Phone;
    std::optional<std::string> Password;
};