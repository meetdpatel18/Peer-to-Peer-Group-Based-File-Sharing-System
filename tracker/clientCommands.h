void createUser(int client_sd, string userId, string password)
{
    if (doClientExist(userId))
    {
        responseToClient = "FALSE|User Id Already Exists, Try with another User Id";
        trackerLog = "User Id: " + userId + " Already Exists";
        Logger::ERROR(trackerLog.c_str());
    }
    else
    {
        insertNewClient(userId, password);
        responseToClient = "TRUE|User with (userId: " + userId + ") is Created Successfully";
        trackerLog = "User with (userId: " + userId + ") is Created Successfully";
        Logger::SUCCESS(trackerLog.c_str());
    }

    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void loginUser(int client_sd, string userId, string password, string clientsServerIP, int clientsServerPORT)
{
    if (doClientExist(userId))
    {
        if (isUserValid(userId, password))
        {
            if (updateClientsServerIPandPORT(userId, clientsServerIP, clientsServerPORT))
            {
                responseToClient = "TRUE|Login Successful for UserId: " + userId;
                trackerLog = "Login Successful for UserId: " + userId;
                Logger::SUCCESS(trackerLog.c_str());
            }
            else
            {
                responseToClient = "FALSE|You are already Logged In Some Other Terminal";
                trackerLog = "UserId: " + userId + " already logged In Some Other Terminal";
                Logger::WARN(trackerLog.c_str());
            }
        }
        else
        {
            responseToClient = "FALSE|Invalid User Id or Password";
            trackerLog = "Invalid User Id or Password for UserId: " + userId;
            Logger::WARN(trackerLog.c_str());
        }
    }
    else
    {
        responseToClient = "FALSE|User Account Does Not Exist";
        trackerLog = "UserId: " + userId + " Does Not Exist ";
        Logger::WARN(trackerLog.c_str());
    }

    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void createGroup(int client_sd, string groupId, string userId)
{
    if (doesGroupExist(groupId))
    {
        responseToClient = "FALSE|Group with this Name Already Exists ";
        trackerLog = "Group with GroupId: " + groupId + " Already Exists";
        Logger::WARN(trackerLog.c_str());
    }
    else
    {
        createNewGroup(groupId, userId);
        responseToClient = "TRUE|Group Created Successfully with GroupId: " + groupId;
        trackerLog = "Group Created Successfully with GroupId: " + groupId;
        Logger::SUCCESS(trackerLog.c_str());
    }

    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void joinGroup(int client_sd, string groupId, string userId)
{
    if (doesGroupExist(groupId))
    {
        if (doesUserExistsInGroup(groupId, userId))
        {
            responseToClient = "FALSE|You are already a part of the group: " + groupId;
            trackerLog = "UserId: " + userId + " is already a part of the group: " + groupId;
            Logger::WARN(trackerLog.c_str());
        }
        else
        {
            addUserToPendingRequests(groupId, userId);
            responseToClient = "TRUE|Join Group Request Sent Successfully";
            trackerLog = "Join Group Request to GroupId: " + groupId + " from UserId: " + userId + " Sent Successfully";
            Logger::SUCCESS(trackerLog.c_str());
        }
    }
    else
    {
        responseToClient = "FALSE|Group with Name: " + groupId + " Does Not Exist";
        trackerLog = "Group with Name: " + groupId + " Does Not Exist";
        Logger::WARN(trackerLog.c_str());
    }

    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void leave_group(int client_sd, string groupId, string userId)
{
    Logger::DEBUG("Leave Group tracker start");

    if (doesGroupExist(groupId))
    {
        Logger::DEBUG("group exist");

        if (doesUserExistsInGroup(groupId, userId))
        {
            Logger::DEBUG("user exist");

            if (isUserGroupOwner(groupId, userId))
            {
                Logger::DEBUG("owner");

                removeGroupOwner(groupId, userId);
            }
            else
            {
                Logger::DEBUG("not owner");

                removeGroupMember(groupId, userId);
            }
            responseToClient = "TRUE|You are no longer part of group: " + groupId;
            trackerLog = "UserId: " + userId + " left the group: " + groupId;
            Logger::SUCCESS(trackerLog.c_str());
        }
        else
        {
            responseToClient = "FALSE|You are already a part of the group: " + groupId;
            trackerLog = "UserId: " + userId + " is already a part of the group: " + groupId;
            Logger::WARN(trackerLog.c_str());
        }
    }
    else
    {
        responseToClient = "FALSE|Group with Name: " + groupId + " Does Not Exist";
        trackerLog = "Group with Name: " + groupId + " Does Not Exist";
        Logger::WARN(trackerLog.c_str());
    }

    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void listPendingJoinRequests(int client_sd, string groupId, string userId)
{
    if (doesGroupExist(groupId))
    {
        if (isUserGroupOwner(groupId, userId))
        {
            vector<string> requestList = getListOfPendingRequests(groupId);
            if (requestList.size() == 0)
            {
                responseToClient = "TRUE|No Pending Requests for Group: " + groupId;
                trackerLog = "No Pending Requests for Group: " + groupId;
            }
            else
            {
                responseToClient = "TRUE|" + vectorToString(requestList);
                trackerLog = "Pending Requests List for Group: " + groupId + " Sent Successfully";
            }
            Logger::SUCCESS(trackerLog.c_str());
        }
        else
        {
            responseToClient = "FALSE|Cannot List Requests as you are not the Group Owner";
            trackerLog = "UserId: " + userId + " is not Group Owner of Group: " + groupId + " hence cant sent request list";
            Logger::WARN(trackerLog.c_str());
        }
    }
    else
    {
        responseToClient = "FALSE|Group with this Name" + groupId + " Does Not Exist";
        trackerLog = "Group with this Name" + groupId + " Does Not Exist";
        Logger::WARN(trackerLog.c_str());
    }

    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void acceptGroupJoinRequests(int client_sd, string groupId, string newUserId, string loggedInUserId)
{
    if (doesGroupExist(groupId))
    {
        if (isUserGroupOwner(groupId, loggedInUserId))
        {
            vector<string> pendingRequestList = getListOfPendingRequests(groupId);
            bool userExistsInPendingList = false;
            for (auto it : pendingRequestList)
            {
                if (it == newUserId)
                {
                    userExistsInPendingList = true;
                    addUserToGroup(groupId, newUserId);
                    responseToClient = "TRUE|User " + newUserId + " is Successfully Added to the Group " + groupId;
                    trackerLog = "UserId: " + newUserId + " is Successfully Added to the Group " + groupId;
                    Logger::SUCCESS(trackerLog.c_str());
                    break;
                }
            }
            if (userExistsInPendingList == false)
            {
                responseToClient = "FALSE|Such User Id Does not Exist in Pending List";
                trackerLog = "UserId: " + newUserId + " Does not Exist in Pending List of Group: " + groupId;
                Logger::WARN(trackerLog.c_str());
            }
        }
        else
        {
            responseToClient = "FALSE|You need to be the Owner of the Group to Accept Requests";
            trackerLog = "User needs to be the Owner of the Group " + groupId + " to Accept Requests";
            Logger::WARN(trackerLog.c_str());
        }
    }
    else
    {
        responseToClient = "FALSE|Group with this Name Does Not Exist";
        trackerLog = "Group with Name: " + groupId + " Does Not Exist";
        Logger::WARN(trackerLog.c_str());
    }

    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void listGroups(int client_sd)
{
    vector<string> groups = getGroupsList();
    if (groups.size() > 0)
    {
        responseToClient = "TRUE|" + vectorToString(groups);
        trackerLog = "Group List Sent Successfully";
        Logger::SUCCESS(trackerLog.c_str());
    }
    else
    {
        responseToClient = "TRUE|";
        trackerLog = "No Groups Exists";
        Logger::SUCCESS(trackerLog.c_str());
    }

    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void listFiles(int client_sd, string groupId, string userId)
{
    if (doesGroupExist(groupId))
    {
        if (doesUserExistsInGroup(groupId, userId))
        {
            vector<string> filesInGroup = getFilesInGroup(groupId);
            if (filesInGroup.size() > 0)
            {
                responseToClient = "TRUE|" + vectorToString(filesInGroup);
                trackerLog = "List of Files for Group: " + groupId + " is sent successfully";
                Logger::SUCCESS(trackerLog.c_str());
            }
            else
            {
                responseToClient = "TRUE|";
                trackerLog = "No Files Exist in Group " + groupId;
                Logger::SUCCESS(trackerLog.c_str());
            }
        }
        else
        {
            responseToClient = "FALSE|You cannot view list of files as you are not member of the group: " + groupId;
            trackerLog = "User: " + userId + " cannot view list of files as he is not the member of group: " + groupId;
            Logger::ERROR(trackerLog.c_str());
        }
    }
    else
    {
        responseToClient = "FALSE|Group with this Name Does Not Exist";
        trackerLog = "Group with Name: " + groupId + " Does Not Exist";
        Logger::WARN(trackerLog.c_str());
    }
    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void uploadFile(int client_sd, string fileName, string groupId, LL fileSize, string SHA1, string loggedInUserId)
{
    Logger::DEBUG("Inside uploadFile function tracker");

    if (doesGroupExist(groupId))
    {
        Logger::DEBUG("after doesGroupExist");

        if (doesUserExistsInGroup(groupId, loggedInUserId))
        {
            Logger::DEBUG("after doesUserExistsInGroup");

            //////////TO BE DONE//////////////
            // if file exists of same name check for SHA1 if SHA1 matches with existing file then add user to seeder list else return false
            if (fileAlreadyExistsInGroup(groupId, fileName))
            {
                if (SHA1MatchesWithExistingFile(groupId, fileName, SHA1))
                {
                    // add user to seeder list
                    addUserToSeedersList(groupId, fileName, loggedInUserId);
                    responseToClient = "TRUE|File Uploaded Successfully";
                    trackerLog = "File: " + fileName + " uploaded successfully to group: " + groupId + " and user: " + loggedInUserId + " added to seeders list";
                    Logger::SUCCESS(trackerLog.c_str());
                }
                else
                {
                    // return false that file with same name cannot be uploaded
                    responseToClient = "FALSE|This file cannot be uploaded because file with same name already Exist in the group";
                    trackerLog = "File already exist with same name and different hash hence file: " + fileName + " cannot be uploaded to group: " + groupId;
                    Logger::ERROR(trackerLog.c_str());
                }
            }
            else
            {
                addNewFile(groupId, fileName, fileSize, SHA1, loggedInUserId);
                responseToClient = "TRUE|File Uploaded Successfully";
                trackerLog = "File: " + fileName + " uploaded successfully to group: " + groupId + " and user: " + loggedInUserId + " added to seeders list";
                Logger::SUCCESS(trackerLog.c_str());
            }
        }
        else
        {
            responseToClient = "FALSE|You cannot upload this file as you are not member of the group :" + groupId;
            trackerLog = "User: " + loggedInUserId + " cannot upload this file as he is not the member of group: " + groupId;
            Logger::ERROR(trackerLog.c_str());
        }
    }
    else
    {
        responseToClient = "FALSE|Group with Name: " + groupId + " Does Not Exist";
        trackerLog = "For Upload to Group with Name: " + groupId + " Does Not Exist";
        Logger::ERROR(trackerLog.c_str());
    }
    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void downloadFile(int client_sd, string groupId, string fileName, string loggedInUserId)
{
    if (doesGroupExist(groupId))
    {
        if (doesUserExistsInGroup(groupId, loggedInUserId))
        {
            // fileMetaData contains fileName|fileSize|SHA1|uid1|ip1|port1|uid2|ip2|port2|........
            string fileMetaData = getFileInfoIfExists(groupId, fileName);

            if (fileMetaData != "")
            {
                Logger::DEBUG(fileMetaData.c_str());

                responseToClient = "TRUE|" + fileMetaData;
                trackerLog = "File Metadata for download request of file " + fileName + " to user " + loggedInUserId + " sent successfully";
                Logger::SUCCESS(trackerLog.c_str());
            }
            else
            {
                responseToClient = "FALSE|File: " + fileName + " is not available in the group and hence can't be downloaded";
                trackerLog = "File: " + fileName + " is not available in the group and hence can't be downloaded by user " + loggedInUserId;
                Logger::WARN(trackerLog.c_str());
            }
        }
        else
        {
            responseToClient = "FALSE|You cannot download this file as you are not member of the group :" + groupId;
            trackerLog = "User: " + loggedInUserId + " cannot download this file as you are not member of the group: " + groupId;
            Logger::WARN(trackerLog.c_str());
        }
    }
    else
    {
        responseToClient = "FALSE|Group with Name: " + groupId + " Does Not Exist";
        trackerLog = "Group with Name: " + groupId + " Does Not Exist";
        Logger::WARN(trackerLog.c_str());
    }
    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void logoutUser(int client_sd, string loggedInUserId)
{
    removeIPandPort(loggedInUserId);
    responseToClient = "TRUE|You are logged out successfully";
    trackerLog = "User: " + loggedInUserId + " logged out successfully";
    Logger::SUCCESS(trackerLog.c_str());

    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void stopShare(int client_sd, string groupId, string fileName, string loggedInUserId)
{
    if (doesGroupExist(groupId))
    {
        if (doesUserExistsInGroup(groupId, loggedInUserId))
        {
            if (fileAlreadyExistsInGroup(groupId, fileName))
            {
                if (stopSharingFile(groupId, fileName, loggedInUserId))
                {
                    responseToClient = "TRUE|File " + fileName + " has been stopped sharing to group " + groupId;
                    trackerLog = "File " + fileName + " has been stopped sharing to group " + groupId + " by user: " + loggedInUserId;
                    Logger::SUCCESS(trackerLog.c_str());
                }
                else
                {
                    responseToClient = "FALSE|You are not owner of the file " + fileName + " to execute stop_share";
                    trackerLog = "User " + loggedInUserId + " is not the owner of the file to execute stop share";
                    Logger::ERROR(trackerLog.c_str());
                }
            }
            else
            {
                responseToClient = "FALSE|File with this name is not being shared by you";
                trackerLog = "File: " + fileName + " has not been shared to " + groupId + " by user: " + loggedInUserId;
                Logger::ERROR(trackerLog.c_str());
            }
        }
        else
        {
            responseToClient = "FALSE|You are not member of the group :" + groupId;
            trackerLog = "User: " + loggedInUserId + " is not the member of group: " + groupId;
            Logger::ERROR(trackerLog.c_str());
        }
    }
    else
    {
        responseToClient = "FALSE|Group with Name: " + groupId + " Does Not Exist";
        trackerLog = "For Stop Share of file " + fileName + " to Group with Name: " + groupId + " Does Not Exist";
        Logger::ERROR(trackerLog.c_str());
    }
    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void addLeacher(int client_sd, string groupId, string fileName, string userId)
{
    responseToClient = "TRUE|Leecher Added Successfully";
    addLeacherToFileInfoMap(groupId, fileName, userId);
    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}

void addSeeder(int client_sd, string groupId, string fileName, string userId)
{
    responseToClient = "TRUE|Seeder Added Successfully";
    addSeederToFileInfoMap(groupId, fileName, userId);
    send(client_sd, responseToClient.c_str(), responseToClient.size(), 0);
}
