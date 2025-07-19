#pragma once
#include <iostream>

struct CreateUserDTO {
    std::string Code;
    std::string FirstName;
    std::optional<std::string> MiddleName;
    std::string LastName;
    std::optional<std::string> SecondLastName;
    std::string Email;
    std::string Phone;
    std::string Password;
};