#include "UserService.hpp"

UserService::UserService(std::shared_ptr<IUserRepository> userRepository)
    : userRepository(std::move(userRepository)) {}

bool UserService::AddUser(const CreateUserDTO &uNewUser)
{
    try
    {
        if (uNewUser.Code.length() == 0)
            throw std::invalid_argument("Code is empty");

        if (uNewUser.FirstName.length() == 0)
            throw std::invalid_argument("FirstName is empty");

        if (uNewUser.LastName.length() == 0)
            throw std::invalid_argument("LastName is empty");

        if (uNewUser.Email.length() == 0)
            throw std::invalid_argument("Email is empty");

        if (uNewUser.Phone.length() == 0)
            throw std::invalid_argument("Phone is empty");

        if (userRepository->Create(uNewUser))
            return true;

        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        throw std::runtime_error(e.what());
    }
};

UserPublicModel UserService::ModifyUser(const UserDTO &uUser)
{
    try
    {
        if (!userRepository->ValidateEntry(uUser.Entry))
            throw std::runtime_error("Entry not found");

        if (uUser.Code.has_value() && !userRepository->ValidateCode(uUser.Code.value()))
            throw std::runtime_error("Code not found");

        if (userRepository->Update(uUser))
        {
            DataTable data = userRepository->ReadByEntry(uUser.Entry);

            UserPublicModel user{
                data[0]["UserEntry"],
                data[0]["UserCode"],
                data[0]["FirstName"],
                data[0]["MiddleName"].IsNull() ? std::nullopt : std::optional<std::string>{data[0]["MiddleName"]},
                data[0]["LastName"],
                data[0]["SecondLastName"].IsNull() ? std::nullopt : std::optional<std::string>{data[0]["SecondLastName"]},
                data[0]["Email"],
                data[0]["Phone"]};

            return user;
        }
        else
            throw std::runtime_error("Error updating user ");
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(e.what());
    }
};

bool UserService::ModifyPassword(const ChangePasswordDTO &dto)
{
    try
    {

        if (dto.Code.has_value() && userRepository->ValidatePassword(UserSearchFilter::Code, dto.Code.value(), dto.OldPassword))
            if (userRepository->UpdatePassword(UserSearchFilter::Code, dto.Code.value(), dto.OldPassword, dto.NewPassword))
                return true;

        if (dto.Email.has_value() && userRepository->ValidatePassword(UserSearchFilter::Email, dto.Email.value(), dto.OldPassword))
            if (userRepository->UpdatePassword(UserSearchFilter::Email, dto.Email.value(), dto.OldPassword, dto.NewPassword))
                return true;

        if (dto.Phone.has_value() && userRepository->ValidatePassword(UserSearchFilter::Phone, dto.Phone.value(), dto.OldPassword))
            if (userRepository->UpdatePassword(UserSearchFilter::Phone, dto.Phone.value(), dto.OldPassword, dto.NewPassword))
                return true;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(e.what());
    }
};

std::vector<UserPublicModel> UserService::SearchUsers(const UserSearchDTO &filter)
{
    try
    {
        std::cout << "Entra a service" << std::endl;

        std::vector<UserPublicModel> users;
        DataTable data = userRepository->ReadUsers(filter);

        if (data.RowsCount() == 0)
        {
            users.emplace_back(
                -1,
                "",
                "",
                std::nullopt,
                "",
                std::nullopt,
                "",
                "");
        }

        if (data.RowsCount() > 0)
            for (int i = 0; i < data.RowsCount(); i++)
            {
                users.emplace_back(
                    data[i]["UserEntry"],
                    data[i]["UserCode"],
                    data[i]["FirstName"],
                    data[i]["MiddleName"].IsNull() ? std::nullopt : std::optional<std::string>{data[i]["MiddleName"]},
                    data[i]["LastName"],
                    data[i]["SecondLastName"].IsNull() ? std::nullopt : std::optional<std::string>{data[i]["SecondLastName"]},
                    data[i]["Email"],
                    data[i]["Phone"]);
            }

        return users;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(e.what());
    }
};

AuthPayloadModel UserService::CreateUserSession(const UserSessionDTO &dto)
{
    try
    {
        UserPublicModel user;
        bool isValid = false;

        if (dto.Code.has_value() && userRepository->ValidatePassword(UserSearchFilter::Code, dto.Code.value(), dto.Password))
        {
            user = GetUserByCode(dto.Code.value());
            isValid = true;
        }

        if (dto.Email.has_value() && userRepository->ValidatePassword(UserSearchFilter::Email, dto.Email.value(), dto.Password))
        {
            user = GetUserByEmail(dto.Email.value());
            isValid = true;
        }

        if (dto.Phone.has_value() && userRepository->ValidatePassword(UserSearchFilter::Phone, dto.Phone.value(), dto.Password))
        {
            user = GetUserByPhone(dto.Code.value());
            isValid = true;
        }

        if (isValid)
        {
            std::string uuid = UUIDv4::GenerateUUIDv4();
            std::string token = JWT::GenerateToken(uuid, 6000);

            const AuthPayloadModel payload{
                token,
                user};

            return payload;
        }
        else
            throw std::runtime_error("Invalid password");
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[Login exception] ") + e.what());
    }
};

