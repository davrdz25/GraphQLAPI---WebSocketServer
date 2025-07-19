#pragma once
#include <iostream>

struct UserSearchDTO {
    bool ExactValues;
    std::optional<std::string> FirstName;
    std::optional<std::string> MiddleName;
    std::optional<std::string> LastName;
    std::optional<std::string> SecondLastName;
};