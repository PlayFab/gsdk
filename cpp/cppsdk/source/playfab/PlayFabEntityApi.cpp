#include <gsdkCommonPch.h>

#ifdef ENABLE_PLAYFABENTITY_API

#include <playfab/PlayFabEntityApi.h>
#include <playfab/PlayFabHttp.h>
#include <playfab/PlayFabSettings.h>

namespace PlayFab
{
    using namespace EntityModels;

    size_t PlayFabEntityAPI::Update()
    {
        return PlayFabHttp::Get().Update();
    }

    void PlayFabEntityAPI::ForgetAllCredentials()
    {
        return PlayFabSettings::ForgetAllCredentials();
    }

    // PlayFabEntity APIs

    void PlayFabEntityAPI::AbortFileUploads(
        AbortFileUploadsRequest& request,
        ProcessApiCallback<AbortFileUploadsResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/File/AbortFileUploads", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnAbortFileUploadsResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<AbortFileUploadsResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnAbortFileUploadsResult(CallRequestContainer& request)
    {
        AbortFileUploadsResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<AbortFileUploadsResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::AcceptGroupApplication(
        AcceptGroupApplicationRequest& request,
        ProcessApiCallback<EmptyResult> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/AcceptGroupApplication", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnAcceptGroupApplicationResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<EmptyResult>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnAcceptGroupApplicationResult(CallRequestContainer& request)
    {
        EmptyResult outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<EmptyResult> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::AcceptGroupInvitation(
        AcceptGroupInvitationRequest& request,
        ProcessApiCallback<EmptyResult> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/AcceptGroupInvitation", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnAcceptGroupInvitationResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<EmptyResult>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnAcceptGroupInvitationResult(CallRequestContainer& request)
    {
        EmptyResult outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<EmptyResult> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::AddMembers(
        AddMembersRequest& request,
        ProcessApiCallback<EmptyResult> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/AddMembers", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnAddMembersResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<EmptyResult>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnAddMembersResult(CallRequestContainer& request)
    {
        EmptyResult outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<EmptyResult> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::ApplyToGroup(
        ApplyToGroupRequest& request,
        ProcessApiCallback<ApplyToGroupResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/ApplyToGroup", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnApplyToGroupResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<ApplyToGroupResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnApplyToGroupResult(CallRequestContainer& request)
    {
        ApplyToGroupResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<ApplyToGroupResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::BlockEntity(
        BlockEntityRequest& request,
        ProcessApiCallback<EmptyResult> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/BlockEntity", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnBlockEntityResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<EmptyResult>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnBlockEntityResult(CallRequestContainer& request)
    {
        EmptyResult outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<EmptyResult> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::ChangeMemberRole(
        ChangeMemberRoleRequest& request,
        ProcessApiCallback<EmptyResult> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/ChangeMemberRole", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnChangeMemberRoleResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<EmptyResult>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnChangeMemberRoleResult(CallRequestContainer& request)
    {
        EmptyResult outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<EmptyResult> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::CreateGroup(
        CreateGroupRequest& request,
        ProcessApiCallback<CreateGroupResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/CreateGroup", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnCreateGroupResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<CreateGroupResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnCreateGroupResult(CallRequestContainer& request)
    {
        CreateGroupResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<CreateGroupResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::CreateRole(
        CreateGroupRoleRequest& request,
        ProcessApiCallback<CreateGroupRoleResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/CreateRole", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnCreateRoleResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<CreateGroupRoleResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnCreateRoleResult(CallRequestContainer& request)
    {
        CreateGroupRoleResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<CreateGroupRoleResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::DeleteFiles(
        DeleteFilesRequest& request,
        ProcessApiCallback<DeleteFilesResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/File/DeleteFiles", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnDeleteFilesResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<DeleteFilesResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnDeleteFilesResult(CallRequestContainer& request)
    {
        DeleteFilesResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<DeleteFilesResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::DeleteGroup(
        DeleteGroupRequest& request,
        ProcessApiCallback<EmptyResult> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/DeleteGroup", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnDeleteGroupResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<EmptyResult>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnDeleteGroupResult(CallRequestContainer& request)
    {
        EmptyResult outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<EmptyResult> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::DeleteRole(
        DeleteRoleRequest& request,
        ProcessApiCallback<EmptyResult> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/DeleteRole", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnDeleteRoleResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<EmptyResult>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnDeleteRoleResult(CallRequestContainer& request)
    {
        EmptyResult outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<EmptyResult> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::FinalizeFileUploads(
        FinalizeFileUploadsRequest& request,
        ProcessApiCallback<FinalizeFileUploadsResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/File/FinalizeFileUploads", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnFinalizeFileUploadsResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<FinalizeFileUploadsResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnFinalizeFileUploadsResult(CallRequestContainer& request)
    {
        FinalizeFileUploadsResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<FinalizeFileUploadsResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::GetEntityToken(
        GetEntityTokenRequest& request,
        ProcessApiCallback<GetEntityTokenResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {
        std::string authKey, authValue;
        if (PlayFabSettings::entityToken.length() > 0) {
            authKey = "X-EntityToken"; authValue = PlayFabSettings::entityToken;
        } else if (PlayFabSettings::clientSessionTicket.length() > 0) {
            authKey = "X-Authorization"; authValue = PlayFabSettings::clientSessionTicket;
        } else if (PlayFabSettings::developerSecretKey.length() > 0) {
            authKey = "X-SecretKey"; authValue = PlayFabSettings::developerSecretKey;
        }

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Authentication/GetEntityToken", authKey, authValue, requestJson, OnGetEntityTokenResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<GetEntityTokenResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnGetEntityTokenResult(CallRequestContainer& request)
    {
        GetEntityTokenResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;
        if (outResult.EntityToken.length() > 0) PlayFabSettings::entityToken = outResult.EntityToken;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<GetEntityTokenResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::GetFiles(
        GetFilesRequest& request,
        ProcessApiCallback<GetFilesResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/File/GetFiles", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnGetFilesResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<GetFilesResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnGetFilesResult(CallRequestContainer& request)
    {
        GetFilesResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<GetFilesResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::GetGlobalPolicy(
        GetGlobalPolicyRequest& request,
        ProcessApiCallback<GetGlobalPolicyResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Profile/GetGlobalPolicy", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnGetGlobalPolicyResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<GetGlobalPolicyResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnGetGlobalPolicyResult(CallRequestContainer& request)
    {
        GetGlobalPolicyResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<GetGlobalPolicyResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::GetGroup(
        GetGroupRequest& request,
        ProcessApiCallback<GetGroupResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/GetGroup", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnGetGroupResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<GetGroupResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnGetGroupResult(CallRequestContainer& request)
    {
        GetGroupResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<GetGroupResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::GetObjects(
        GetObjectsRequest& request,
        ProcessApiCallback<GetObjectsResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Object/GetObjects", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnGetObjectsResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<GetObjectsResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnGetObjectsResult(CallRequestContainer& request)
    {
        GetObjectsResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<GetObjectsResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::GetProfile(
        GetEntityProfileRequest& request,
        ProcessApiCallback<GetEntityProfileResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Profile/GetProfile", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnGetProfileResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<GetEntityProfileResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnGetProfileResult(CallRequestContainer& request)
    {
        GetEntityProfileResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<GetEntityProfileResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::InitiateFileUploads(
        InitiateFileUploadsRequest& request,
        ProcessApiCallback<InitiateFileUploadsResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/File/InitiateFileUploads", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnInitiateFileUploadsResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<InitiateFileUploadsResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnInitiateFileUploadsResult(CallRequestContainer& request)
    {
        InitiateFileUploadsResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<InitiateFileUploadsResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::InviteToGroup(
        InviteToGroupRequest& request,
        ProcessApiCallback<InviteToGroupResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/InviteToGroup", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnInviteToGroupResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<InviteToGroupResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnInviteToGroupResult(CallRequestContainer& request)
    {
        InviteToGroupResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<InviteToGroupResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::IsMember(
        IsMemberRequest& request,
        ProcessApiCallback<IsMemberResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/IsMember", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnIsMemberResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<IsMemberResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnIsMemberResult(CallRequestContainer& request)
    {
        IsMemberResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<IsMemberResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::ListGroupApplications(
        ListGroupApplicationsRequest& request,
        ProcessApiCallback<ListGroupApplicationsResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/ListGroupApplications", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnListGroupApplicationsResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<ListGroupApplicationsResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnListGroupApplicationsResult(CallRequestContainer& request)
    {
        ListGroupApplicationsResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<ListGroupApplicationsResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::ListGroupBlocks(
        ListGroupBlocksRequest& request,
        ProcessApiCallback<ListGroupBlocksResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/ListGroupBlocks", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnListGroupBlocksResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<ListGroupBlocksResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnListGroupBlocksResult(CallRequestContainer& request)
    {
        ListGroupBlocksResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<ListGroupBlocksResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::ListGroupInvitations(
        ListGroupInvitationsRequest& request,
        ProcessApiCallback<ListGroupInvitationsResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/ListGroupInvitations", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnListGroupInvitationsResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<ListGroupInvitationsResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnListGroupInvitationsResult(CallRequestContainer& request)
    {
        ListGroupInvitationsResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<ListGroupInvitationsResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::ListGroupMembers(
        ListGroupMembersRequest& request,
        ProcessApiCallback<ListGroupMembersResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/ListGroupMembers", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnListGroupMembersResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<ListGroupMembersResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnListGroupMembersResult(CallRequestContainer& request)
    {
        ListGroupMembersResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<ListGroupMembersResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::ListMembership(
        ListMembershipRequest& request,
        ProcessApiCallback<ListMembershipResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/ListMembership", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnListMembershipResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<ListMembershipResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnListMembershipResult(CallRequestContainer& request)
    {
        ListMembershipResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<ListMembershipResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::ListMembershipOpportunities(
        ListMembershipOpportunitiesRequest& request,
        ProcessApiCallback<ListMembershipOpportunitiesResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/ListMembershipOpportunities", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnListMembershipOpportunitiesResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<ListMembershipOpportunitiesResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnListMembershipOpportunitiesResult(CallRequestContainer& request)
    {
        ListMembershipOpportunitiesResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<ListMembershipOpportunitiesResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::RemoveGroupApplication(
        RemoveGroupApplicationRequest& request,
        ProcessApiCallback<EmptyResult> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/RemoveGroupApplication", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnRemoveGroupApplicationResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<EmptyResult>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnRemoveGroupApplicationResult(CallRequestContainer& request)
    {
        EmptyResult outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<EmptyResult> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::RemoveGroupInvitation(
        RemoveGroupInvitationRequest& request,
        ProcessApiCallback<EmptyResult> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/RemoveGroupInvitation", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnRemoveGroupInvitationResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<EmptyResult>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnRemoveGroupInvitationResult(CallRequestContainer& request)
    {
        EmptyResult outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<EmptyResult> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::RemoveMembers(
        RemoveMembersRequest& request,
        ProcessApiCallback<EmptyResult> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/RemoveMembers", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnRemoveMembersResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<EmptyResult>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnRemoveMembersResult(CallRequestContainer& request)
    {
        EmptyResult outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<EmptyResult> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::SetGlobalPolicy(
        SetGlobalPolicyRequest& request,
        ProcessApiCallback<SetGlobalPolicyResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Profile/SetGlobalPolicy", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnSetGlobalPolicyResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<SetGlobalPolicyResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnSetGlobalPolicyResult(CallRequestContainer& request)
    {
        SetGlobalPolicyResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<SetGlobalPolicyResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::SetObjects(
        SetObjectsRequest& request,
        ProcessApiCallback<SetObjectsResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Object/SetObjects", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnSetObjectsResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<SetObjectsResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnSetObjectsResult(CallRequestContainer& request)
    {
        SetObjectsResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<SetObjectsResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::SetProfilePolicy(
        SetEntityProfilePolicyRequest& request,
        ProcessApiCallback<SetEntityProfilePolicyResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Profile/SetProfilePolicy", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnSetProfilePolicyResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<SetEntityProfilePolicyResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnSetProfilePolicyResult(CallRequestContainer& request)
    {
        SetEntityProfilePolicyResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<SetEntityProfilePolicyResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::UnblockEntity(
        UnblockEntityRequest& request,
        ProcessApiCallback<EmptyResult> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/UnblockEntity", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnUnblockEntityResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<EmptyResult>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnUnblockEntityResult(CallRequestContainer& request)
    {
        EmptyResult outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<EmptyResult> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::UpdateGroup(
        UpdateGroupRequest& request,
        ProcessApiCallback<UpdateGroupResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/UpdateGroup", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnUpdateGroupResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<UpdateGroupResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnUpdateGroupResult(CallRequestContainer& request)
    {
        UpdateGroupResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<UpdateGroupResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }

    void PlayFabEntityAPI::UpdateRole(
        UpdateGroupRoleRequest& request,
        ProcessApiCallback<UpdateGroupRoleResponse> callback,
        ErrorCallback errorCallback,
        void* customData
    )
    {

        IPlayFabHttp& http = IPlayFabHttp::Get();
        const auto requestJson = request.ToJson();
        http.AddRequest("/Group/UpdateRole", "X-EntityToken", PlayFabSettings::entityToken, requestJson, OnUpdateRoleResult, SharedVoidPointer((callback == nullptr) ? nullptr : new ProcessApiCallback<UpdateGroupRoleResponse>(callback)), errorCallback, customData);
    }

    void PlayFabEntityAPI::OnUpdateRoleResult(CallRequestContainer& request)
    {
        UpdateGroupRoleResponse outResult;
        outResult.FromJson(request.errorWrapper.Data);
        outResult.Request = request.errorWrapper.Request;

        const auto internalPtr = request.successCallback.get();
        if (internalPtr != nullptr)
        {
            const auto callback = (*static_cast<ProcessApiCallback<UpdateGroupRoleResponse> *>(internalPtr));
            callback(outResult, request.customData);
        }
    }
}

#endif
