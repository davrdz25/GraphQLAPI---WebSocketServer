#pragma once
#include <graphqlservice/GraphQLService.h>
#include <memory>
#include "../Generated/User/QueryObject.h"
#include "UserResolvers/UserQueryResolver.hpp"

namespace graphql::resolvers
{

    class RootQueryResolver
    {
    private:
        std::shared_ptr<UserQueryResolver> _userQ;

    public:
        explicit RootQueryResolver(std::shared_ptr<UserQueryResolver> userQ) noexcept
            : _userQ(std::move(userQ))
        {
        }
        
        graphql::service::AwaitableObject<std::shared_ptr<graphql::user::object::UserQuery>>
        getUser() const
        {
            co_return std::make_shared<graphql::user::object::UserQuery>(_userQ);
        }

        void beginSelectionSet(const graphql::service::SelectionSetParams &) const {}
        void endSelectionSet(const graphql::service::SelectionSetParams &) const {}
    };
}
