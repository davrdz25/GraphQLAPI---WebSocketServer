#include "UserRepository.hpp"

UserRepository::UserRepository(std::shared_ptr<SQL> Database) : Database(std::move(Database)) {}

bool UserRepository::Create(const CreateUserDTO &uNewuser)
{
    try
    {
        Database->BeginTransaction();

        GetCurrentSequence();

        std::cout << "Password text : " << "[" << uNewuser.Password << "]\n";

        std::vector<uint8_t> hashedPassword = Hasher::HashPassword(uNewuser.Password);

        std::string sQuery = "INSERT INTO Users (UserEntry, UserCode, FirstName, MiddleName, LastName, SecondLastName, Email, Phone, Password) "
                             "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";

        std::vector<SQLParam> params = {
            MakeSQLParam(_UserEntry),
            MakeSQLParam(uNewuser.Code),
            MakeSQLParam(uNewuser.FirstName),
            MakeSQLParam(uNewuser.MiddleName),
            MakeSQLParam(uNewuser.LastName),
            MakeSQLParam(uNewuser.SecondLastName),
            MakeSQLParam(uNewuser.Email),
            MakeSQLParam(uNewuser.Phone),
            MakeSQLParam(hashedPassword)};

        std::cout << "[Debug] HASH GUARDADO:\n";
        for (auto b : hashedPassword)
            std::printf("%02X ", b);
        std::cout << std::endl;

        if (!Database->RunPrepared(sQuery, params))
        {
            Database->RollbackTransaction();
            throw std::runtime_error("Error creating user");
        }

        if (!UpdateUserSequence())
        {
            Database->RollbackTransaction();
            throw std::runtime_error("Error updating sequence");
        }

        Database->CommitTransaction();

        return true;
    }
    catch (const std::exception &e)
    {
        Database->RollbackTransaction();
        throw std::runtime_error(std::string("[CreateUser Exception] ") + " " + e.what());
    }
}

bool UserRepository::UpdateUserSequence()
{
    try
    {
        const std::string sQuery = "UPDATE Sequences SET User_Seq = User_Seq + 1";

        if (Database->RunStatement(sQuery))
            return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[UpdateUserSequence Exception] ") + " " + e.what());
    }
};

bool UserRepository::GetCurrentSequence()
{
    try
    {
        const std::string sQuery = "SELECT ISNULL(User_Seq, 0) + 1 User_Seq FROM Sequences WHERE Entry = 1";

        DataTable data = Database->FetchResults(sQuery);

        if (data.RowsCount() == 1)
            _UserEntry = data[0]["User_Seq"];
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[GetCurrentSequence Exception] ") + " " + e.what());
    }
};

bool UserRepository::Update(const UserDTO &user)
{
    try
    {
        std::string sQuery = "UPDATE Users SET FirstName = ?, MiddleName = ?, LastName = ?, SecondLastName = ?, Email = ?, Phone = ? ";
        std::vector<SQLParam> updateParams;

        updateParams.emplace_back(MakeSQLParam(user.FirstName));
        updateParams.emplace_back(MakeSQLParam(user.MiddleName));
        updateParams.emplace_back(MakeSQLParam(user.LastName));
        updateParams.emplace_back(MakeSQLParam(user.SecondLastName));
        updateParams.emplace_back(MakeSQLParam(user.Email));
        updateParams.emplace_back(MakeSQLParam(user.Phone));

        sQuery += "WHERE UserEntry = ?";
        updateParams.emplace_back(MakeSQLParam(user.Entry));

        Database->BeginTransaction();
        Database->RunPrepared(sQuery, updateParams);
        Database->CommitTransaction();

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(e.what());
    }
};

