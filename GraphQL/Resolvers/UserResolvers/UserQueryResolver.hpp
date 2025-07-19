#pragma once

#include <graphqlservice/GraphQLService.h>
#include "../Generated/User/UserQueryObject.h"
#include "UserPublicResolver.hpp"
#include "../../Services/User/UserService.hpp"
#include "../../DTOs/UserDTO.hpp"
#include "../../DTOs/UserSearchDTO.hpp"


#include <memory>
#include <vector>

class UserQueryResolver
{
public:
    explicit UserQueryResolver(std::shared_ptr<UserService> userService)
        : m_userService(std::move(userService)) {}

    graphql::service::AwaitableObject<std::shared_ptr<graphql::user::object::UserPublic>> getGetUser(graphql::service::FieldParams &&params, std::optional<int> entry, std::optional<std::string> code, std::optional<std::string> email, std::optional<std::string> phone) const
    {
        try
        {
            const int countParams = entry.has_value() + code.has_value() + email.has_value() + phone.has_value();

            if (countParams == 0)
                throw std::runtime_error("A parameter must be sent");

            if (countParams == 0)
                throw std::runtime_error("Too many parameters");

            UserPublicModel model;

            if (entry.has_value())
            {
                model = m_userService->GetUserByEntry(entry.value());

                std::shared_ptr<UserPublicResolver> resolver = std::make_shared<UserPublicResolver>(model);
                co_return std::make_shared<graphql::user::object::UserPublic>(std::move(resolver));
            }

            if (code.has_value())
            {
                model = m_userService->GetUserByCode(code.value());

                std::shared_ptr<UserPublicResolver> resolver = std::make_shared<UserPublicResolver>(model);
                co_return std::make_shared<graphql::user::object::UserPublic>(std::move(resolver));
            }

            if (email.has_value())
            {
                model = m_userService->GetUserByEmail(email.value());

                std::shared_ptr<UserPublicResolver> resolver = std::make_shared<UserPublicResolver>(model);
                co_return std::make_shared<graphql::user::object::UserPublic>(std::move(resolver));
            }

            if (phone.has_value())
            {
                model = m_userService->GetUserByPhone(phone.value());

                std::shared_ptr<UserPublicResolver> resolver = std::make_shared<UserPublicResolver>(model);
                co_return std::make_shared<graphql::user::object::UserPublic>(std::move(resolver));
            }
        }
        catch (const std::exception &e)
        {
            throw graphql::service::schema_exception({graphql::service::schema_error{
                .message = e.what(),
                .path = {"User", "GetUser"}}});
        }
    }

    graphql::service::AwaitableObject<std::vector<std::shared_ptr<graphql::user::object::UserPublic>>> getGetUsers(graphql::service::FieldParams &&params, const graphql::user::SearchUser &userFilter) const
    {
        try
        {
            std::vector<std::shared_ptr<graphql::user::object::UserPublic>> gqlUsers;
            
            UserSearchDTO filter = {
                userFilter.ExactValues,
                userFilter.FirstName,
                userFilter.MiddleName,
                userFilter.LastName,
                userFilter.SecondLastName,
            };

            std::vector<UserPublicModel> model = m_userService->SearchUsers(filter);

            for (const UserPublicModel &user : model)
            {
                std::shared_ptr<UserPublicResolver> resolver = std::make_shared<UserPublicResolver>(user);
                gqlUsers.emplace_back(std::make_shared<graphql::user::object::UserPublic>(std::move(resolver)));
            }

            co_return gqlUsers;
        }
        catch (const std::exception &e)
        {
            throw graphql::service::schema_exception({graphql::service::schema_error{
                .message = e.what(),
                .path = {"User", "GetUsers"}}});
        }
    }

private:
    std::shared_ptr<UserService> m_userService;
};