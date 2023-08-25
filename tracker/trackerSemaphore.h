void initSemaphores()
{
    sem_init(&clientInfoMapReadLock, 0, 1);
    sem_init(&clientInfoMapWriteLock, 0, 1);

    sem_init(&groupInfoMapReadLock, 0, 1);
    sem_init(&groupInfoMapWriteLock, 0, 1);

    sem_init(&fileInfoMapReadLock, 0, 1);
    sem_init(&fileInfoMapWriteLock, 0, 1);
}

void clientInfoReadLock()
{
    sem_wait(&clientInfoMapReadLock);
    clientInfoMapReaderCount++;

    if (clientInfoMapReaderCount == 1)
        sem_wait(&clientInfoMapWriteLock);

    sem_post(&clientInfoMapReadLock);
}

void clientInfoReadUnlock()
{
    sem_wait(&clientInfoMapReadLock);
    clientInfoMapReaderCount--;

    if (clientInfoMapReaderCount == 0)
    {
        sem_post(&clientInfoMapWriteLock);
    }

    sem_post(&clientInfoMapReadLock);
}

void groupInfoReadLock()
{
    sem_wait(&groupInfoMapReadLock);
    groupInfoMapReaderCount++;

    if (groupInfoMapReaderCount == 1)
        sem_wait(&groupInfoMapWriteLock);

    sem_post(&groupInfoMapReadLock);
}

void groupInfoReadUnlock()
{
    sem_wait(&groupInfoMapReadLock);
    groupInfoMapReaderCount--;

    if (groupInfoMapReaderCount == 0)
    {
        sem_post(&groupInfoMapWriteLock);
    }

    sem_post(&groupInfoMapReadLock);
}

void fileInfoReadLock()
{
    sem_wait(&fileInfoMapReadLock);
    fileInfoMapReaderCount++;

    if (fileInfoMapReaderCount == 1)
        sem_wait(&fileInfoMapWriteLock);

    sem_post(&fileInfoMapReadLock);
}

void fileInfoReadUnlock()
{
    sem_wait(&fileInfoMapReadLock);
    fileInfoMapReaderCount--;

    if (fileInfoMapReaderCount == 0)
    {
        sem_post(&fileInfoMapWriteLock);
    }

    sem_post(&fileInfoMapReadLock);
}