UserPublicModel UserService::GetUserByEntry(const int &iEntry)
{
    try
    {
        DataTable data;

        data = userRepository->ReadByEntry(iEntry);

        if (data.RowsCount() > 1)
            throw std::runtime_error("Inconstence retreiving Users, not exists or exists more than one");

        if (data.RowsCount() == 0)
            return UserPublicModel{
                -1,
                "",
                "",
                std::nullopt,
                "",
                std::nullopt,
                "",
                ""};

        UserPublicModel user{
            data[0]["UserEntry"],
            data[0]["UserCode"],
            data[0]["FirstName"],
            data[0]["MiddleName"].IsNull() ? std::nullopt : std::optional<std::string>{data[0]["MiddleName"]},
            data[0]["LastName"],
            data[0]["SecondLastName"].IsNull() ? std::nullopt : std::optional<std::string>{data[0]["SecondLastName"]},
            data[0]["Email"],
            data[0]["Phone"]};

        return user;
    }
    catch (const std::exception &e)
    {
        std::runtime_error(e.what());
    }
};

UserPublicModel UserService::GetUserByCode(const std::string &code)
{
    try
    {
        DataTable data;

        data = userRepository->ReadByCode(code);

        if (data.RowsCount() > 1)
            throw std::runtime_error("Inconstence retreiving Users, not exists or exists more than one");

        if (data.RowsCount() == 0)
            return UserPublicModel{
                -1,
                "",
                "",
                std::nullopt,
                "",
                std::nullopt,
                "",
                ""};
        
        UserPublicModel user{
            data[0]["UserEntry"],
            data[0]["UserCode"],
            data[0]["FirstName"],
            data[0]["MiddleName"].IsNull() ? std::nullopt : std::optional<std::string>{data[0]["MiddleName"]},
            data[0]["LastName"],
            data[0]["SecondLastName"].IsNull() ? std::nullopt : std::optional<std::string>{data[0]["SecondLastName"]},
            data[0]["Email"],
            data[0]["Phone"]};

        return user;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[GetUser Exception] ") + " " + e.what());
    }
};

UserPublicModel UserService::GetUserByPhone(const std::string &phone)
{
    try
    {
        DataTable data;

        data = userRepository->ReadByPhone(phone);

        if (data.RowsCount() > 1)
            throw std::runtime_error("Inconstence retreiving Users, not exists or exists more than one");
        
        if (data.RowsCount() == 0)
            return UserPublicModel{
                -1,
                "",
                "",
                std::nullopt,
                "",
                std::nullopt,
                "",
                ""};

        UserPublicModel user{
            data[0]["UserEntry"],
            data[0]["UserCode"],
            data[0]["FirstName"],
            data[0]["MiddleName"].IsNull() ? std::nullopt : std::optional<std::string>{data[0]["MiddleName"]},
            data[0]["LastName"],
            data[0]["SecondLastName"].IsNull() ? std::nullopt : std::optional<std::string>{data[0]["SecondLastName"]},
            data[0]["Email"],
            data[0]["Phone"]};

        return user;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[GetUser Exception] ") + " " + e.what());
    }
};

UserPublicModel UserService::GetUserByEmail(const std::string &email)
{
    try
    {
        DataTable data;

        data = userRepository->ReadByEmail(email);

        if (data.RowsCount() > 1)
            throw std::runtime_error("Inconstence retreiving Users, not exists or exists more than one");

        if (data.RowsCount() == 0)
            return UserPublicModel{
                -1,
                "",
                "",
                std::nullopt,
                "",
                std::nullopt,
                "",
                ""};
                
        UserPublicModel user{
            data[0]["UserEntry"],
            data[0]["UserCode"],
            data[0]["FirstName"],
            data[0]["MiddleName"].IsNull() ? std::nullopt : std::optional<std::string>{data[0]["MiddleName"]},
            data[0]["LastName"],
            data[0]["SecondLastName"].IsNull() ? std::nullopt : std::optional<std::string>{data[0]["SecondLastName"]},
            data[0]["Email"],
            data[0]["Phone"]};

        return user;
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string("[GetUser Exception] ") + " " + e.what());
    };
};