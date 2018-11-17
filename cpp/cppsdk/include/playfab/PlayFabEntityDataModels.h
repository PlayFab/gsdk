#pragma once

#ifdef ENABLE_PLAYFABENTITY_API

#include <gsdkCommonPch.h>
#include <playfab/PlayFabBaseModel.h>

namespace PlayFab
{
    namespace EntityModels
    {
        // Entity Enums
        enum EffectType
        {
            EffectTypeAllow,
            EffectTypeDeny
        };

        inline void ToJsonEnum(const EffectType input, Json::Value& output)
        {
            if (input == EffectTypeAllow) output = Json::Value("Allow");
            if (input == EffectTypeDeny) output = Json::Value("Deny");
        }
        inline void FromJsonEnum(const Json::Value& input, EffectType& output)
        {
            if (!input.isString()) return;
            const std::string& inputStr = input.asString();
            if (inputStr == "Allow") output = EffectTypeAllow;
            if (inputStr == "Deny") output = EffectTypeDeny;
        }

        enum EntityTypes
        {
            EntityTypestitle,
            EntityTypesmaster_player_account,
            EntityTypestitle_player_account,
            EntityTypescharacter,
            EntityTypesgroup
        };

        inline void ToJsonEnum(const EntityTypes input, Json::Value& output)
        {
            if (input == EntityTypestitle) output = Json::Value("title");
            if (input == EntityTypesmaster_player_account) output = Json::Value("master_player_account");
            if (input == EntityTypestitle_player_account) output = Json::Value("title_player_account");
            if (input == EntityTypescharacter) output = Json::Value("character");
            if (input == EntityTypesgroup) output = Json::Value("group");
        }
        inline void FromJsonEnum(const Json::Value& input, EntityTypes& output)
        {
            if (!input.isString()) return;
            const std::string& inputStr = input.asString();
            if (inputStr == "title") output = EntityTypestitle;
            if (inputStr == "master_player_account") output = EntityTypesmaster_player_account;
            if (inputStr == "title_player_account") output = EntityTypestitle_player_account;
            if (inputStr == "character") output = EntityTypescharacter;
            if (inputStr == "group") output = EntityTypesgroup;
        }

        enum OperationTypes
        {
            OperationTypesCreated,
            OperationTypesUpdated,
            OperationTypesDeleted,
            OperationTypesNone
        };

        inline void ToJsonEnum(const OperationTypes input, Json::Value& output)
        {
            if (input == OperationTypesCreated) output = Json::Value("Created");
            if (input == OperationTypesUpdated) output = Json::Value("Updated");
            if (input == OperationTypesDeleted) output = Json::Value("Deleted");
            if (input == OperationTypesNone) output = Json::Value("None");
        }
        inline void FromJsonEnum(const Json::Value& input, OperationTypes& output)
        {
            if (!input.isString()) return;
            const std::string& inputStr = input.asString();
            if (inputStr == "Created") output = OperationTypesCreated;
            if (inputStr == "Updated") output = OperationTypesUpdated;
            if (inputStr == "Deleted") output = OperationTypesDeleted;
            if (inputStr == "None") output = OperationTypesNone;
        }

        // Entity Classes
        struct EntityKey : public PlayFabBaseModel
        {
            std::string Id;
            Boxed<EntityTypes> Type;
            std::string TypeString;

            EntityKey() :
                PlayFabBaseModel(),
                Id(),
                Type(),
                TypeString()
            {}

            EntityKey(const EntityKey& src) :
                PlayFabBaseModel(),
                Id(src.Id),
                Type(src.Type),
                TypeString(src.TypeString)
            {}