bool doClientExist(string userId)
{
    bool exists = true;

    clientInfoReadLock();
    // CRITICAL SECTION STARTS
    if (clientInfoMap.find(userId) == clientInfoMap.end())
    {
        exists = false;
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS
    clientInfoReadUnlock();
    return exists;
}

void insertNewClient(string userId, string password)
{
    sem_wait(&clientInfoMapWriteLock);
    ClientInfo *client = new ClientInfo(userId, password);
    clientInfoMap[userId] = client;
    flushClientInfoMapTofile();
    sem_post(&clientInfoMapWriteLock);
}

bool isUserValid(string userId, string password)
{
    bool isValid = true;

    clientInfoReadLock();
    // CRITICAL SECTION STARTS
    if (clientInfoMap[userId]->getPassword() != password)
    {
        isValid = false;
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS
    clientInfoReadUnlock();
    return isValid;
}

bool updateClientsServerIPandPORT(string userId, string clientsServerIP, int clientsServerPORT)
{
    bool updateNeeded = false;
    sem_wait(&clientInfoMapWriteLock);

    ClientInfo *client = clientInfoMap[userId];
    if (client->getIP() == "$")
    {
        client->setIP(clientsServerIP);
        client->setPORT(clientsServerPORT);
        client->setisUserLoggedIn(true);
        clientInfoMap[userId] = client;
        updateNeeded = true;
        flushClientInfoMapTofile();
    }
    else
    {
        updateNeeded = false;
    }

    sem_post(&clientInfoMapWriteLock);
    return updateNeeded;
}

bool doesGroupExist(string groupId)
{
    bool exists = true;
    groupInfoReadLock();
    // CRITICAL SECTION STARTS
    if (groupInfoMap.find(groupId) == groupInfoMap.end())
    {
        exists = false;
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS
    groupInfoReadUnlock();
    return exists;
}

void createNewGroup(string groupId, string userId)
{
    sem_wait(&groupInfoMapWriteLock);
    GroupInfo *group = new GroupInfo(groupId, userId);
    group->addOwnerToGroup(userId);
    groupInfoMap[groupId] = group;
    flushGroupInfoMapTofile();
    sem_post(&groupInfoMapWriteLock);
}

bool doesUserExistsInGroup(string groupId, string userId)
{
    bool exists = false;

    groupInfoReadLock();
    // CRITICAL SECTION STARTS
    GroupInfo *group = groupInfoMap[groupId];
    if (group->doesUserExistInGroup(userId))
    {
        exists = true;
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS

    groupInfoReadUnlock();
    return exists;
}

void addUserToPendingRequests(string groupId, string userId)
{
    sem_wait(&groupInfoMapWriteLock);
    GroupInfo *group = groupInfoMap[groupId];
    group->addUserToPendingList(userId);
    groupInfoMap[groupId] = group;
    flushGroupInfoMapTofile();
    sem_post(&groupInfoMapWriteLock);
}

bool isUserGroupOwner(string groupId, string userId)
{
    bool isOwner = false;

    groupInfoReadLock();

    // CRITICAL SECTION STARTS
    GroupInfo *group = groupInfoMap[groupId];
    if (group->getOwnerId() == userId)
    {
        isOwner = true;
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS

    groupInfoReadUnlock();
    return isOwner;
}

void removeGroupMember(string groupId, string userId)
{
    sem_wait(&groupInfoMapWriteLock);
    GroupInfo *group = groupInfoMap[groupId];
    vector<string> members = group->getGroupMembers();
    vector<string> filesShared = group->getGroupFiles();
    vector<string> filesToBeRemoved;

    for (auto fileName : filesShared)
    {
        if (isSharedByUserAlone(groupId, fileName, userId))
        {
            filesToBeRemoved.push_back(fileName);
        }
    }
    removeFilesFromFileInfoMap(groupId, filesToBeRemoved);

    vector<string> filesRemaining;
    set_difference(filesShared.begin(), filesShared.end(), filesToBeRemoved.begin(), filesToBeRemoved.end(),
                   inserter(filesRemaining, filesRemaining.begin()));

    group->setFilesList(filesRemaining);

    for (int i = 0; i < members.size(); i++)
    {
        if (members[i] == userId)
        {
            members.erase(members.begin() + i);
            break;
        }
    }
    group->setMembersList(members);
    groupInfoMap[groupId] = group;
    flushGroupInfoMapTofile();
    sem_post(&groupInfoMapWriteLock);
}

void removeGroupOwner(string groupId, string ownerId)
{
    sem_wait(&groupInfoMapWriteLock);
    GroupInfo *group = groupInfoMap[groupId];
    vector<string> members = group->getGroupMembers();

    // if the group owner is the only member then remove all its shared files
    vector<string> filesShared = group->getGroupFiles();

    if (members.size() == 1)
    {
        removeFilesFromFileInfoMap(groupId, filesShared);
        groupInfoMap.erase(groupId);
    }
    else
    {
        vector<string> filesToBeRemoved;
        for (auto fileName : filesShared)
        {
            if (isSharedByUserAlone(groupId, fileName, ownerId))
            {
                filesToBeRemoved.push_back(fileName);
            }
        }
        removeFilesFromFileInfoMap(groupId, filesToBeRemoved);

        vector<string> filesRemaining;
        set_difference(filesShared.begin(), filesShared.end(), filesToBeRemoved.begin(), filesToBeRemoved.end(),
                       inserter(filesRemaining, filesRemaining.begin()));

        group->setFilesList(filesRemaining);

        string nextOwner = members[1];
        members.erase(members.begin());
        group->setOwnerId(nextOwner);
        group->setMembersList(members);
        groupInfoMap[groupId] = group;
    }
    flushGroupInfoMapTofile();
    sem_post(&groupInfoMapWriteLock);
}

bool isSharedByUserAlone(string groupId, string fileName, string userId)
{
    bool isUserTheOnlyFileOwner = false;
    string key = groupId + DELIMITERSTR + fileName;

    fileInfoReadLock();

    // CRITICAL SECTION STARTS

    vector<string>
        seedersList = fileInfoMap[key]->getSeedersList();
    if (seedersList.size() == 1 && seedersList[0] == userId)
    {
        isUserTheOnlyFileOwner = true;
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS
    fileInfoReadUnlock();
    return isUserTheOnlyFileOwner;
}

void removeFilesFromFileInfoMap(string groupId, vector<string> filesShared)
{
    sem_wait(&fileInfoMapWriteLock);

    // CRITICAL SECTION STARTS
    string key;
    for (auto fileName : filesShared)
    {
        key = groupId + DELIMITERSTR + fileName;
        fileInfoMap.erase(key);
    }

    // CRITICAL SECTION ENDS
    flushFileInfoMapTofile();
    sem_post(&fileInfoMapWriteLock);
}

void removeFileFromGroupInfoMap(string groupId, string fileName)
{
    sem_wait(&groupInfoMapWriteLock);

    GroupInfo *group = groupInfoMap[groupId];
    vector<string> files = group->getGroupFiles();
    int i;
    for (i = 0; i < files.size(); i++)
    {
        if (files[i] == fileName)
        {
            files.erase(files.begin() + i);
            break;
        }
    }
    group->setFilesList(files);
    groupInfoMap[groupId] = group;
    flushGroupInfoMapTofile();
    sem_post(&groupInfoMapWriteLock);
}

bool stopSharingFile(string groupId, string fileName, string userId)
{
    bool canStopShare = false;
    sem_wait(&fileInfoMapWriteLock);
    string key = groupId + DELIMITERSTR + fileName;

    FileInfo *file = fileInfoMap[key];
    vector<string> seedersList = file->getSeedersList();

    if (seedersList.size() == 1)
    {
        if (seedersList[0] == userId)
        {
            fileInfoMap.erase(key);
            flushFileInfoMapTofile();
            removeFileFromGroupInfoMap(groupId, fileName);
            canStopShare = true;
        }
    }
    else
    {
        for (int i = 0; i < seedersList.size(); i++)
        {
            if (seedersList[i] == userId)
            {
                seedersList.erase(seedersList.begin() + i);
                canStopShare = true;
                break;
            }
        }
        file->setSeedersList(seedersList);
        fileInfoMap[key] = file;
        flushFileInfoMapTofile();
    }
    sem_post(&fileInfoMapWriteLock);
    return canStopShare;
}

vector<string> getListOfPendingRequests(string groupId)
{
    vector<string> listRequests;

    groupInfoReadLock();

    // CRITICAL SECTION STARTS
    GroupInfo *group = groupInfoMap[groupId];
    listRequests = group->getPendingRequestsList();

    sleep(0.1);
    // CRITICAL SECTION ENDS

    groupInfoReadUnlock();
    return listRequests;
}

void addUserToGroup(string groupId, string newUserId)
{
    sem_wait(&groupInfoMapWriteLock);

    // CRITICAL SECTION STARTS
    GroupInfo *group = groupInfoMap[groupId];
    group->addUserToGroup(newUserId);
    groupInfoMap[groupId] = group;
    flushGroupInfoMapTofile();
    // CRITICAL SECTION ENDS

    sem_post(&groupInfoMapWriteLock);
}

vector<string> getGroupsList()
{
    vector<string> listRequests;

    groupInfoReadLock();

    // CRITICAL SECTION STARTS
    for (auto it : groupInfoMap)
    {
        listRequests.push_back(it.first);
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS

    groupInfoReadUnlock();
    return listRequests;
}

vector<string> getFilesInGroup(string groupId)
{
    vector<string> filesList;

    groupInfoReadLock();

    // CRITICAL SECTION STARTS
    filesList = groupInfoMap[groupId]->getGroupFiles();
    // CRITICAL SECTION ENDS

    groupInfoReadUnlock();
    return filesList;
}

void addUserToSeedersList(string groupId, string fileName, string loggedInUserId)
{
    string key = groupId + DELIMITERSTR + fileName;

    sem_wait(&fileInfoMapWriteLock);

    // CRITICAL SECTION STARTS

    FileInfo *file = fileInfoMap[key];
    file->addMemberToSeedersList(loggedInUserId);
    fileInfoMap[key] = file;
    flushFileInfoMapTofile();
    // CRITICAL SECTION ENDS

    sem_post(&fileInfoMapWriteLock);
}

bool fileAlreadyExistsInGroup(string groupId, string fileName)
{
    bool exists = false;
    string key = groupId + DELIMITERSTR + fileName;

    fileInfoReadLock();

    // CRITICAL SECTION STARTS
    if (fileInfoMap.find(key) != fileInfoMap.end())
    {
        exists = true;
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS

    fileInfoReadUnlock();
    return exists;
}

bool SHA1MatchesWithExistingFile(string groupId, string fileName, string SHA1)
{
    bool matches = false;
    string key = groupId + DELIMITERSTR + fileName;

    fileInfoReadLock();

    // CRITICAL SECTION STARTS
    FileInfo *file = fileInfoMap[key];
    if (file->getSHA1() == SHA1)
    {
        matches = true;
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS

    fileInfoReadUnlock();
    return matches;
}

void addNewFile(string groupId, string fileName, LL fileSize, string SHA1, string loggedInUserId)
{
    // ADDING FILE META DATA TO FILE INFO MAP
    string key = groupId + DELIMITERSTR + fileName;

    sem_wait(&fileInfoMapWriteLock);

    // CRITICAL SECTION STARTS
    FileInfo *file = new FileInfo(fileName, fileSize, SHA1, loggedInUserId);
    fileInfoMap[key] = file;
    flushFileInfoMapTofile();
    // CRITICAL SECTION ENDS

    sem_post(&fileInfoMapWriteLock);

    // ADDING FILE META DATA TO GROUP INFO MAP

    sem_wait(&groupInfoMapWriteLock);

    // CRITICAL SECTION STARTS
    GroupInfo *group = groupInfoMap[groupId];
    group->addNewFileToGroup(fileName);
    groupInfoMap[groupId] = group;
    flushGroupInfoMapTofile();
    // CRITICAL SECTION ENDS

    sem_post(&groupInfoMapWriteLock);
}

string getFileInfoIfExists(string groupId, string fileName)
{
    string key = groupId + DELIMITERSTR + fileName;
    FileInfo *fileInfo;
    bool fileExists = false;

    vector<string> fileMetaData;
    vector<string> userMetaData;

    fileInfoReadLock();

    // CRITICAL SECTION STARTS
    if (fileInfoMap.find(key) != fileInfoMap.end())
    {
        fileInfo = fileInfoMap[key];
        fileExists = true;

        fileMetaData.push_back(fileInfo->getFileName());

        fileMetaData.push_back(to_string(fileInfo->getFileSize()));
        fileMetaData.push_back(fileInfo->getSHA1());

        vector<string> peersFileList = fileInfo->getSeedersList();
        vector<string> leechersList = fileInfo->getLeechersList();
        peersFileList.insert(peersFileList.end(), leechersList.begin(), leechersList.end());

        userMetaData = getUserMetaData(peersFileList);

        fileMetaData.insert(fileMetaData.end(), userMetaData.begin(), userMetaData.end());
    }

    sleep(0.1);
    // CRITICAL SECTION ENDS

    fileInfoReadUnlock();

    if (!fileExists)
        return "";

    return vectorToString(fileMetaData, DELIMITERSTR);
}

vector<string> getUserMetaData(vector<string> peersList)
{
    ClientInfo *client;
    vector<string> userMetaData;

    clientInfoReadLock();

    // CRITICAL SECTION STARTS

    for (auto it : peersList)
    {
        if (clientInfoMap.find(it) == clientInfoMap.end())
            continue;
        client = clientInfoMap[it];
        if (client->getIP() == "$")
            continue;
        userMetaData.push_back(client->getClientId());
        userMetaData.push_back(client->getIP());
        userMetaData.push_back(to_string(client->getPORT()));
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS

    clientInfoReadUnlock();
    return userMetaData;
}

void removeIPandPort(string loggedInUserId)
{
    sem_wait(&clientInfoMapWriteLock);
    ClientInfo *client = clientInfoMap[loggedInUserId];
    client->setIP("$");
    client->setPORT(-1);
    client->setisUserLoggedIn(false);
    clientInfoMap[loggedInUserId] = client;
    flushClientInfoMapTofile();
    sem_post(&clientInfoMapWriteLock);
}

void addLeacherToFileInfoMap(string groupId, string fileName, string userId)
{
    string key = groupId + DELIMITERSTR + fileName;

    sem_wait(&fileInfoMapWriteLock);
    FileInfo *file = fileInfoMap[key];
    vector<string> leechersList = file->getLeechersList();
    bool exists = false;
    for (LL i = 0; i < leechersList.size(); i++)
    {
        if (leechersList[i] == userId)
        {
            exists = true;
            break;
        }
    }
    if (!exists)
    {
        leechersList.push_back(userId);
    }
    file->setLeechersList(leechersList);
    fileInfoMap[key] = file;
    flushFileInfoMapTofile();
    sem_post(&fileInfoMapWriteLock);
}

void addSeederToFileInfoMap(string groupId, string fileName, string userId)
{

    string key = groupId + DELIMITERSTR + fileName;

    sem_wait(&fileInfoMapWriteLock);
    FileInfo *file = fileInfoMap[key];
    vector<string> leechersList = file->getLeechersList();

    bool exists = false;
    for (LL i = 0; i < leechersList.size(); i++)
    {
        if (leechersList[i] == userId)
        {
            leechersList.erase(leechersList.begin() + i);
            exists = true;
            break;
        }
    }
    vector<string> seedersList = file->getSeedersList();
    if (exists)
    {
        exists = false;
        for (LL i = 0; i < seedersList.size(); i++)
        {
            if (seedersList[i] == userId)
            {
                exists = true;
                break;
            }
        }
        if (!exists)
        {
            seedersList.push_back(userId);
        }
    }
    file->setLeechersList(leechersList);
    file->setSeedersList(seedersList);
    fileInfoMap[key] = file;
    flushFileInfoMapTofile();
    sem_post(&fileInfoMapWriteLock);
}
