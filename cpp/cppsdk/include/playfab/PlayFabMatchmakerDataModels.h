#pragma once

#ifdef ENABLE_PLAYFABSERVER_API

#include <gsdkCommonPch.h>
#include <playfab/PlayFabBaseModel.h>

namespace PlayFab
{
    namespace MatchmakerModels
    {
        // Matchmaker Enums
        enum Region
        {
            RegionUSCentral,
            RegionUSEast,
            RegionEUWest,
            RegionSingapore,
            RegionJapan,
            RegionBrazil,
            RegionAustralia
        };

        inline void ToJsonEnum(const Region input, Json::Value& output)
        {
            if (input == RegionUSCentral) output = Json::Value("USCentral");
            if (input == RegionUSEast) output = Json::Value("USEast");
            if (input == RegionEUWest) output = Json::Value("EUWest");
            if (input == RegionSingapore) output = Json::Value("Singapore");
            if (input == RegionJapan) output = Json::Value("Japan");
            if (input == RegionBrazil) output = Json::Value("Brazil");
            if (input == RegionAustralia) output = Json::Value("Australia");
        }
        inline void FromJsonEnum(const Json::Value& input, Region& output)
        {
            if (!input.isString()) return;
            const std::string& inputStr = input.asString();
            if (inputStr == "USCentral") output = RegionUSCentral;
            if (inputStr == "USEast") output = RegionUSEast;
            if (inputStr == "EUWest") output = RegionEUWest;
            if (inputStr == "Singapore") output = RegionSingapore;
            if (inputStr == "Japan") output = RegionJapan;
            if (inputStr == "Brazil") output = RegionBrazil;
            if (inputStr == "Australia") output = RegionAustralia;
        }

        // Matchmaker Classes
        struct AuthUserRequest : public PlayFabRequestCommon
        {
            std::string AuthorizationTicket;

            AuthUserRequest() :
                PlayFabRequestCommon(),
                AuthorizationTicket()
            {}

            AuthUserRequest(const AuthUserRequest& src) :
                PlayFabRequestCommon(),
                AuthorizationTicket(src.AuthorizationTicket)
            {}