            ~EntityKey() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["Id"], Id);
                FromJsonUtilE(input["Type"], Type);
                FromJsonUtilS(input["TypeString"], TypeString);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Id; ToJsonUtilS(Id, each_Id); output["Id"] = each_Id;
                Json::Value each_Type; ToJsonUtilE(Type, each_Type); output["Type"] = each_Type;
                Json::Value each_TypeString; ToJsonUtilS(TypeString, each_TypeString); output["TypeString"] = each_TypeString;
                return output;
            }
        };

        struct AbortFileUploadsRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            std::list<std::string> FileNames;
            Boxed<Int32> ProfileVersion;

            AbortFileUploadsRequest() :
                PlayFabRequestCommon(),
                Entity(),
                FileNames(),
                ProfileVersion()
            {}

            AbortFileUploadsRequest(const AbortFileUploadsRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                FileNames(src.FileNames),
                ProfileVersion(src.ProfileVersion)
            {}

            ~AbortFileUploadsRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilS(input["FileNames"], FileNames);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_FileNames; ToJsonUtilS(FileNames, each_FileNames); output["FileNames"] = each_FileNames;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                return output;
            }
        };

        struct AbortFileUploadsResponse : public PlayFabResultCommon
        {
            Boxed<EntityKey> Entity;
            Int32 ProfileVersion;

            AbortFileUploadsResponse() :
                PlayFabResultCommon(),
                Entity(),
                ProfileVersion()
            {}

            AbortFileUploadsResponse(const AbortFileUploadsResponse& src) :
                PlayFabResultCommon(),
                Entity(src.Entity),
                ProfileVersion(src.ProfileVersion)
            {}

            ~AbortFileUploadsResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                return output;
            }
        };

        struct AcceptGroupApplicationRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            EntityKey Group;

            AcceptGroupApplicationRequest() :
                PlayFabRequestCommon(),
                Entity(),
                Group()
            {}

            AcceptGroupApplicationRequest(const AcceptGroupApplicationRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                Group(src.Group)
            {}

            ~AcceptGroupApplicationRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct AcceptGroupInvitationRequest : public PlayFabRequestCommon
        {
            Boxed<EntityKey> Entity;
            EntityKey Group;

            AcceptGroupInvitationRequest() :
                PlayFabRequestCommon(),
                Entity(),
                Group()
            {}

            AcceptGroupInvitationRequest(const AcceptGroupInvitationRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                Group(src.Group)
            {}

            ~AcceptGroupInvitationRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct AddMembersRequest : public PlayFabRequestCommon
        {
            EntityKey Group;
            std::list<EntityKey> Members;
            std::string RoleId;

            AddMembersRequest() :
                PlayFabRequestCommon(),
                Group(),
                Members(),
                RoleId()
            {}

            AddMembersRequest(const AddMembersRequest& src) :
                PlayFabRequestCommon(),
                Group(src.Group),
                Members(src.Members),
                RoleId(src.RoleId)
            {}

            ~AddMembersRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilO(input["Members"], Members);
                FromJsonUtilS(input["RoleId"], RoleId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_Members; ToJsonUtilO(Members, each_Members); output["Members"] = each_Members;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                return output;
            }
        };

        struct ApplyToGroupRequest : public PlayFabRequestCommon
        {
            Boxed<bool> AutoAcceptOutstandingInvite;
            Boxed<EntityKey> Entity;
            EntityKey Group;

            ApplyToGroupRequest() :
                PlayFabRequestCommon(),
                AutoAcceptOutstandingInvite(),
                Entity(),
                Group()
            {}

            ApplyToGroupRequest(const ApplyToGroupRequest& src) :
                PlayFabRequestCommon(),
                AutoAcceptOutstandingInvite(src.AutoAcceptOutstandingInvite),
                Entity(src.Entity),
                Group(src.Group)
            {}

            ~ApplyToGroupRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["AutoAcceptOutstandingInvite"], AutoAcceptOutstandingInvite);
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_AutoAcceptOutstandingInvite; ToJsonUtilP(AutoAcceptOutstandingInvite, each_AutoAcceptOutstandingInvite); output["AutoAcceptOutstandingInvite"] = each_AutoAcceptOutstandingInvite;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct EntityWithLineage : public PlayFabBaseModel
        {
            Boxed<EntityKey> Key;
            std::map<std::string, EntityKey> Lineage;

            EntityWithLineage() :
                PlayFabBaseModel(),
                Key(),
                Lineage()
            {}

            EntityWithLineage(const EntityWithLineage& src) :
                PlayFabBaseModel(),
                Key(src.Key),
                Lineage(src.Lineage)
            {}

            ~EntityWithLineage() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Key"], Key);
                FromJsonUtilO(input["Lineage"], Lineage);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Key; ToJsonUtilO(Key, each_Key); output["Key"] = each_Key;
                Json::Value each_Lineage; ToJsonUtilO(Lineage, each_Lineage); output["Lineage"] = each_Lineage;
                return output;
            }
        };

        struct ApplyToGroupResponse : public PlayFabResultCommon
        {
            Boxed<EntityWithLineage> Entity;
            time_t Expires;
            Boxed<EntityKey> Group;

            ApplyToGroupResponse() :
                PlayFabResultCommon(),
                Entity(),
                Expires(),
                Group()
            {}

            ApplyToGroupResponse(const ApplyToGroupResponse& src) :
                PlayFabResultCommon(),
                Entity(src.Entity),
                Expires(src.Expires),
                Group(src.Group)
            {}

            ~ApplyToGroupResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilT(input["Expires"], Expires);
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Expires; ToJsonUtilT(Expires, each_Expires); output["Expires"] = each_Expires;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct BlockEntityRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            EntityKey Group;

            BlockEntityRequest() :
                PlayFabRequestCommon(),
                Entity(),
                Group()
            {}

            BlockEntityRequest(const BlockEntityRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                Group(src.Group)
            {}

            ~BlockEntityRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct ChangeMemberRoleRequest : public PlayFabRequestCommon
        {
            std::string DestinationRoleId;
            EntityKey Group;
            std::list<EntityKey> Members;
            std::string OriginRoleId;

            ChangeMemberRoleRequest() :
                PlayFabRequestCommon(),
                DestinationRoleId(),
                Group(),
                Members(),
                OriginRoleId()
            {}

            ChangeMemberRoleRequest(const ChangeMemberRoleRequest& src) :
                PlayFabRequestCommon(),
                DestinationRoleId(src.DestinationRoleId),
                Group(src.Group),
                Members(src.Members),
                OriginRoleId(src.OriginRoleId)
            {}

            ~ChangeMemberRoleRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["DestinationRoleId"], DestinationRoleId);
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilO(input["Members"], Members);
                FromJsonUtilS(input["OriginRoleId"], OriginRoleId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_DestinationRoleId; ToJsonUtilS(DestinationRoleId, each_DestinationRoleId); output["DestinationRoleId"] = each_DestinationRoleId;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_Members; ToJsonUtilO(Members, each_Members); output["Members"] = each_Members;
                Json::Value each_OriginRoleId; ToJsonUtilS(OriginRoleId, each_OriginRoleId); output["OriginRoleId"] = each_OriginRoleId;
                return output;
            }
        };

        struct CreateGroupRequest : public PlayFabRequestCommon
        {
            Boxed<EntityKey> Entity;
            std::string GroupName;

            CreateGroupRequest() :
                PlayFabRequestCommon(),
                Entity(),
                GroupName()
            {}

            CreateGroupRequest(const CreateGroupRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                GroupName(src.GroupName)
            {}

            ~CreateGroupRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilS(input["GroupName"], GroupName);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_GroupName; ToJsonUtilS(GroupName, each_GroupName); output["GroupName"] = each_GroupName;
                return output;
            }
        };

        struct CreateGroupResponse : public PlayFabResultCommon
        {
            std::string AdminRoleId;
            time_t Created;
            EntityKey Group;
            std::string GroupName;
            std::string MemberRoleId;
            Int32 ProfileVersion;
            std::map<std::string, std::string> Roles;

            CreateGroupResponse() :
                PlayFabResultCommon(),
                AdminRoleId(),
                Created(),
                Group(),
                GroupName(),
                MemberRoleId(),
                ProfileVersion(),
                Roles()
            {}

            CreateGroupResponse(const CreateGroupResponse& src) :
                PlayFabResultCommon(),
                AdminRoleId(src.AdminRoleId),
                Created(src.Created),
                Group(src.Group),
                GroupName(src.GroupName),
                MemberRoleId(src.MemberRoleId),
                ProfileVersion(src.ProfileVersion),
                Roles(src.Roles)
            {}

            ~CreateGroupResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["AdminRoleId"], AdminRoleId);
                FromJsonUtilT(input["Created"], Created);
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilS(input["GroupName"], GroupName);
                FromJsonUtilS(input["MemberRoleId"], MemberRoleId);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
                FromJsonUtilS(input["Roles"], Roles);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_AdminRoleId; ToJsonUtilS(AdminRoleId, each_AdminRoleId); output["AdminRoleId"] = each_AdminRoleId;
                Json::Value each_Created; ToJsonUtilT(Created, each_Created); output["Created"] = each_Created;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_GroupName; ToJsonUtilS(GroupName, each_GroupName); output["GroupName"] = each_GroupName;
                Json::Value each_MemberRoleId; ToJsonUtilS(MemberRoleId, each_MemberRoleId); output["MemberRoleId"] = each_MemberRoleId;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                Json::Value each_Roles; ToJsonUtilS(Roles, each_Roles); output["Roles"] = each_Roles;
                return output;
            }
        };

        struct CreateGroupRoleRequest : public PlayFabRequestCommon
        {
            EntityKey Group;
            std::string RoleId;
            std::string RoleName;

            CreateGroupRoleRequest() :
                PlayFabRequestCommon(),
                Group(),
                RoleId(),
                RoleName()
            {}

            CreateGroupRoleRequest(const CreateGroupRoleRequest& src) :
                PlayFabRequestCommon(),
                Group(src.Group),
                RoleId(src.RoleId),
                RoleName(src.RoleName)
            {}

            ~CreateGroupRoleRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilS(input["RoleId"], RoleId);
                FromJsonUtilS(input["RoleName"], RoleName);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                Json::Value each_RoleName; ToJsonUtilS(RoleName, each_RoleName); output["RoleName"] = each_RoleName;
                return output;
            }
        };

        struct CreateGroupRoleResponse : public PlayFabResultCommon
        {
            Int32 ProfileVersion;
            std::string RoleId;
            std::string RoleName;

            CreateGroupRoleResponse() :
                PlayFabResultCommon(),
                ProfileVersion(),
                RoleId(),
                RoleName()
            {}

            CreateGroupRoleResponse(const CreateGroupRoleResponse& src) :
                PlayFabResultCommon(),
                ProfileVersion(src.ProfileVersion),
                RoleId(src.RoleId),
                RoleName(src.RoleName)
            {}

            ~CreateGroupRoleResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
                FromJsonUtilS(input["RoleId"], RoleId);
                FromJsonUtilS(input["RoleName"], RoleName);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                Json::Value each_RoleName; ToJsonUtilS(RoleName, each_RoleName); output["RoleName"] = each_RoleName;
                return output;
            }
        };

        struct DeleteFilesRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            std::list<std::string> FileNames;
            Boxed<Int32> ProfileVersion;

            DeleteFilesRequest() :
                PlayFabRequestCommon(),
                Entity(),
                FileNames(),
                ProfileVersion()
            {}

            DeleteFilesRequest(const DeleteFilesRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                FileNames(src.FileNames),
                ProfileVersion(src.ProfileVersion)
            {}

            ~DeleteFilesRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilS(input["FileNames"], FileNames);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_FileNames; ToJsonUtilS(FileNames, each_FileNames); output["FileNames"] = each_FileNames;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                return output;
            }
        };

        struct DeleteFilesResponse : public PlayFabResultCommon
        {
            Boxed<EntityKey> Entity;
            Int32 ProfileVersion;

            DeleteFilesResponse() :
                PlayFabResultCommon(),
                Entity(),
                ProfileVersion()
            {}

            DeleteFilesResponse(const DeleteFilesResponse& src) :
                PlayFabResultCommon(),
                Entity(src.Entity),
                ProfileVersion(src.ProfileVersion)
            {}

            ~DeleteFilesResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                return output;
            }
        };

        struct DeleteGroupRequest : public PlayFabRequestCommon
        {
            EntityKey Group;

            DeleteGroupRequest() :
                PlayFabRequestCommon(),
                Group()
            {}

            DeleteGroupRequest(const DeleteGroupRequest& src) :
                PlayFabRequestCommon(),
                Group(src.Group)
            {}

            ~DeleteGroupRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct DeleteRoleRequest : public PlayFabRequestCommon
        {
            EntityKey Group;
            std::string RoleId;

            DeleteRoleRequest() :
                PlayFabRequestCommon(),
                Group(),
                RoleId()
            {}

            DeleteRoleRequest(const DeleteRoleRequest& src) :
                PlayFabRequestCommon(),
                Group(src.Group),
                RoleId(src.RoleId)
            {}

            ~DeleteRoleRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilS(input["RoleId"], RoleId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                return output;
            }
        };

        struct EmptyResult : public PlayFabResultCommon
        {

            EmptyResult() :
                PlayFabResultCommon()
            {}

            EmptyResult(const EmptyResult&) :
                PlayFabResultCommon()
            {}

            ~EmptyResult() { }

            void FromJson(Json::Value&) override
            {
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                return output;
            }
        };

        struct EntityDataObject : public PlayFabBaseModel
        {
            Json::Value DataObject;
            std::string EscapedDataObject;
            std::string ObjectName;

            EntityDataObject() :
                PlayFabBaseModel(),
                DataObject(),
                EscapedDataObject(),
                ObjectName()
            {}

            EntityDataObject(const EntityDataObject& src) :
                PlayFabBaseModel(),
                DataObject(src.DataObject),
                EscapedDataObject(src.EscapedDataObject),
                ObjectName(src.ObjectName)
            {}

            ~EntityDataObject() { }

            void FromJson(Json::Value& input) override
            {
                DataObject = input["DataObject"];
                FromJsonUtilS(input["EscapedDataObject"], EscapedDataObject);
                FromJsonUtilS(input["ObjectName"], ObjectName);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                output["DataObject"] = DataObject;
                Json::Value each_EscapedDataObject; ToJsonUtilS(EscapedDataObject, each_EscapedDataObject); output["EscapedDataObject"] = each_EscapedDataObject;
                Json::Value each_ObjectName; ToJsonUtilS(ObjectName, each_ObjectName); output["ObjectName"] = each_ObjectName;
                return output;
            }
        };

        struct EntityMemberRole : public PlayFabBaseModel
        {
            std::list<EntityWithLineage> Members;
            std::string RoleId;
            std::string RoleName;

            EntityMemberRole() :
                PlayFabBaseModel(),
                Members(),
                RoleId(),
                RoleName()
            {}

            EntityMemberRole(const EntityMemberRole& src) :
                PlayFabBaseModel(),
                Members(src.Members),
                RoleId(src.RoleId),
                RoleName(src.RoleName)
            {}

            ~EntityMemberRole() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Members"], Members);
                FromJsonUtilS(input["RoleId"], RoleId);
                FromJsonUtilS(input["RoleName"], RoleName);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Members; ToJsonUtilO(Members, each_Members); output["Members"] = each_Members;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                Json::Value each_RoleName; ToJsonUtilS(RoleName, each_RoleName); output["RoleName"] = each_RoleName;
                return output;
            }
        };

        struct EntityPermissionStatement : public PlayFabBaseModel
        {
            std::string Action;
            std::string Comment;
            Json::Value Condition;
            EffectType Effect;
            Json::Value Principal;
            std::string Resource;

            EntityPermissionStatement() :
                PlayFabBaseModel(),
                Action(),
                Comment(),
                Condition(),
                Effect(),
                Principal(),
                Resource()
            {}

            EntityPermissionStatement(const EntityPermissionStatement& src) :
                PlayFabBaseModel(),
                Action(src.Action),
                Comment(src.Comment),
                Condition(src.Condition),
                Effect(src.Effect),
                Principal(src.Principal),
                Resource(src.Resource)
            {}

            ~EntityPermissionStatement() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["Action"], Action);
                FromJsonUtilS(input["Comment"], Comment);
                Condition = input["Condition"];
                FromJsonEnum(input["Effect"], Effect);
                Principal = input["Principal"];
                FromJsonUtilS(input["Resource"], Resource);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Action; ToJsonUtilS(Action, each_Action); output["Action"] = each_Action;
                Json::Value each_Comment; ToJsonUtilS(Comment, each_Comment); output["Comment"] = each_Comment;
                output["Condition"] = Condition;
                Json::Value each_Effect; ToJsonEnum(Effect, each_Effect); output["Effect"] = each_Effect;
                output["Principal"] = Principal;
                Json::Value each_Resource; ToJsonUtilS(Resource, each_Resource); output["Resource"] = each_Resource;
                return output;
            }
        };

        struct EntityProfileFileMetadata : public PlayFabBaseModel
        {
            std::string Checksum;
            std::string FileName;
            time_t LastModified;
            Int32 Size;

            EntityProfileFileMetadata() :
                PlayFabBaseModel(),
                Checksum(),
                FileName(),
                LastModified(),
                Size()
            {}

            EntityProfileFileMetadata(const EntityProfileFileMetadata& src) :
                PlayFabBaseModel(),
                Checksum(src.Checksum),
                FileName(src.FileName),
                LastModified(src.LastModified),
                Size(src.Size)
            {}

            ~EntityProfileFileMetadata() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["Checksum"], Checksum);
                FromJsonUtilS(input["FileName"], FileName);
                FromJsonUtilT(input["LastModified"], LastModified);
                FromJsonUtilP(input["Size"], Size);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Checksum; ToJsonUtilS(Checksum, each_Checksum); output["Checksum"] = each_Checksum;
                Json::Value each_FileName; ToJsonUtilS(FileName, each_FileName); output["FileName"] = each_FileName;
                Json::Value each_LastModified; ToJsonUtilT(LastModified, each_LastModified); output["LastModified"] = each_LastModified;
                Json::Value each_Size; ToJsonUtilP(Size, each_Size); output["Size"] = each_Size;
                return output;
            }
        };

        struct EntityProfileBody : public PlayFabBaseModel
        {
            Boxed<EntityKey> Entity;
            std::string EntityChain;
            std::map<std::string, EntityProfileFileMetadata> Files;
            std::map<std::string, EntityDataObject> Objects;
            std::list<EntityPermissionStatement> Permissions;
            Int32 VersionNumber;

            EntityProfileBody() :
                PlayFabBaseModel(),
                Entity(),
                EntityChain(),
                Files(),
                Objects(),
                Permissions(),
                VersionNumber()
            {}

            EntityProfileBody(const EntityProfileBody& src) :
                PlayFabBaseModel(),
                Entity(src.Entity),
                EntityChain(src.EntityChain),
                Files(src.Files),
                Objects(src.Objects),
                Permissions(src.Permissions),
                VersionNumber(src.VersionNumber)
            {}

            ~EntityProfileBody() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilS(input["EntityChain"], EntityChain);
                FromJsonUtilO(input["Files"], Files);
                FromJsonUtilO(input["Objects"], Objects);
                FromJsonUtilO(input["Permissions"], Permissions);
                FromJsonUtilP(input["VersionNumber"], VersionNumber);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_EntityChain; ToJsonUtilS(EntityChain, each_EntityChain); output["EntityChain"] = each_EntityChain;
                Json::Value each_Files; ToJsonUtilO(Files, each_Files); output["Files"] = each_Files;
                Json::Value each_Objects; ToJsonUtilO(Objects, each_Objects); output["Objects"] = each_Objects;
                Json::Value each_Permissions; ToJsonUtilO(Permissions, each_Permissions); output["Permissions"] = each_Permissions;
                Json::Value each_VersionNumber; ToJsonUtilP(VersionNumber, each_VersionNumber); output["VersionNumber"] = each_VersionNumber;
                return output;
            }
        };

        struct FinalizeFileUploadsRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            std::list<std::string> FileNames;

            FinalizeFileUploadsRequest() :
                PlayFabRequestCommon(),
                Entity(),
                FileNames()
            {}

            FinalizeFileUploadsRequest(const FinalizeFileUploadsRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                FileNames(src.FileNames)
            {}

            ~FinalizeFileUploadsRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilS(input["FileNames"], FileNames);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_FileNames; ToJsonUtilS(FileNames, each_FileNames); output["FileNames"] = each_FileNames;
                return output;
            }
        };

        struct GetFileMetadata : public PlayFabBaseModel
        {
            std::string Checksum;
            std::string DownloadUrl;
            std::string FileName;
            time_t LastModified;
            Int32 Size;

            GetFileMetadata() :
                PlayFabBaseModel(),
                Checksum(),
                DownloadUrl(),
                FileName(),
                LastModified(),
                Size()
            {}

            GetFileMetadata(const GetFileMetadata& src) :
                PlayFabBaseModel(),
                Checksum(src.Checksum),
                DownloadUrl(src.DownloadUrl),
                FileName(src.FileName),
                LastModified(src.LastModified),
                Size(src.Size)
            {}

            ~GetFileMetadata() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["Checksum"], Checksum);
                FromJsonUtilS(input["DownloadUrl"], DownloadUrl);
                FromJsonUtilS(input["FileName"], FileName);
                FromJsonUtilT(input["LastModified"], LastModified);
                FromJsonUtilP(input["Size"], Size);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Checksum; ToJsonUtilS(Checksum, each_Checksum); output["Checksum"] = each_Checksum;
                Json::Value each_DownloadUrl; ToJsonUtilS(DownloadUrl, each_DownloadUrl); output["DownloadUrl"] = each_DownloadUrl;
                Json::Value each_FileName; ToJsonUtilS(FileName, each_FileName); output["FileName"] = each_FileName;
                Json::Value each_LastModified; ToJsonUtilT(LastModified, each_LastModified); output["LastModified"] = each_LastModified;
                Json::Value each_Size; ToJsonUtilP(Size, each_Size); output["Size"] = each_Size;
                return output;
            }
        };

        struct FinalizeFileUploadsResponse : public PlayFabResultCommon
        {
            Boxed<EntityKey> Entity;
            std::map<std::string, GetFileMetadata> Metadata;
            Int32 ProfileVersion;

            FinalizeFileUploadsResponse() :
                PlayFabResultCommon(),
                Entity(),
                Metadata(),
                ProfileVersion()
            {}

            FinalizeFileUploadsResponse(const FinalizeFileUploadsResponse& src) :
                PlayFabResultCommon(),
                Entity(src.Entity),
                Metadata(src.Metadata),
                ProfileVersion(src.ProfileVersion)
            {}

            ~FinalizeFileUploadsResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Metadata"], Metadata);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Metadata; ToJsonUtilO(Metadata, each_Metadata); output["Metadata"] = each_Metadata;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                return output;
            }
        };

        struct GetEntityProfileRequest : public PlayFabRequestCommon
        {
            Boxed<bool> DataAsObject;
            EntityKey Entity;

            GetEntityProfileRequest() :
                PlayFabRequestCommon(),
                DataAsObject(),
                Entity()
            {}

            GetEntityProfileRequest(const GetEntityProfileRequest& src) :
                PlayFabRequestCommon(),
                DataAsObject(src.DataAsObject),
                Entity(src.Entity)
            {}

            ~GetEntityProfileRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["DataAsObject"], DataAsObject);
                FromJsonUtilO(input["Entity"], Entity);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_DataAsObject; ToJsonUtilP(DataAsObject, each_DataAsObject); output["DataAsObject"] = each_DataAsObject;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                return output;
            }
        };

        struct GetEntityProfileResponse : public PlayFabResultCommon
        {
            Boxed<EntityProfileBody> Profile;

            GetEntityProfileResponse() :
                PlayFabResultCommon(),
                Profile()
            {}

            GetEntityProfileResponse(const GetEntityProfileResponse& src) :
                PlayFabResultCommon(),
                Profile(src.Profile)
            {}

            ~GetEntityProfileResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Profile"], Profile);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Profile; ToJsonUtilO(Profile, each_Profile); output["Profile"] = each_Profile;
                return output;
            }
        };

        struct GetEntityTokenRequest : public PlayFabRequestCommon
        {
            Boxed<EntityKey> Entity;

            GetEntityTokenRequest() :
                PlayFabRequestCommon(),
                Entity()
            {}

            GetEntityTokenRequest(const GetEntityTokenRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity)
            {}

            ~GetEntityTokenRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                return output;
            }
        };

        struct GetEntityTokenResponse : public PlayFabResultCommon
        {
            Boxed<EntityKey> Entity;
            std::string EntityToken;
            Boxed<time_t> TokenExpiration;

            GetEntityTokenResponse() :
                PlayFabResultCommon(),
                Entity(),
                EntityToken(),
                TokenExpiration()
            {}

            GetEntityTokenResponse(const GetEntityTokenResponse& src) :
                PlayFabResultCommon(),
                Entity(src.Entity),
                EntityToken(src.EntityToken),
                TokenExpiration(src.TokenExpiration)
            {}

            ~GetEntityTokenResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilS(input["EntityToken"], EntityToken);
                FromJsonUtilT(input["TokenExpiration"], TokenExpiration);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_EntityToken; ToJsonUtilS(EntityToken, each_EntityToken); output["EntityToken"] = each_EntityToken;
                Json::Value each_TokenExpiration; ToJsonUtilT(TokenExpiration, each_TokenExpiration); output["TokenExpiration"] = each_TokenExpiration;
                return output;
            }
        };

        struct GetFilesRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;

            GetFilesRequest() :
                PlayFabRequestCommon(),
                Entity()
            {}

            GetFilesRequest(const GetFilesRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity)
            {}

            ~GetFilesRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                return output;
            }
        };

        struct GetFilesResponse : public PlayFabResultCommon
        {
            Boxed<EntityKey> Entity;
            std::map<std::string, GetFileMetadata> Metadata;
            Int32 ProfileVersion;

            GetFilesResponse() :
                PlayFabResultCommon(),
                Entity(),
                Metadata(),
                ProfileVersion()
            {}

            GetFilesResponse(const GetFilesResponse& src) :
                PlayFabResultCommon(),
                Entity(src.Entity),
                Metadata(src.Metadata),
                ProfileVersion(src.ProfileVersion)
            {}

            ~GetFilesResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Metadata"], Metadata);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Metadata; ToJsonUtilO(Metadata, each_Metadata); output["Metadata"] = each_Metadata;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                return output;
            }
        };

        struct GetGlobalPolicyRequest : public PlayFabRequestCommon
        {

            GetGlobalPolicyRequest() :
                PlayFabRequestCommon()
            {}

            GetGlobalPolicyRequest(const GetGlobalPolicyRequest&) :
                PlayFabRequestCommon()
            {}

            ~GetGlobalPolicyRequest() { }

            void FromJson(Json::Value&) override
            {
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                return output;
            }
        };

        struct GetGlobalPolicyResponse : public PlayFabResultCommon
        {
            std::list<EntityPermissionStatement> Permissions;

            GetGlobalPolicyResponse() :
                PlayFabResultCommon(),
                Permissions()
            {}

            GetGlobalPolicyResponse(const GetGlobalPolicyResponse& src) :
                PlayFabResultCommon(),
                Permissions(src.Permissions)
            {}

            ~GetGlobalPolicyResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Permissions"], Permissions);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Permissions; ToJsonUtilO(Permissions, each_Permissions); output["Permissions"] = each_Permissions;
                return output;
            }
        };

        struct GetGroupRequest : public PlayFabRequestCommon
        {
            Boxed<EntityKey> Group;
            std::string GroupName;

            GetGroupRequest() :
                PlayFabRequestCommon(),
                Group(),
                GroupName()
            {}

            GetGroupRequest(const GetGroupRequest& src) :
                PlayFabRequestCommon(),
                Group(src.Group),
                GroupName(src.GroupName)
            {}

            ~GetGroupRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilS(input["GroupName"], GroupName);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_GroupName; ToJsonUtilS(GroupName, each_GroupName); output["GroupName"] = each_GroupName;
                return output;
            }
        };

        struct GetGroupResponse : public PlayFabResultCommon
        {
            std::string AdminRoleId;
            time_t Created;
            EntityKey Group;
            std::string GroupName;
            std::string MemberRoleId;
            Int32 ProfileVersion;
            std::map<std::string, std::string> Roles;

            GetGroupResponse() :
                PlayFabResultCommon(),
                AdminRoleId(),
                Created(),
                Group(),
                GroupName(),
                MemberRoleId(),
                ProfileVersion(),
                Roles()
            {}

            GetGroupResponse(const GetGroupResponse& src) :
                PlayFabResultCommon(),
                AdminRoleId(src.AdminRoleId),
                Created(src.Created),
                Group(src.Group),
                GroupName(src.GroupName),
                MemberRoleId(src.MemberRoleId),
                ProfileVersion(src.ProfileVersion),
                Roles(src.Roles)
            {}

            ~GetGroupResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["AdminRoleId"], AdminRoleId);
                FromJsonUtilT(input["Created"], Created);
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilS(input["GroupName"], GroupName);
                FromJsonUtilS(input["MemberRoleId"], MemberRoleId);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
                FromJsonUtilS(input["Roles"], Roles);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_AdminRoleId; ToJsonUtilS(AdminRoleId, each_AdminRoleId); output["AdminRoleId"] = each_AdminRoleId;
                Json::Value each_Created; ToJsonUtilT(Created, each_Created); output["Created"] = each_Created;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_GroupName; ToJsonUtilS(GroupName, each_GroupName); output["GroupName"] = each_GroupName;
                Json::Value each_MemberRoleId; ToJsonUtilS(MemberRoleId, each_MemberRoleId); output["MemberRoleId"] = each_MemberRoleId;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                Json::Value each_Roles; ToJsonUtilS(Roles, each_Roles); output["Roles"] = each_Roles;
                return output;
            }
        };

        struct GetObjectsRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            Boxed<bool> EscapeObject;

            GetObjectsRequest() :
                PlayFabRequestCommon(),
                Entity(),
                EscapeObject()
            {}

            GetObjectsRequest(const GetObjectsRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                EscapeObject(src.EscapeObject)
            {}

            ~GetObjectsRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilP(input["EscapeObject"], EscapeObject);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_EscapeObject; ToJsonUtilP(EscapeObject, each_EscapeObject); output["EscapeObject"] = each_EscapeObject;
                return output;
            }
        };

        struct ObjectResult : public PlayFabResultCommon
        {
            Json::Value DataObject;
            std::string EscapedDataObject;
            std::string ObjectName;

            ObjectResult() :
                PlayFabResultCommon(),
                DataObject(),
                EscapedDataObject(),
                ObjectName()
            {}

            ObjectResult(const ObjectResult& src) :
                PlayFabResultCommon(),
                DataObject(src.DataObject),
                EscapedDataObject(src.EscapedDataObject),
                ObjectName(src.ObjectName)
            {}

            ~ObjectResult() { }

            void FromJson(Json::Value& input) override
            {
                DataObject = input["DataObject"];
                FromJsonUtilS(input["EscapedDataObject"], EscapedDataObject);
                FromJsonUtilS(input["ObjectName"], ObjectName);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                output["DataObject"] = DataObject;
                Json::Value each_EscapedDataObject; ToJsonUtilS(EscapedDataObject, each_EscapedDataObject); output["EscapedDataObject"] = each_EscapedDataObject;
                Json::Value each_ObjectName; ToJsonUtilS(ObjectName, each_ObjectName); output["ObjectName"] = each_ObjectName;
                return output;
            }
        };

        struct GetObjectsResponse : public PlayFabResultCommon
        {
            Boxed<EntityKey> Entity;
            std::map<std::string, ObjectResult> Objects;
            Int32 ProfileVersion;

            GetObjectsResponse() :
                PlayFabResultCommon(),
                Entity(),
                Objects(),
                ProfileVersion()
            {}

            GetObjectsResponse(const GetObjectsResponse& src) :
                PlayFabResultCommon(),
                Entity(src.Entity),
                Objects(src.Objects),
                ProfileVersion(src.ProfileVersion)
            {}

            ~GetObjectsResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Objects"], Objects);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Objects; ToJsonUtilO(Objects, each_Objects); output["Objects"] = each_Objects;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                return output;
            }
        };

        struct GroupApplication : public PlayFabBaseModel
        {
            Boxed<EntityWithLineage> Entity;
            time_t Expires;
            Boxed<EntityKey> Group;

            GroupApplication() :
                PlayFabBaseModel(),
                Entity(),
                Expires(),
                Group()
            {}

            GroupApplication(const GroupApplication& src) :
                PlayFabBaseModel(),
                Entity(src.Entity),
                Expires(src.Expires),
                Group(src.Group)
            {}

            ~GroupApplication() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilT(input["Expires"], Expires);
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Expires; ToJsonUtilT(Expires, each_Expires); output["Expires"] = each_Expires;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct GroupBlock : public PlayFabBaseModel
        {
            Boxed<EntityWithLineage> Entity;
            EntityKey Group;

            GroupBlock() :
                PlayFabBaseModel(),
                Entity(),
                Group()
            {}

            GroupBlock(const GroupBlock& src) :
                PlayFabBaseModel(),
                Entity(src.Entity),
                Group(src.Group)
            {}

            ~GroupBlock() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct GroupInvitation : public PlayFabBaseModel
        {
            time_t Expires;
            Boxed<EntityKey> Group;
            Boxed<EntityWithLineage> InvitedByEntity;
            Boxed<EntityWithLineage> InvitedEntity;
            std::string RoleId;

            GroupInvitation() :
                PlayFabBaseModel(),
                Expires(),
                Group(),
                InvitedByEntity(),
                InvitedEntity(),
                RoleId()
            {}

            GroupInvitation(const GroupInvitation& src) :
                PlayFabBaseModel(),
                Expires(src.Expires),
                Group(src.Group),
                InvitedByEntity(src.InvitedByEntity),
                InvitedEntity(src.InvitedEntity),
                RoleId(src.RoleId)
            {}

            ~GroupInvitation() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilT(input["Expires"], Expires);
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilO(input["InvitedByEntity"], InvitedByEntity);
                FromJsonUtilO(input["InvitedEntity"], InvitedEntity);
                FromJsonUtilS(input["RoleId"], RoleId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Expires; ToJsonUtilT(Expires, each_Expires); output["Expires"] = each_Expires;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_InvitedByEntity; ToJsonUtilO(InvitedByEntity, each_InvitedByEntity); output["InvitedByEntity"] = each_InvitedByEntity;
                Json::Value each_InvitedEntity; ToJsonUtilO(InvitedEntity, each_InvitedEntity); output["InvitedEntity"] = each_InvitedEntity;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                return output;
            }
        };

        struct GroupRole : public PlayFabBaseModel
        {
            std::string RoleId;
            std::string RoleName;

            GroupRole() :
                PlayFabBaseModel(),
                RoleId(),
                RoleName()
            {}

            GroupRole(const GroupRole& src) :
                PlayFabBaseModel(),
                RoleId(src.RoleId),
                RoleName(src.RoleName)
            {}

            ~GroupRole() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["RoleId"], RoleId);
                FromJsonUtilS(input["RoleName"], RoleName);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                Json::Value each_RoleName; ToJsonUtilS(RoleName, each_RoleName); output["RoleName"] = each_RoleName;
                return output;
            }
        };

        struct GroupWithRoles : public PlayFabBaseModel
        {
            Boxed<EntityKey> Group;
            std::string GroupName;
            Int32 ProfileVersion;
            std::list<GroupRole> Roles;

            GroupWithRoles() :
                PlayFabBaseModel(),
                Group(),
                GroupName(),
                ProfileVersion(),
                Roles()
            {}

            GroupWithRoles(const GroupWithRoles& src) :
                PlayFabBaseModel(),
                Group(src.Group),
                GroupName(src.GroupName),
                ProfileVersion(src.ProfileVersion),
                Roles(src.Roles)
            {}

            ~GroupWithRoles() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilS(input["GroupName"], GroupName);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
                FromJsonUtilO(input["Roles"], Roles);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_GroupName; ToJsonUtilS(GroupName, each_GroupName); output["GroupName"] = each_GroupName;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                Json::Value each_Roles; ToJsonUtilO(Roles, each_Roles); output["Roles"] = each_Roles;
                return output;
            }
        };

        struct InitiateFileUploadMetadata : public PlayFabBaseModel
        {
            std::string FileName;
            std::string UploadUrl;

            InitiateFileUploadMetadata() :
                PlayFabBaseModel(),
                FileName(),
                UploadUrl()
            {}

            InitiateFileUploadMetadata(const InitiateFileUploadMetadata& src) :
                PlayFabBaseModel(),
                FileName(src.FileName),
                UploadUrl(src.UploadUrl)
            {}

            ~InitiateFileUploadMetadata() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["FileName"], FileName);
                FromJsonUtilS(input["UploadUrl"], UploadUrl);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_FileName; ToJsonUtilS(FileName, each_FileName); output["FileName"] = each_FileName;
                Json::Value each_UploadUrl; ToJsonUtilS(UploadUrl, each_UploadUrl); output["UploadUrl"] = each_UploadUrl;
                return output;
            }
        };

        struct InitiateFileUploadsRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            std::list<std::string> FileNames;
            Boxed<Int32> ProfileVersion;

            InitiateFileUploadsRequest() :
                PlayFabRequestCommon(),
                Entity(),
                FileNames(),
                ProfileVersion()
            {}

            InitiateFileUploadsRequest(const InitiateFileUploadsRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                FileNames(src.FileNames),
                ProfileVersion(src.ProfileVersion)
            {}

            ~InitiateFileUploadsRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilS(input["FileNames"], FileNames);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_FileNames; ToJsonUtilS(FileNames, each_FileNames); output["FileNames"] = each_FileNames;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                return output;
            }
        };

        struct InitiateFileUploadsResponse : public PlayFabResultCommon
        {
            Boxed<EntityKey> Entity;
            Int32 ProfileVersion;
            std::list<InitiateFileUploadMetadata> UploadDetails;

            InitiateFileUploadsResponse() :
                PlayFabResultCommon(),
                Entity(),
                ProfileVersion(),
                UploadDetails()
            {}

            InitiateFileUploadsResponse(const InitiateFileUploadsResponse& src) :
                PlayFabResultCommon(),
                Entity(src.Entity),
                ProfileVersion(src.ProfileVersion),
                UploadDetails(src.UploadDetails)
            {}

            ~InitiateFileUploadsResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
                FromJsonUtilO(input["UploadDetails"], UploadDetails);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                Json::Value each_UploadDetails; ToJsonUtilO(UploadDetails, each_UploadDetails); output["UploadDetails"] = each_UploadDetails;
                return output;
            }
        };

        struct InviteToGroupRequest : public PlayFabRequestCommon
        {
            Boxed<bool> AutoAcceptOutstandingApplication;
            EntityKey Entity;
            EntityKey Group;
            std::string RoleId;

            InviteToGroupRequest() :
                PlayFabRequestCommon(),
                AutoAcceptOutstandingApplication(),
                Entity(),
                Group(),
                RoleId()
            {}

            InviteToGroupRequest(const InviteToGroupRequest& src) :
                PlayFabRequestCommon(),
                AutoAcceptOutstandingApplication(src.AutoAcceptOutstandingApplication),
                Entity(src.Entity),
                Group(src.Group),
                RoleId(src.RoleId)
            {}

            ~InviteToGroupRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["AutoAcceptOutstandingApplication"], AutoAcceptOutstandingApplication);
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilS(input["RoleId"], RoleId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_AutoAcceptOutstandingApplication; ToJsonUtilP(AutoAcceptOutstandingApplication, each_AutoAcceptOutstandingApplication); output["AutoAcceptOutstandingApplication"] = each_AutoAcceptOutstandingApplication;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                return output;
            }
        };

        struct InviteToGroupResponse : public PlayFabResultCommon
        {
            time_t Expires;
            Boxed<EntityKey> Group;
            Boxed<EntityWithLineage> InvitedByEntity;
            Boxed<EntityWithLineage> InvitedEntity;
            std::string RoleId;

            InviteToGroupResponse() :
                PlayFabResultCommon(),
                Expires(),
                Group(),
                InvitedByEntity(),
                InvitedEntity(),
                RoleId()
            {}

            InviteToGroupResponse(const InviteToGroupResponse& src) :
                PlayFabResultCommon(),
                Expires(src.Expires),
                Group(src.Group),
                InvitedByEntity(src.InvitedByEntity),
                InvitedEntity(src.InvitedEntity),
                RoleId(src.RoleId)
            {}

            ~InviteToGroupResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilT(input["Expires"], Expires);
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilO(input["InvitedByEntity"], InvitedByEntity);
                FromJsonUtilO(input["InvitedEntity"], InvitedEntity);
                FromJsonUtilS(input["RoleId"], RoleId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Expires; ToJsonUtilT(Expires, each_Expires); output["Expires"] = each_Expires;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_InvitedByEntity; ToJsonUtilO(InvitedByEntity, each_InvitedByEntity); output["InvitedByEntity"] = each_InvitedByEntity;
                Json::Value each_InvitedEntity; ToJsonUtilO(InvitedEntity, each_InvitedEntity); output["InvitedEntity"] = each_InvitedEntity;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                return output;
            }
        };

        struct IsMemberRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            EntityKey Group;
            std::string RoleId;

            IsMemberRequest() :
                PlayFabRequestCommon(),
                Entity(),
                Group(),
                RoleId()
            {}

            IsMemberRequest(const IsMemberRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                Group(src.Group),
                RoleId(src.RoleId)
            {}

            ~IsMemberRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilS(input["RoleId"], RoleId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                return output;
            }
        };

        struct IsMemberResponse : public PlayFabResultCommon
        {
            bool IsMember;

            IsMemberResponse() :
                PlayFabResultCommon(),
                IsMember()
            {}

            IsMemberResponse(const IsMemberResponse& src) :
                PlayFabResultCommon(),
                IsMember(src.IsMember)
            {}

            ~IsMemberResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["IsMember"], IsMember);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_IsMember; ToJsonUtilP(IsMember, each_IsMember); output["IsMember"] = each_IsMember;
                return output;
            }
        };

        struct ListGroupApplicationsRequest : public PlayFabRequestCommon
        {
            EntityKey Group;

            ListGroupApplicationsRequest() :
                PlayFabRequestCommon(),
                Group()
            {}

            ListGroupApplicationsRequest(const ListGroupApplicationsRequest& src) :
                PlayFabRequestCommon(),
                Group(src.Group)
            {}

            ~ListGroupApplicationsRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct ListGroupApplicationsResponse : public PlayFabResultCommon
        {
            std::list<GroupApplication> Applications;

            ListGroupApplicationsResponse() :
                PlayFabResultCommon(),
                Applications()
            {}

            ListGroupApplicationsResponse(const ListGroupApplicationsResponse& src) :
                PlayFabResultCommon(),
                Applications(src.Applications)
            {}

            ~ListGroupApplicationsResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Applications"], Applications);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Applications; ToJsonUtilO(Applications, each_Applications); output["Applications"] = each_Applications;
                return output;
            }
        };

        struct ListGroupBlocksRequest : public PlayFabRequestCommon
        {
            EntityKey Group;

            ListGroupBlocksRequest() :
                PlayFabRequestCommon(),
                Group()
            {}

            ListGroupBlocksRequest(const ListGroupBlocksRequest& src) :
                PlayFabRequestCommon(),
                Group(src.Group)
            {}

            ~ListGroupBlocksRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct ListGroupBlocksResponse : public PlayFabResultCommon
        {
            std::list<GroupBlock> BlockedEntities;

            ListGroupBlocksResponse() :
                PlayFabResultCommon(),
                BlockedEntities()
            {}

            ListGroupBlocksResponse(const ListGroupBlocksResponse& src) :
                PlayFabResultCommon(),
                BlockedEntities(src.BlockedEntities)
            {}

            ~ListGroupBlocksResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["BlockedEntities"], BlockedEntities);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_BlockedEntities; ToJsonUtilO(BlockedEntities, each_BlockedEntities); output["BlockedEntities"] = each_BlockedEntities;
                return output;
            }
        };

        struct ListGroupInvitationsRequest : public PlayFabRequestCommon
        {
            EntityKey Group;

            ListGroupInvitationsRequest() :
                PlayFabRequestCommon(),
                Group()
            {}

            ListGroupInvitationsRequest(const ListGroupInvitationsRequest& src) :
                PlayFabRequestCommon(),
                Group(src.Group)
            {}

            ~ListGroupInvitationsRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct ListGroupInvitationsResponse : public PlayFabResultCommon
        {
            std::list<GroupInvitation> Invitations;

            ListGroupInvitationsResponse() :
                PlayFabResultCommon(),
                Invitations()
            {}

            ListGroupInvitationsResponse(const ListGroupInvitationsResponse& src) :
                PlayFabResultCommon(),
                Invitations(src.Invitations)
            {}

            ~ListGroupInvitationsResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Invitations"], Invitations);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Invitations; ToJsonUtilO(Invitations, each_Invitations); output["Invitations"] = each_Invitations;
                return output;
            }
        };

        struct ListGroupMembersRequest : public PlayFabRequestCommon
        {
            EntityKey Group;

            ListGroupMembersRequest() :
                PlayFabRequestCommon(),
                Group()
            {}

            ListGroupMembersRequest(const ListGroupMembersRequest& src) :
                PlayFabRequestCommon(),
                Group(src.Group)
            {}

            ~ListGroupMembersRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct ListGroupMembersResponse : public PlayFabResultCommon
        {
            std::list<EntityMemberRole> Members;

            ListGroupMembersResponse() :
                PlayFabResultCommon(),
                Members()
            {}

            ListGroupMembersResponse(const ListGroupMembersResponse& src) :
                PlayFabResultCommon(),
                Members(src.Members)
            {}

            ~ListGroupMembersResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Members"], Members);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Members; ToJsonUtilO(Members, each_Members); output["Members"] = each_Members;
                return output;
            }
        };

        struct ListMembershipOpportunitiesRequest : public PlayFabRequestCommon
        {
            Boxed<EntityKey> Entity;

            ListMembershipOpportunitiesRequest() :
                PlayFabRequestCommon(),
                Entity()
            {}

            ListMembershipOpportunitiesRequest(const ListMembershipOpportunitiesRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity)
            {}

            ~ListMembershipOpportunitiesRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                return output;
            }
        };

        struct ListMembershipOpportunitiesResponse : public PlayFabResultCommon
        {
            std::list<GroupApplication> Applications;
            std::list<GroupInvitation> Invitations;

            ListMembershipOpportunitiesResponse() :
                PlayFabResultCommon(),
                Applications(),
                Invitations()
            {}

            ListMembershipOpportunitiesResponse(const ListMembershipOpportunitiesResponse& src) :
                PlayFabResultCommon(),
                Applications(src.Applications),
                Invitations(src.Invitations)
            {}

            ~ListMembershipOpportunitiesResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Applications"], Applications);
                FromJsonUtilO(input["Invitations"], Invitations);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Applications; ToJsonUtilO(Applications, each_Applications); output["Applications"] = each_Applications;
                Json::Value each_Invitations; ToJsonUtilO(Invitations, each_Invitations); output["Invitations"] = each_Invitations;
                return output;
            }
        };

        struct ListMembershipRequest : public PlayFabRequestCommon
        {
            Boxed<EntityKey> Entity;

            ListMembershipRequest() :
                PlayFabRequestCommon(),
                Entity()
            {}

            ListMembershipRequest(const ListMembershipRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity)
            {}

            ~ListMembershipRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                return output;
            }
        };

        struct ListMembershipResponse : public PlayFabResultCommon
        {
            std::list<GroupWithRoles> Groups;

            ListMembershipResponse() :
                PlayFabResultCommon(),
                Groups()
            {}

            ListMembershipResponse(const ListMembershipResponse& src) :
                PlayFabResultCommon(),
                Groups(src.Groups)
            {}

            ~ListMembershipResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Groups"], Groups);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Groups; ToJsonUtilO(Groups, each_Groups); output["Groups"] = each_Groups;
                return output;
            }
        };

        struct RemoveGroupApplicationRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            EntityKey Group;

            RemoveGroupApplicationRequest() :
                PlayFabRequestCommon(),
                Entity(),
                Group()
            {}

            RemoveGroupApplicationRequest(const RemoveGroupApplicationRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                Group(src.Group)
            {}

            ~RemoveGroupApplicationRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct RemoveGroupInvitationRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            EntityKey Group;

            RemoveGroupInvitationRequest() :
                PlayFabRequestCommon(),
                Entity(),
                Group()
            {}

            RemoveGroupInvitationRequest(const RemoveGroupInvitationRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                Group(src.Group)
            {}

            ~RemoveGroupInvitationRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct RemoveMembersRequest : public PlayFabRequestCommon
        {
            EntityKey Group;
            std::list<EntityKey> Members;
            std::string RoleId;

            RemoveMembersRequest() :
                PlayFabRequestCommon(),
                Group(),
                Members(),
                RoleId()
            {}

            RemoveMembersRequest(const RemoveMembersRequest& src) :
                PlayFabRequestCommon(),
                Group(src.Group),
                Members(src.Members),
                RoleId(src.RoleId)
            {}

            ~RemoveMembersRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilO(input["Members"], Members);
                FromJsonUtilS(input["RoleId"], RoleId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_Members; ToJsonUtilO(Members, each_Members); output["Members"] = each_Members;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                return output;
            }
        };

        struct SetEntityProfilePolicyRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            std::list<EntityPermissionStatement> Statements;

            SetEntityProfilePolicyRequest() :
                PlayFabRequestCommon(),
                Entity(),
                Statements()
            {}

            SetEntityProfilePolicyRequest(const SetEntityProfilePolicyRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                Statements(src.Statements)
            {}

            ~SetEntityProfilePolicyRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Statements"], Statements);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Statements; ToJsonUtilO(Statements, each_Statements); output["Statements"] = each_Statements;
                return output;
            }
        };

        struct SetEntityProfilePolicyResponse : public PlayFabResultCommon
        {
            std::list<EntityPermissionStatement> Permissions;

            SetEntityProfilePolicyResponse() :
                PlayFabResultCommon(),
                Permissions()
            {}

            SetEntityProfilePolicyResponse(const SetEntityProfilePolicyResponse& src) :
                PlayFabResultCommon(),
                Permissions(src.Permissions)
            {}

            ~SetEntityProfilePolicyResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Permissions"], Permissions);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Permissions; ToJsonUtilO(Permissions, each_Permissions); output["Permissions"] = each_Permissions;
                return output;
            }
        };

        struct SetGlobalPolicyRequest : public PlayFabRequestCommon
        {
            std::list<EntityPermissionStatement> Permissions;

            SetGlobalPolicyRequest() :
                PlayFabRequestCommon(),
                Permissions()
            {}

            SetGlobalPolicyRequest(const SetGlobalPolicyRequest& src) :
                PlayFabRequestCommon(),
                Permissions(src.Permissions)
            {}

            ~SetGlobalPolicyRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Permissions"], Permissions);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Permissions; ToJsonUtilO(Permissions, each_Permissions); output["Permissions"] = each_Permissions;
                return output;
            }
        };

        struct SetGlobalPolicyResponse : public PlayFabResultCommon
        {

            SetGlobalPolicyResponse() :
                PlayFabResultCommon()
            {}

            SetGlobalPolicyResponse(const SetGlobalPolicyResponse&) :
                PlayFabResultCommon()
            {}

            ~SetGlobalPolicyResponse() { }

            void FromJson(Json::Value&) override
            {
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                return output;
            }
        };

        struct SetObject : public PlayFabBaseModel
        {
            Json::Value DataObject;
            Boxed<bool> DeleteObject;
            std::string EscapedDataObject;
            std::string ObjectName;

            SetObject() :
                PlayFabBaseModel(),
                DataObject(),
                DeleteObject(),
                EscapedDataObject(),
                ObjectName()
            {}

            SetObject(const SetObject& src) :
                PlayFabBaseModel(),
                DataObject(src.DataObject),
                DeleteObject(src.DeleteObject),
                EscapedDataObject(src.EscapedDataObject),
                ObjectName(src.ObjectName)
            {}

            ~SetObject() { }

            void FromJson(Json::Value& input) override
            {
                DataObject = input["DataObject"];
                FromJsonUtilP(input["DeleteObject"], DeleteObject);
                FromJsonUtilS(input["EscapedDataObject"], EscapedDataObject);
                FromJsonUtilS(input["ObjectName"], ObjectName);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                output["DataObject"] = DataObject;
                Json::Value each_DeleteObject; ToJsonUtilP(DeleteObject, each_DeleteObject); output["DeleteObject"] = each_DeleteObject;
                Json::Value each_EscapedDataObject; ToJsonUtilS(EscapedDataObject, each_EscapedDataObject); output["EscapedDataObject"] = each_EscapedDataObject;
                Json::Value each_ObjectName; ToJsonUtilS(ObjectName, each_ObjectName); output["ObjectName"] = each_ObjectName;
                return output;
            }
        };

        struct SetObjectInfo : public PlayFabBaseModel
        {
            std::string ObjectName;
            std::string OperationReason;
            Boxed<OperationTypes> SetResult;

            SetObjectInfo() :
                PlayFabBaseModel(),
                ObjectName(),
                OperationReason(),
                SetResult()
            {}

            SetObjectInfo(const SetObjectInfo& src) :
                PlayFabBaseModel(),
                ObjectName(src.ObjectName),
                OperationReason(src.OperationReason),
                SetResult(src.SetResult)
            {}

            ~SetObjectInfo() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["ObjectName"], ObjectName);
                FromJsonUtilS(input["OperationReason"], OperationReason);
                FromJsonUtilE(input["SetResult"], SetResult);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_ObjectName; ToJsonUtilS(ObjectName, each_ObjectName); output["ObjectName"] = each_ObjectName;
                Json::Value each_OperationReason; ToJsonUtilS(OperationReason, each_OperationReason); output["OperationReason"] = each_OperationReason;
                Json::Value each_SetResult; ToJsonUtilE(SetResult, each_SetResult); output["SetResult"] = each_SetResult;
                return output;
            }
        };

        struct SetObjectsRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            Boxed<Int32> ExpectedProfileVersion;
            std::list<SetObject> Objects;

            SetObjectsRequest() :
                PlayFabRequestCommon(),
                Entity(),
                ExpectedProfileVersion(),
                Objects()
            {}

            SetObjectsRequest(const SetObjectsRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                ExpectedProfileVersion(src.ExpectedProfileVersion),
                Objects(src.Objects)
            {}

            ~SetObjectsRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilP(input["ExpectedProfileVersion"], ExpectedProfileVersion);
                FromJsonUtilO(input["Objects"], Objects);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_ExpectedProfileVersion; ToJsonUtilP(ExpectedProfileVersion, each_ExpectedProfileVersion); output["ExpectedProfileVersion"] = each_ExpectedProfileVersion;
                Json::Value each_Objects; ToJsonUtilO(Objects, each_Objects); output["Objects"] = each_Objects;
                return output;
            }
        };

        struct SetObjectsResponse : public PlayFabResultCommon
        {
            Int32 ProfileVersion;
            std::list<SetObjectInfo> SetResults;

            SetObjectsResponse() :
                PlayFabResultCommon(),
                ProfileVersion(),
                SetResults()
            {}

            SetObjectsResponse(const SetObjectsResponse& src) :
                PlayFabResultCommon(),
                ProfileVersion(src.ProfileVersion),
                SetResults(src.SetResults)
            {}

            ~SetObjectsResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
                FromJsonUtilO(input["SetResults"], SetResults);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                Json::Value each_SetResults; ToJsonUtilO(SetResults, each_SetResults); output["SetResults"] = each_SetResults;
                return output;
            }
        };

        struct UnblockEntityRequest : public PlayFabRequestCommon
        {
            EntityKey Entity;
            EntityKey Group;

            UnblockEntityRequest() :
                PlayFabRequestCommon(),
                Entity(),
                Group()
            {}

            UnblockEntityRequest(const UnblockEntityRequest& src) :
                PlayFabRequestCommon(),
                Entity(src.Entity),
                Group(src.Group)
            {}

            ~UnblockEntityRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilO(input["Entity"], Entity);
                FromJsonUtilO(input["Group"], Group);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_Entity; ToJsonUtilO(Entity, each_Entity); output["Entity"] = each_Entity;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                return output;
            }
        };

        struct UpdateGroupRequest : public PlayFabRequestCommon
        {
            std::string AdminRoleId;
            Boxed<Int32> ExpectedProfileVersion;
            EntityKey Group;
            std::string GroupName;
            std::string MemberRoleId;

            UpdateGroupRequest() :
                PlayFabRequestCommon(),
                AdminRoleId(),
                ExpectedProfileVersion(),
                Group(),
                GroupName(),
                MemberRoleId()
            {}

            UpdateGroupRequest(const UpdateGroupRequest& src) :
                PlayFabRequestCommon(),
                AdminRoleId(src.AdminRoleId),
                ExpectedProfileVersion(src.ExpectedProfileVersion),
                Group(src.Group),
                GroupName(src.GroupName),
                MemberRoleId(src.MemberRoleId)
            {}

            ~UpdateGroupRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["AdminRoleId"], AdminRoleId);
                FromJsonUtilP(input["ExpectedProfileVersion"], ExpectedProfileVersion);
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilS(input["GroupName"], GroupName);
                FromJsonUtilS(input["MemberRoleId"], MemberRoleId);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_AdminRoleId; ToJsonUtilS(AdminRoleId, each_AdminRoleId); output["AdminRoleId"] = each_AdminRoleId;
                Json::Value each_ExpectedProfileVersion; ToJsonUtilP(ExpectedProfileVersion, each_ExpectedProfileVersion); output["ExpectedProfileVersion"] = each_ExpectedProfileVersion;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_GroupName; ToJsonUtilS(GroupName, each_GroupName); output["GroupName"] = each_GroupName;
                Json::Value each_MemberRoleId; ToJsonUtilS(MemberRoleId, each_MemberRoleId); output["MemberRoleId"] = each_MemberRoleId;
                return output;
            }
        };

        struct UpdateGroupResponse : public PlayFabResultCommon
        {
            std::string OperationReason;
            Int32 ProfileVersion;
            Boxed<OperationTypes> SetResult;

            UpdateGroupResponse() :
                PlayFabResultCommon(),
                OperationReason(),
                ProfileVersion(),
                SetResult()
            {}

            UpdateGroupResponse(const UpdateGroupResponse& src) :
                PlayFabResultCommon(),
                OperationReason(src.OperationReason),
                ProfileVersion(src.ProfileVersion),
                SetResult(src.SetResult)
            {}

            ~UpdateGroupResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["OperationReason"], OperationReason);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
                FromJsonUtilE(input["SetResult"], SetResult);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_OperationReason; ToJsonUtilS(OperationReason, each_OperationReason); output["OperationReason"] = each_OperationReason;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                Json::Value each_SetResult; ToJsonUtilE(SetResult, each_SetResult); output["SetResult"] = each_SetResult;
                return output;
            }
        };

        struct UpdateGroupRoleRequest : public PlayFabRequestCommon
        {
            Boxed<Int32> ExpectedProfileVersion;
            EntityKey Group;
            std::string RoleId;
            std::string RoleName;

            UpdateGroupRoleRequest() :
                PlayFabRequestCommon(),
                ExpectedProfileVersion(),
                Group(),
                RoleId(),
                RoleName()
            {}

            UpdateGroupRoleRequest(const UpdateGroupRoleRequest& src) :
                PlayFabRequestCommon(),
                ExpectedProfileVersion(src.ExpectedProfileVersion),
                Group(src.Group),
                RoleId(src.RoleId),
                RoleName(src.RoleName)
            {}

            ~UpdateGroupRoleRequest() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilP(input["ExpectedProfileVersion"], ExpectedProfileVersion);
                FromJsonUtilO(input["Group"], Group);
                FromJsonUtilS(input["RoleId"], RoleId);
                FromJsonUtilS(input["RoleName"], RoleName);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_ExpectedProfileVersion; ToJsonUtilP(ExpectedProfileVersion, each_ExpectedProfileVersion); output["ExpectedProfileVersion"] = each_ExpectedProfileVersion;
                Json::Value each_Group; ToJsonUtilO(Group, each_Group); output["Group"] = each_Group;
                Json::Value each_RoleId; ToJsonUtilS(RoleId, each_RoleId); output["RoleId"] = each_RoleId;
                Json::Value each_RoleName; ToJsonUtilS(RoleName, each_RoleName); output["RoleName"] = each_RoleName;
                return output;
            }
        };

        struct UpdateGroupRoleResponse : public PlayFabResultCommon
        {
            std::string OperationReason;
            Int32 ProfileVersion;
            Boxed<OperationTypes> SetResult;

            UpdateGroupRoleResponse() :
                PlayFabResultCommon(),
                OperationReason(),
                ProfileVersion(),
                SetResult()
            {}

            UpdateGroupRoleResponse(const UpdateGroupRoleResponse& src) :
                PlayFabResultCommon(),
                OperationReason(src.OperationReason),
                ProfileVersion(src.ProfileVersion),
                SetResult(src.SetResult)
            {}

            ~UpdateGroupRoleResponse() { }

            void FromJson(Json::Value& input) override
            {
                FromJsonUtilS(input["OperationReason"], OperationReason);
                FromJsonUtilP(input["ProfileVersion"], ProfileVersion);
                FromJsonUtilE(input["SetResult"], SetResult);
            }

            Json::Value ToJson() const override
            {
                Json::Value output;
                Json::Value each_OperationReason; ToJsonUtilS(OperationReason, each_OperationReason); output["OperationReason"] = each_OperationReason;
                Json::Value each_ProfileVersion; ToJsonUtilP(ProfileVersion, each_ProfileVersion); output["ProfileVersion"] = each_ProfileVersion;
                Json::Value each_SetResult; ToJsonUtilE(SetResult, each_SetResult); output["SetResult"] = each_SetResult;
                return output;
            }
        };

    }
}

#endif
