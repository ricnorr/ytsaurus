#include "cypress_user_manager.h"

#include "auth_cache.h"
#include "private.h"

#include <yt/yt/client/api/client.h>

#include <library/cpp/yt/logging/logger.h>

namespace NYT::NAuth {

using namespace NYTree;
using namespace NObjectClient;

////////////////////////////////////////////////////////////////////////////////

static constexpr auto& Logger = AuthLogger;

////////////////////////////////////////////////////////////////////////////////

class TCypressUserManager
    : public ICypressUserManager
{
public:
    TCypressUserManager(
        TCypressUserManagerConfigPtr config,
        NApi::IClientPtr client)
        : Config_(std::move(config))
        , Client_(std::move(client))
    { }

    TFuture<TObjectId> CreateUser(const TString& name) override
    {
        YT_LOG_DEBUG("Creating user object (Name: %v)", name);
        NApi::TCreateObjectOptions options;
        options.IgnoreExisting = true;

        auto attributes = CreateEphemeralAttributes();
        attributes->Set("name", name);
        options.Attributes = std::move(attributes);

        return Client_->CreateObject(
            EObjectType::User,
            options);
    }

private:
    const TCypressUserManagerConfigPtr Config_;
    const NApi::IClientPtr Client_;
};

////////////////////////////////////////////////////////////////////////////////

ICypressUserManagerPtr CreateCypressUserManager(
    TCypressUserManagerConfigPtr config,
    NApi::IClientPtr client)
{
    return New<TCypressUserManager>(
        std::move(config),
        std::move(client));
}

////////////////////////////////////////////////////////////////////////////////

class TCachingCypressUserManager
    : public ICypressUserManager
    , private TAuthCache<TString, TObjectId>
{
public:
    TCachingCypressUserManager(
        TCachingCypressUserManagerConfigPtr config,
        ICypressUserManagerPtr CypressUserManager,
        NProfiling::TProfiler profiler)
        : TAuthCache(config->Cache, std::move(profiler))
        , CypressUserManager_(std::move(CypressUserManager))
    { }

    TFuture<TObjectId> CreateUser(const TString& name) override
    {
        return Get(name);
    }

private:
    const ICypressUserManagerPtr CypressUserManager_;

    TFuture<TObjectId> DoGet(const TString& name) noexcept override
    {
        return CypressUserManager_->CreateUser(name);
    }
};

////////////////////////////////////////////////////////////////////////////////

ICypressUserManagerPtr CreateCachingCypressUserManager(
    TCachingCypressUserManagerConfigPtr config,
    ICypressUserManagerPtr userManager,
    NProfiling::TProfiler profiler)
{
    return New<TCachingCypressUserManager>(
        std::move(config),
        std::move(userManager),
        std::move(profiler));
}

////////////////////////////////////////////////////////////////////////////////

} // namespace NYT::NAuth
