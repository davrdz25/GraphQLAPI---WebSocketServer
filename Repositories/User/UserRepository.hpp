#pragma once

#include "../../Interfaces/IUserRepository.hpp"
#include "../../Database/SQL.hpp"
#include "../../Utils/Hasher.hpp"

#include <numeric>

class UserRepository : public IUserRepository
{
private:
        std::shared_ptr<SQL> Database;
        int _UserEntry;
public:
    explicit UserRepository(std::shared_ptr<SQL> Database);
    ~UserRepository(){};

    bool Create(const CreateUserDTO&) override;
    bool Update(const UserDTO &) override;
    bool Delete(const UserDTO &) override;

    DataTable ReadUsers(const UserSearchDTO &) override;
    DataTable ReadByEntry(const int &) override;
    DataTable ReadByCode(const std::string &) override;
    DataTable ReadByFirstName(const std::string &) override;
    DataTable ReadByLastName(const std::string &) override;
    DataTable ReadByPhone(const std::string &) override;
    DataTable ReadByEmail(const std::string &) override;

    bool ValidateEntry(const int &) override;
    bool ValidateCode(const std::string &) override;
    bool UpdatePassword(const UserSearchFilter &, const std::string &, const std::string &, const std::string &) override;
    bool UpdateUserSequence() override;
    bool GetCurrentSequence() override;
    bool ValidatePassword(const UserSearchFilter &,const std::string &, const std::string &) override;
};