bool UserRepository::Delete(const UserDTO &)
{
    try
    {
        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
};

bool UserRepository::UpdatePassword(const UserSearchFilter &filter, const std::string &value, const std::string &oldPassword, const std::string &newPassword)
{
    try
    {
        std::string sQuery = "UPDATE Users SET Password = ? WHERE ";

        std::vector<uint8_t> hashedPassword = Hasher::HashPassword(newPassword);

        std::vector<SQLParam> vParams = {
            MakeSQLParam(hashedPassword)
        };

        switch (filter)
        {
        case UserSearchFilter::Code:
            sQuery += "UserCode = ?";
            vParams.push_back(MakeSQLParam(value));
            break;

        case UserSearchFilter::Email:
            sQuery += "Email = ?";
            vParams.push_back(MakeSQLParam(value));
            break;

        case UserSearchFilter::Phone:
            sQuery += "Phone = ?";
            vParams.push_back(MakeSQLParam(value));
            break;

        default:
            break;
        }

        std::cout << sQuery << std::endl;

        Database->BeginTransaction();
        Database->RunPrepared(sQuery, vParams);
        Database->CommitTransaction();

        return true;
    }
    catch (const std::exception &e)
    {
        Database->RollbackTransaction();
        throw std::runtime_error(e.what());
    }
};

DataTable UserRepository::ReadByCode(const std::string &Code)
{
    try
    {
        DataTable dataTable;

        std::string sQuery = "SELECT [UserEntry], UserCode, FirstName, MiddleName, LastName, SecondLastName, Phone, Email FROM Users WHERE UserCode = ?";

        dataTable = Database->FetchPrepared(sQuery, Code);

        return dataTable;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[ReadByCode Exception] ") + " " + e.what());
    }
};

DataTable UserRepository::ReadByFirstName(const std::string &firstName)
{
    try
    {
        DataTable dataTable;

        std::string sQuery = "SELECT [UserEntry], UserCode, FirstName, MiddleName, LastName, SecondLastName, Phone, Email FROM Users WHERE FirstName = ?";

        dataTable = Database->FetchPrepared(sQuery, firstName);

        return dataTable;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
};

DataTable UserRepository::ReadByLastName(const std::string &lastName)
{
    try
    {
        DataTable dataTable;

        std::string sQuery = "SELECT [UserEntry], UserCode, FirstName, MiddleName, LastName, SecondLastName, Phone, Email FROM Users WHERE LastName = ?";

        dataTable = Database->FetchPrepared(sQuery, lastName);

        return dataTable;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
};

DataTable UserRepository::ReadByPhone(const std::string &phone)
{
    try
    {
        DataTable dataTable;

        std::string sQuery = "SELECT [UserEntry], UserCode, FirstName, MiddleName, LastName, SecondLastName, Phone, Email FROM Users WHERE Phone = ?";

        dataTable = Database->FetchPrepared(sQuery, phone);

        return dataTable;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
};

DataTable UserRepository::ReadByEmail(const std::string &email)
{
    try
    {
        DataTable dataTable;

        std::string sQuery = "SELECT [UserEntry], UserCode, FirstName, MiddleName, LastName, SecondLastName, Phone, Email FROM Users WHERE Email = ?";

        dataTable = Database->FetchPrepared(sQuery, email);

        return dataTable;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
};

DataTable UserRepository::ReadUsers(const UserSearchDTO &filter)
{
    try
    {
        DataTable dataTable;
        std::string baseQuery = "SELECT [UserEntry], UserCode, FirstName, MiddleName, LastName, SecondLastName, Phone, Email FROM Users";
        std::vector<std::string> conditions;
        std::vector<std::string> parameters;

        auto addCondition = [&](const std::string& field, const std::optional<std::string>& value)
        {
            if (value.has_value()) {
                /* TODO Implements LIKE condition */
                /* if (filter.ExactValues) { */
                    conditions.push_back(field + " = ?");
                    parameters.emplace_back(value.value());
                /*}  else { 
                    conditions.push_back(field + " LIKE ?");
                    parameters.emplace_back("%" + value.value() + "%");
                 } */
            }
        };

        addCondition("FirstName",      filter.FirstName);
        addCondition("MiddleName",     filter.MiddleName);
        addCondition("LastName",       filter.LastName);
        addCondition("SecondLastName", filter.SecondLastName);

        std::string query = baseQuery;
        if (!conditions.empty()) {
            query += " WHERE " + std::accumulate(
                std::next(conditions.begin()), conditions.end(), conditions[0],
                [&](std::string acc, const std::string& cond) {
                    return acc + (filter.ExactValues ? " AND " : " OR ") + cond;
                }
            );
        }

        dataTable = Database->FetchPrepared(query, parameters);
        std::cout << "Total Users " << dataTable.RowsCount() << std::endl;
        return dataTable;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("ReadUsers exception: ") + e.what());
    }
}

DataTable UserRepository::ReadByEntry(const int &iEntry)
{
    try
    {
        DataTable dataTable;

        std::string sQuery = "SELECT [UserEntry], UserCode, FirstName, MiddleName, LastName, SecondLastName, Phone, Email FROM Users WHERE [UserEntry] = ?";

        dataTable = Database->FetchPrepared(sQuery, std::to_string(iEntry));

        return dataTable;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
};

bool UserRepository::ValidatePassword(const UserSearchFilter &searchFilter, const std::string &filterValue, const std::string &Password)
{
    try
    {
        std::string sQuery = "SELECT Password FROM Users WHERE ";

        switch (searchFilter)
        {
        case UserSearchFilter::Entry:
            sQuery += "UserEntry = ?";
            break;

        case UserSearchFilter::Code:
            sQuery += "UserCode = ?";
            break;

        case UserSearchFilter::Email:
            sQuery += "Email = ?";
            break;

        case UserSearchFilter::Phone:
            sQuery += "Phone = ?";
            break;
        }

        std::cout << "Password text : " << "[" << Password << "]\n";

        DataTable data = Database->FetchPrepared(sQuery, filterValue);

        std::vector<uint8_t> userPassword = data[0]["Password"];

        if (Hasher::VerifyPassword(Password, userPassword))
            return true;

        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
};

bool UserRepository::ValidateEntry(const int &entry)
{
    try
    {
        const std::string sQuery = "SELECT ISNULL(COUNT(*), 0) Total FROM Users WHERE UserEntry = ?";

        DataTable data = Database->FetchPrepared(sQuery, std::to_string(entry));

        if (data.RowsCount() == 0)
            return false;

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(e.what());
    }
};

bool UserRepository::ValidateCode(const std::string &code)
{
    try
    {
        const std::string sQuery = "SELECT ISNULL(COUNT(*), 0) Total FROM Users WHERE UserCode = ?";

        DataTable data = Database->FetchPrepared(sQuery, code);

        if (data.RowsCount() == 0)
            return false;

        return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(e.what());
    }
};