            ~AuthUserRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["AuthorizationTicket"], AuthorizationTicket);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_AuthorizationTicket; ToJsonUtilS(AuthorizationTicket, each_AuthorizationTicket); output["AuthorizationTicket"] = each_AuthorizationTicket;
                return output;
            }
        };

        struct AuthUserResponse : public PlayFabResultCommon
        {
            bool Authorized;
            std::string PlayFabId;

            AuthUserResponse() :
                PlayFabResultCommon(),
                Authorized(),
                PlayFabId()
            {}

            AuthUserResponse(const AuthUserResponse& src) :
                PlayFabResultCommon(),
                Authorized(src.Authorized),
                PlayFabId(src.PlayFabId)
            {}

            ~AuthUserResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["Authorized"], Authorized);
                FromJsonUtilS(input["PlayFabId"], PlayFabId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Authorized; ToJsonUtilP(Authorized, each_Authorized); output["Authorized"] = each_Authorized;
                Json::Value each_PlayFabId; ToJsonUtilS(PlayFabId, each_PlayFabId); output["PlayFabId"] = each_PlayFabId;
                return output;
            }
        };

        struct DeregisterGameRequest : public PlayFabRequestCommon
        {
            std::string LobbyId;

            DeregisterGameRequest() :
                PlayFabRequestCommon(),
                LobbyId()
            {}

            DeregisterGameRequest(const DeregisterGameRequest& src) :
                PlayFabRequestCommon(),
                LobbyId(src.LobbyId)
            {}

            ~DeregisterGameRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["LobbyId"], LobbyId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_LobbyId; ToJsonUtilS(LobbyId, each_LobbyId); output["LobbyId"] = each_LobbyId;
                return output;
            }
        };

        struct DeregisterGameResponse : public PlayFabResultCommon
        {

            DeregisterGameResponse() :
                PlayFabResultCommon()
            {}

            DeregisterGameResponse(const DeregisterGameResponse&) :
                PlayFabResultCommon()
            {}

            ~DeregisterGameResponse() { }

            void FromJson(Json::Value&) override
            {
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                return output;
            }
        };

        struct ItemInstance : public PlayFabBaseModel
        {
            std::string Annotation;
            std::list<std::string> BundleContents;
            std::string BundleParent;
            std::string CatalogVersion;
            std::map<std::string, std::string> CustomData;
            std::string DisplayName;
            Boxed<time_t> Expiration;
            std::string ItemClass;
            std::string ItemId;
            std::string ItemInstanceId;
            Boxed<time_t> PurchaseDate;
            Boxed<Int32> RemainingUses;
            std::string UnitCurrency;
            Uint32 UnitPrice;
            Boxed<Int32> UsesIncrementedBy;

            ItemInstance() :
                PlayFabBaseModel(),
                Annotation(),
                BundleContents(),
                BundleParent(),
                CatalogVersion(),
                CustomData(),
                DisplayName(),
                Expiration(),
                ItemClass(),
                ItemId(),
                ItemInstanceId(),
                PurchaseDate(),
                RemainingUses(),
                UnitCurrency(),
                UnitPrice(),
                UsesIncrementedBy()
            {}

            ItemInstance(const ItemInstance& src) :
                PlayFabBaseModel(),
                Annotation(src.Annotation),
                BundleContents(src.BundleContents),
                BundleParent(src.BundleParent),
                CatalogVersion(src.CatalogVersion),
                CustomData(src.CustomData),
                DisplayName(src.DisplayName),
                Expiration(src.Expiration),
                ItemClass(src.ItemClass),
                ItemId(src.ItemId),
                ItemInstanceId(src.ItemInstanceId),
                PurchaseDate(src.PurchaseDate),
                RemainingUses(src.RemainingUses),
                UnitCurrency(src.UnitCurrency),
                UnitPrice(src.UnitPrice),
                UsesIncrementedBy(src.UsesIncrementedBy)
            {}

            ~ItemInstance() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["Annotation"], Annotation);
                FromJsonUtilS(input["BundleContents"], BundleContents);
                FromJsonUtilS(input["BundleParent"], BundleParent);
                FromJsonUtilS(input["CatalogVersion"], CatalogVersion);
                FromJsonUtilS(input["CustomData"], CustomData);
                FromJsonUtilS(input["DisplayName"], DisplayName);
                FromJsonUtilT(input["Expiration"], Expiration);
                FromJsonUtilS(input["ItemClass"], ItemClass);
                FromJsonUtilS(input["ItemId"], ItemId);
                FromJsonUtilS(input["ItemInstanceId"], ItemInstanceId);
                FromJsonUtilT(input["PurchaseDate"], PurchaseDate);
                FromJsonUtilP(input["RemainingUses"], RemainingUses);
                FromJsonUtilS(input["UnitCurrency"], UnitCurrency);
                FromJsonUtilP(input["UnitPrice"], UnitPrice);
                FromJsonUtilP(input["UsesIncrementedBy"], UsesIncrementedBy);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Annotation; ToJsonUtilS(Annotation, each_Annotation); output["Annotation"] = each_Annotation;
                Json::Value each_BundleContents; ToJsonUtilS(BundleContents, each_BundleContents); output["BundleContents"] = each_BundleContents;
                Json::Value each_BundleParent; ToJsonUtilS(BundleParent, each_BundleParent); output["BundleParent"] = each_BundleParent;
                Json::Value each_CatalogVersion; ToJsonUtilS(CatalogVersion, each_CatalogVersion); output["CatalogVersion"] = each_CatalogVersion;
                Json::Value each_CustomData; ToJsonUtilS(CustomData, each_CustomData); output["CustomData"] = each_CustomData;
                Json::Value each_DisplayName; ToJsonUtilS(DisplayName, each_DisplayName); output["DisplayName"] = each_DisplayName;
                Json::Value each_Expiration; ToJsonUtilT(Expiration, each_Expiration); output["Expiration"] = each_Expiration;
                Json::Value each_ItemClass; ToJsonUtilS(ItemClass, each_ItemClass); output["ItemClass"] = each_ItemClass;
                Json::Value each_ItemId; ToJsonUtilS(ItemId, each_ItemId); output["ItemId"] = each_ItemId;
                Json::Value each_ItemInstanceId; ToJsonUtilS(ItemInstanceId, each_ItemInstanceId); output["ItemInstanceId"] = each_ItemInstanceId;
                Json::Value each_PurchaseDate; ToJsonUtilT(PurchaseDate, each_PurchaseDate); output["PurchaseDate"] = each_PurchaseDate;
                Json::Value each_RemainingUses; ToJsonUtilP(RemainingUses, each_RemainingUses); output["RemainingUses"] = each_RemainingUses;
                Json::Value each_UnitCurrency; ToJsonUtilS(UnitCurrency, each_UnitCurrency); output["UnitCurrency"] = each_UnitCurrency;
                Json::Value each_UnitPrice; ToJsonUtilP(UnitPrice, each_UnitPrice); output["UnitPrice"] = each_UnitPrice;
                Json::Value each_UsesIncrementedBy; ToJsonUtilP(UsesIncrementedBy, each_UsesIncrementedBy); output["UsesIncrementedBy"] = each_UsesIncrementedBy;
                return output;
            }
        };

        struct PlayerJoinedRequest : public PlayFabRequestCommon
        {
            std::string LobbyId;
            std::string PlayFabId;

            PlayerJoinedRequest() :
                PlayFabRequestCommon(),
                LobbyId(),
                PlayFabId()
            {}

            PlayerJoinedRequest(const PlayerJoinedRequest& src) :
                PlayFabRequestCommon(),
                LobbyId(src.LobbyId),
                PlayFabId(src.PlayFabId)
            {}

            ~PlayerJoinedRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["LobbyId"], LobbyId);
                FromJsonUtilS(input["PlayFabId"], PlayFabId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_LobbyId; ToJsonUtilS(LobbyId, each_LobbyId); output["LobbyId"] = each_LobbyId;
                Json::Value each_PlayFabId; ToJsonUtilS(PlayFabId, each_PlayFabId); output["PlayFabId"] = each_PlayFabId;
                return output;
            }
        };

        struct PlayerJoinedResponse : public PlayFabResultCommon
        {

            PlayerJoinedResponse() :
                PlayFabResultCommon()
            {}

            PlayerJoinedResponse(const PlayerJoinedResponse&) :
                PlayFabResultCommon()
            {}

            ~PlayerJoinedResponse() { }

            void FromJson(Json::Value&) override
            {
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                return output;
            }
        };

        struct PlayerLeftRequest : public PlayFabRequestCommon
        {
            std::string LobbyId;
            std::string PlayFabId;

            PlayerLeftRequest() :
                PlayFabRequestCommon(),
                LobbyId(),
                PlayFabId()
            {}

            PlayerLeftRequest(const PlayerLeftRequest& src) :
                PlayFabRequestCommon(),
                LobbyId(src.LobbyId),
                PlayFabId(src.PlayFabId)
            {}

            ~PlayerLeftRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["LobbyId"], LobbyId);
                FromJsonUtilS(input["PlayFabId"], PlayFabId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_LobbyId; ToJsonUtilS(LobbyId, each_LobbyId); output["LobbyId"] = each_LobbyId;
                Json::Value each_PlayFabId; ToJsonUtilS(PlayFabId, each_PlayFabId); output["PlayFabId"] = each_PlayFabId;
                return output;
            }
        };

        struct PlayerLeftResponse : public PlayFabResultCommon
        {

            PlayerLeftResponse() :
                PlayFabResultCommon()
            {}

            PlayerLeftResponse(const PlayerLeftResponse&) :
                PlayFabResultCommon()
            {}

            ~PlayerLeftResponse() { }

            void FromJson(Json::Value&) override
            {
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                return output;
            }
        };

        struct RegisterGameRequest : public PlayFabRequestCommon
        {
            std::string Build;
            std::string GameMode;
            std::string LobbyId;
            Region pfRegion;
            std::string ServerHost;
            std::string ServerIPV6Address;
            std::string ServerPort;
            std::map<std::string, std::string> Tags;

            RegisterGameRequest() :
                PlayFabRequestCommon(),
                Build(),
                GameMode(),
                LobbyId(),
                pfRegion(),
                ServerHost(),
                ServerIPV6Address(),
                ServerPort(),
                Tags()
            {}

            RegisterGameRequest(const RegisterGameRequest& src) :
                PlayFabRequestCommon(),
                Build(src.Build),
                GameMode(src.GameMode),
                LobbyId(src.LobbyId),
                pfRegion(src.pfRegion),
                ServerHost(src.ServerHost),
                ServerIPV6Address(src.ServerIPV6Address),
                ServerPort(src.ServerPort),
                Tags(src.Tags)
            {}

            ~RegisterGameRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["Build"], Build);
                FromJsonUtilS(input["GameMode"], GameMode);
                FromJsonUtilS(input["LobbyId"], LobbyId);
                FromJsonEnum(input["pfRegion"], pfRegion);
                FromJsonUtilS(input["ServerHost"], ServerHost);
                FromJsonUtilS(input["ServerIPV6Address"], ServerIPV6Address);
                FromJsonUtilS(input["ServerPort"], ServerPort);
                FromJsonUtilS(input["Tags"], Tags);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Build; ToJsonUtilS(Build, each_Build); output["Build"] = each_Build;
                Json::Value each_GameMode; ToJsonUtilS(GameMode, each_GameMode); output["GameMode"] = each_GameMode;
                Json::Value each_LobbyId; ToJsonUtilS(LobbyId, each_LobbyId); output["LobbyId"] = each_LobbyId;
                Json::Value each_pfRegion; ToJsonEnum(pfRegion, each_pfRegion); output["Region"] = each_pfRegion;
                Json::Value each_ServerHost; ToJsonUtilS(ServerHost, each_ServerHost); output["ServerHost"] = each_ServerHost;
                Json::Value each_ServerIPV6Address; ToJsonUtilS(ServerIPV6Address, each_ServerIPV6Address); output["ServerIPV6Address"] = each_ServerIPV6Address;
                Json::Value each_ServerPort; ToJsonUtilS(ServerPort, each_ServerPort); output["ServerPort"] = each_ServerPort;
                Json::Value each_Tags; ToJsonUtilS(Tags, each_Tags); output["Tags"] = each_Tags;
                return output;
            }
        };

        struct RegisterGameResponse : public PlayFabResultCommon
        {
            std::string LobbyId;

            RegisterGameResponse() :
                PlayFabResultCommon(),
                LobbyId()
            {}

            RegisterGameResponse(const RegisterGameResponse& src) :
                PlayFabResultCommon(),
                LobbyId(src.LobbyId)
            {}

            ~RegisterGameResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["LobbyId"], LobbyId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_LobbyId; ToJsonUtilS(LobbyId, each_LobbyId); output["LobbyId"] = each_LobbyId;
                return output;
            }
        };

        struct StartGameRequest : public PlayFabRequestCommon
        {
            std::string Build;
            std::string CustomCommandLineData;
            std::string ExternalMatchmakerEventEndpoint;
            std::string GameMode;
            Region pfRegion;

            StartGameRequest() :
                PlayFabRequestCommon(),
                Build(),
                CustomCommandLineData(),
                ExternalMatchmakerEventEndpoint(),
                GameMode(),
                pfRegion()
            {}

            StartGameRequest(const StartGameRequest& src) :
                PlayFabRequestCommon(),
                Build(src.Build),
                CustomCommandLineData(src.CustomCommandLineData),
                ExternalMatchmakerEventEndpoint(src.ExternalMatchmakerEventEndpoint),
                GameMode(src.GameMode),
                pfRegion(src.pfRegion)
            {}

            ~StartGameRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["Build"], Build);
                FromJsonUtilS(input["CustomCommandLineData"], CustomCommandLineData);
                FromJsonUtilS(input["ExternalMatchmakerEventEndpoint"], ExternalMatchmakerEventEndpoint);
                FromJsonUtilS(input["GameMode"], GameMode);
                FromJsonEnum(input["pfRegion"], pfRegion);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Build; ToJsonUtilS(Build, each_Build); output["Build"] = each_Build;
                Json::Value each_CustomCommandLineData; ToJsonUtilS(CustomCommandLineData, each_CustomCommandLineData); output["CustomCommandLineData"] = each_CustomCommandLineData;
                Json::Value each_ExternalMatchmakerEventEndpoint; ToJsonUtilS(ExternalMatchmakerEventEndpoint, each_ExternalMatchmakerEventEndpoint); output["ExternalMatchmakerEventEndpoint"] = each_ExternalMatchmakerEventEndpoint;
                Json::Value each_GameMode; ToJsonUtilS(GameMode, each_GameMode); output["GameMode"] = each_GameMode;
                Json::Value each_pfRegion; ToJsonEnum(pfRegion, each_pfRegion); output["Region"] = each_pfRegion;
                return output;
            }
        };

        struct StartGameResponse : public PlayFabResultCommon
        {
            std::string GameID;
            std::string ServerHostname;
            std::string ServerIPV6Address;
            Uint32 ServerPort;

            StartGameResponse() :
                PlayFabResultCommon(),
                GameID(),
                ServerHostname(),
                ServerIPV6Address(),
                ServerPort()
            {}

            StartGameResponse(const StartGameResponse& src) :
                PlayFabResultCommon(),
                GameID(src.GameID),
                ServerHostname(src.ServerHostname),
                ServerIPV6Address(src.ServerIPV6Address),
                ServerPort(src.ServerPort)
            {}

            ~StartGameResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["GameID"], GameID);
                FromJsonUtilS(input["ServerHostname"], ServerHostname);
                FromJsonUtilS(input["ServerIPV6Address"], ServerIPV6Address);
                FromJsonUtilP(input["ServerPort"], ServerPort);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_GameID; ToJsonUtilS(GameID, each_GameID); output["GameID"] = each_GameID;
                Json::Value each_ServerHostname; ToJsonUtilS(ServerHostname, each_ServerHostname); output["ServerHostname"] = each_ServerHostname;
                Json::Value each_ServerIPV6Address; ToJsonUtilS(ServerIPV6Address, each_ServerIPV6Address); output["ServerIPV6Address"] = each_ServerIPV6Address;
                Json::Value each_ServerPort; ToJsonUtilP(ServerPort, each_ServerPort); output["ServerPort"] = each_ServerPort;
                return output;
            }
        };

        struct UserInfoRequest : public PlayFabRequestCommon
        {
            Int32 MinCatalogVersion;
            std::string PlayFabId;

            UserInfoRequest() :
                PlayFabRequestCommon(),
                MinCatalogVersion(),
                PlayFabId()
            {}

            UserInfoRequest(const UserInfoRequest& src) :
                PlayFabRequestCommon(),
                MinCatalogVersion(src.MinCatalogVersion),
                PlayFabId(src.PlayFabId)
            {}

            ~UserInfoRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["MinCatalogVersion"], MinCatalogVersion);
                FromJsonUtilS(input["PlayFabId"], PlayFabId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_MinCatalogVersion; ToJsonUtilP(MinCatalogVersion, each_MinCatalogVersion); output["MinCatalogVersion"] = each_MinCatalogVersion;
                Json::Value each_PlayFabId; ToJsonUtilS(PlayFabId, each_PlayFabId); output["PlayFabId"] = each_PlayFabId;
                return output;
            }
        };

        struct VirtualCurrencyRechargeTime : public PlayFabBaseModel
        {
            Int32 RechargeMax;
            time_t RechargeTime;
            Int32 SecondsToRecharge;

            VirtualCurrencyRechargeTime() :
                PlayFabBaseModel(),
                RechargeMax(),
                RechargeTime(),
                SecondsToRecharge()
            {}

            VirtualCurrencyRechargeTime(const VirtualCurrencyRechargeTime& src) :
                PlayFabBaseModel(),
                RechargeMax(src.RechargeMax),
                RechargeTime(src.RechargeTime),
                SecondsToRecharge(src.SecondsToRecharge)
            {}

            ~VirtualCurrencyRechargeTime() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["RechargeMax"], RechargeMax);
                FromJsonUtilT(input["RechargeTime"], RechargeTime);
                FromJsonUtilP(input["SecondsToRecharge"], SecondsToRecharge);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_RechargeMax; ToJsonUtilP(RechargeMax, each_RechargeMax); output["RechargeMax"] = each_RechargeMax;
                Json::Value each_RechargeTime; ToJsonUtilT(RechargeTime, each_RechargeTime); output["RechargeTime"] = each_RechargeTime;
                Json::Value each_SecondsToRecharge; ToJsonUtilP(SecondsToRecharge, each_SecondsToRecharge); output["SecondsToRecharge"] = each_SecondsToRecharge;
                return output;
            }
        };

        struct UserInfoResponse : public PlayFabResultCommon
        {
            std::list<ItemInstance> Inventory;
            bool IsDeveloper;
            std::string PlayFabId;
            std::string SteamId;
            std::string TitleDisplayName;
            std::string Username;
            std::map<std::string, Int32> VirtualCurrency;
            std::map<std::string, VirtualCurrencyRechargeTime> VirtualCurrencyRechargeTimes;

            UserInfoResponse() :
                PlayFabResultCommon(),
                Inventory(),
                IsDeveloper(),
                PlayFabId(),
                SteamId(),
                TitleDisplayName(),
                Username(),
                VirtualCurrency(),
                VirtualCurrencyRechargeTimes()
            {}

            UserInfoResponse(const UserInfoResponse& src) :
                PlayFabResultCommon(),
                Inventory(src.Inventory),
                IsDeveloper(src.IsDeveloper),
                PlayFabId(src.PlayFabId),
                SteamId(src.SteamId),
                TitleDisplayName(src.TitleDisplayName),
                Username(src.Username),
                VirtualCurrency(src.VirtualCurrency),
                VirtualCurrencyRechargeTimes(src.VirtualCurrencyRechargeTimes)
            {}

            ~UserInfoResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Inventory"], Inventory);
                FromJsonUtilP(input["IsDeveloper"], IsDeveloper);
                FromJsonUtilS(input["PlayFabId"], PlayFabId);
                FromJsonUtilS(input["SteamId"], SteamId);
                FromJsonUtilS(input["TitleDisplayName"], TitleDisplayName);
                FromJsonUtilS(input["Username"], Username);
                FromJsonUtilP(input["VirtualCurrency"], VirtualCurrency);
                FromJsonUtilO(input["VirtualCurrencyRechargeTimes"], VirtualCurrencyRechargeTimes);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Inventory; ToJsonUtilO(Inventory, each_Inventory); output["Inventory"] = each_Inventory;
                Json::Value each_IsDeveloper; ToJsonUtilP(IsDeveloper, each_IsDeveloper); output["IsDeveloper"] = each_IsDeveloper;
                Json::Value each_PlayFabId; ToJsonUtilS(PlayFabId, each_PlayFabId); output["PlayFabId"] = each_PlayFabId;
                Json::Value each_SteamId; ToJsonUtilS(SteamId, each_SteamId); output["SteamId"] = each_SteamId;
                Json::Value each_TitleDisplayName; ToJsonUtilS(TitleDisplayName, each_TitleDisplayName); output["TitleDisplayName"] = each_TitleDisplayName;
                Json::Value each_Username; ToJsonUtilS(Username, each_Username); output["Username"] = each_Username;
                Json::Value each_VirtualCurrency; ToJsonUtilP(VirtualCurrency, each_VirtualCurrency); output["VirtualCurrency"] = each_VirtualCurrency;
                Json::Value each_VirtualCurrencyRechargeTimes; ToJsonUtilO(VirtualCurrencyRechargeTimes, each_VirtualCurrencyRechargeTimes); output["VirtualCurrencyRechargeTimes"] = each_VirtualCurrencyRechargeTimes;
                return output;
            }
        };

    }
}

#endif
