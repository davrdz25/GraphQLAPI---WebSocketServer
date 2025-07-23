#pragma once

#include <graphqlservice/GraphQLService.h>

#include <memory>
#include <optional>
#include <string>
#include "UserPublicResolver.hpp"
#include "AuthResolver.hpp"
#include "../../Generated/User/UserMutationObject.h"
#include "../../../Services/User/UserService.hpp"
#include "../../../DTOs/CreateUserDTO.hpp"
#include "../../../DTOs/ChangePasswordDTO.hpp"
#include "../../../Models/UserModel.hpp"
#include "../../../Models/UserPublicModel.hpp"
#include "../../../Models/AuthPayloadModel.hpp"

class UserMutationResolver
{
private:
    std::shared_ptr<UserService> m_userService;

public:
    explicit UserMutationResolver(std::shared_ptr<UserService> userService) noexcept
        : m_userService(std::move(userService))
    {
    }

    graphql::service::AwaitableObject<std::shared_ptr<graphql::user::object::UserPublic>> getCreateUser(graphql::service::FieldParams &&params, const graphql::user::CreateUserInput &newUser) const
    {
        try
        {
            if (newUser.Code == "")
                throw std::runtime_error("Code cannot be empty");

            if (newUser.FirstName == "")
                throw std::runtime_error("First name cannot be empty");

            if (newUser.LastName == "")
                throw std::runtime_error("Last name cannot be empty");

            if (newUser.Email == "")
                throw std::runtime_error("Email cannot be empty");

            if (newUser.Phone == "")
                throw std::runtime_error("Phone cannot be empty");

            if (newUser.Password == "")
                throw std::runtime_error("Password cannot be empty");

            const CreateUserDTO userParam{
                newUser.Code,
                newUser.FirstName,
                newUser.MiddleName,
                newUser.LastName,
                newUser.SecondLastName,
                newUser.Email,
                newUser.Phone,
                newUser.Password};

            m_userService->AddUser(userParam);

            auto userModel = m_userService->GetUserByCode(newUser.Code);

            auto resolver = std::make_shared<UserPublicResolver>(userModel);

            co_return std::make_shared<graphql::user::object::UserPublic>(std::move(resolver));
        }
        catch (const std::exception &e)
        {
            throw graphql::service::schema_exception({graphql::service::schema_error{
                .message = e.what(),
                .path = {"User", "CreateUser"}}});
        }
    }

    graphql::service::AwaitableObject<std::shared_ptr<graphql::user::object::UserPublic>> getUpdateUser(graphql::service::FieldParams &&params, const graphql::user::UpdateUserInput &userChanges) const
    {
        try
        {
            if (!userChanges.FirstName.has_value() &&
                !userChanges.MiddleName.has_value() &&
                !userChanges.LastName.has_value() &&
                !userChanges.SecondLastName.has_value() &&
                !userChanges.Email.has_value() &&
                !userChanges.Phone.has_value())
                throw std::runtime_error("No fields to update");

            const UserDTO user{
                userChanges.Entry,
                userChanges.Code,
                userChanges.FirstName,
                userChanges.MiddleName,
                userChanges.LastName,
                userChanges.SecondLastName,
                userChanges.Email,
                userChanges.Phone,
            };

            auto userModel = m_userService->ModifyUser(user);

            auto resolver = std::make_shared<UserPublicResolver>(userModel);

            co_return std::make_shared<graphql::user::object::UserPublic>(std::move(resolver));
        }
        catch (const std::exception &e)
        {
            throw graphql::service::schema_exception({graphql::service::schema_error{
                .message = e.what(),
                .path = {"User", "UpdateUser"}}});
        }
    }

    graphql::service::AwaitableScalar<bool> getModifyPassword(graphql::service::FieldParams &&params, const graphql::user::ChangePasswordInput &user) const
    {
        try
        {
            const int sentParams = user.Code.has_value() + user.Email.has_value() + user.Phone.has_value();

            if (sentParams == 0)
                throw std::runtime_error("Al menos uno de los campos (Email, Code, Phone) debe de ser enviado");

            if (sentParams > 1)
                throw std::runtime_error("Solo uno de los campos (Email, Code, Phone) debe de ser enviado");
            if (user.Code == "")
                throw std::runtime_error("Code cannot be empty");

            if (user.OldPassword == user.NewPassword)
                throw std::runtime_error("New password not to be equal than old password");

            const ChangePasswordDTO newPass = {
                user.Code,
                user.Email,
                user.Phone,
                user.OldPassword,
                user.NewPassword};

            if (user.Code.has_value())
                return m_userService->ModifyPassword(newPass);

            return false;
        }
        catch (const std::exception &e)
        {
            throw graphql::service::schema_exception({graphql::service::schema_error{
                .message = e.what(),
                .path = {"User", "ModifyPassword"}}});
        }
    }

    graphql::service::AwaitableObject<std::shared_ptr<graphql::user::object::AuthPayload>> getLoginUser(graphql::service::FieldParams &&params, const graphql::user::UserSession &dto) const
    {
        try
        {
            const int sentParams = dto.Code.has_value() + dto.Email.has_value() + dto.Phone.has_value();

            if (sentParams == 0)
                throw std::runtime_error("Al menos uno de los campos (Email, Code, Phone) debe de ser enviado");

            if (sentParams > 1)
                throw std::runtime_error("Solo uno de los campos (Email, Code, Phone) debe de ser enviado");

            const UserSessionDTO userSession{
                dto.Code,
                dto.Email,
                dto.Phone,
                dto.Password};

            auto model = m_userService->CreateUserSession(userSession);
            auto resolver = std::make_shared<AuthPayloadResolver>(model);

            co_return std::make_shared<graphql::user::object::AuthPayload>(std::move(resolver));
        }
        catch (const std::exception &e)
        {
            throw graphql::service::schema_exception({graphql::service::schema_error{
                .message = e.what(),
                .path = {"User", "LoginUser"}}});
        }
    };
};