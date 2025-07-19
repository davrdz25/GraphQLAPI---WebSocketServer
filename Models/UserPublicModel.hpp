#pragma once
#include <iostream>
#include "../Database/DataTable.hpp"

struct UserPublicModel {
    int Entry;
    std::string Code;
    std::string FirstName;
    std::optional<std::string> MiddleName;
    std::string LastName;
    std::optional<std::string> SecondLastName;
    std::string Email;
    std::string Phone;
};
