#pragma once

#include <memory>
#include "../Repositories/User/UserRepository.hpp"
#include "../Interfaces/IUserRepository.hpp"
#include "../Models/UserPublicModel.hpp"
#include "../Models/AuthPayloadModel.hpp"
#include "../DTOs/UserSessionDTO.hpp"
#include "../DTOs/ChangePasswordDTO.hpp"
#include "../Utils/Hasher.hpp"
#include "../Utils/JWT.hpp"
#include "../Utils/UUIDv4.hpp"
#include "../Enums/UserSearchFilter.hpp"
#include "../DTOs/UserSearchDTO.hpp"

class UserService {
    public: 
        explicit UserService(std::shared_ptr<IUserRepository> userRepository);

        bool AddUser(const CreateUserDTO&);
        UserPublicModel ModifyUser(const UserDTO&);
        bool ModifyPassword(const ChangePasswordDTO &);
        bool ExistsUser(const UserDTO&);
        std::vector<UserPublicModel> SearchUsers(const UserSearchDTO &);
        UserPublicModel GetUserByEntry(const int&);
        UserPublicModel GetUserByCode(const std::string&);
        UserPublicModel GetUserByPhone(const std::string&);
        UserPublicModel GetUserByEmail(const std::string&);
        AuthPayloadModel CreateUserSession(const UserSessionDTO &);

    private:
        std::shared_ptr<IUserRepository> userRepository;

};