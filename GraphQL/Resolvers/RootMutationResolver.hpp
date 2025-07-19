#pragma once
#include <graphqlservice/GraphQLService.h>
#include <memory>
#include "../Generated/User/MutationObject.h"  
#include "UserResolvers/UserMutationResolver.hpp"

namespace graphql::resolvers
{
    class RootMutationResolver
    {
    private:
        std::shared_ptr<UserMutationResolver> _userM;

    public:
        explicit RootMutationResolver(std::shared_ptr<UserMutationResolver> userM) noexcept
            : _userM(std::move(userM))
        {
        }

        [[nodiscard]] service::AwaitableObject<std::shared_ptr<graphql::user::object::UserMutation>>
        applyUser(service::FieldParams &&params) const
        {
            co_return std::make_shared<graphql::user::object::UserMutation>(_userM);
        }

        void beginSelectionSet(const service::SelectionSetParams &) const {}
        void endSelectionSet(const service::SelectionSetParams &) const {}
    };

}
