class ClientInfo
{
private:
    string clientId, password, IP;
    int PORT;
    bool isUserLoggedIn;

public:
    ClientInfo() {}
    ClientInfo(string clientId, string password)
    {
        this->clientId = clientId;
        this->password = password;
        this->IP = "$";
        this->PORT = -1;
        this->isUserLoggedIn = false;
    }

    // SETTER METHODS
    void setClientId(string clientId)
    {
        this->clientId = clientId;
    }
    void setPassword(string password)
    {
        this->password = password;
    }
    void setIP(string IP)
    {
        this->IP = IP;
    }
    void setPORT(int PORT)
    {
        this->PORT = PORT;
    }
    void setisUserLoggedIn(bool isUserLoggedIn)
    {
        this->isUserLoggedIn = isUserLoggedIn;
    }

    // GETTER METHODS
    string getClientId()
    {
        return this->clientId;
    }
    string getPassword()
    {
        return this->password;
    }
    string getIP()
    {
        return this->IP;
    }
    int getPORT()
    {
        return this->PORT;
    }
    bool getIsUserLoggedIn()
    {
        return this->isUserLoggedIn;
    }
};

class GroupInfo
{
private:
    string groupId, ownerId;
    // filesId is combination of key "groupId|fileName" '|' is delimeter seperator
    vector<string> groupMembersId, pendingRequestsList, filesId;

public:
    GroupInfo() {}
    GroupInfo(string groupId, string ownerId)
    {
        this->groupId = groupId;
        this->ownerId = ownerId;
    }

    // SETTER METHODS
    void setGroupId(string groupId)
    {
        this->groupId = groupId;
    }

    void setOwnerId(string ownerId)
    {
        this->ownerId = ownerId;
    }

    void setMembersList(vector<string> groupMembersId)
    {
        this->groupMembersId = groupMembersId;
    }
    void setPendingRequestsList(vector<string> pendingRequestsList)
    {
        this->pendingRequestsList = pendingRequestsList;
    }

    void setFilesList(vector<string> filesId)
    {
        this->filesId = filesId;
    }

    void addOwnerToGroup(string userId)
    {
        this->ownerId = userId;
        this->groupMembersId.push_back(userId);
    }

    void addUserToPendingList(string userId)
    {
        bool existsInPendingList = false;
        for (auto it : pendingRequestsList)
        {
            if (it == userId)
            {
                existsInPendingList = true;
                break;
            }
        }
        if (!existsInPendingList)
        {
            this->pendingRequestsList.push_back(userId);
        }
    }

    void addUserToGroup(string userId)
    {
        for (int i = 0; i < pendingRequestsList.size(); i++)
        {
            if (pendingRequestsList[i] == userId)
            {
                this->pendingRequestsList.erase(pendingRequestsList.begin() + i);
                break;
            }
        }
        this->groupMembersId.push_back(userId);
    }

    void addNewFileToGroup(string fileName)
    {
        bool existsInFilesList = false;
        for (auto it : filesId)
        {
            if (it == fileName)
            {
                existsInFilesList = true;
                break;
            }
        }
        if (!existsInFilesList)
        {
            this->filesId.push_back(fileName);
        }
    }

    // GETTER METHODS
    string getGroupId()
    {
        return this->groupId;
    }
    string getOwnerId()
    {
        return this->ownerId;
    }
    vector<string> getGroupMembers()
    {
        return this->groupMembersId;
    }
    vector<string> getPendingRequestsList()
    {
        return this->pendingRequestsList;
    }

    vector<string> getGroupFiles()
    {
        return this->filesId;
    }

    bool doesUserExistInGroup(string userId)
    {
        for (auto it : groupMembersId)
        {
            if (it == userId)
            {
                return true;
            }
        }
        return false;
    }
};

class FileInfo
{
private:
    string fileName, SHA1;
    LL fileSize;
    vector<string> seedersList, leechersList, chunksHash;

public:
    FileInfo(string fileName, LL fileSize, string SHA1, string loggedInUserId)
    {
        this->fileName = fileName;
        this->fileSize = fileSize;
        this->SHA1 = SHA1;
        this->seedersList.push_back(loggedInUserId);
    }

    // SETTER METHODS
    void setFileName(string fileName)
    {
        this->fileName = fileName;
    }

    void addMemberToSeedersList(string seeder)
    {
        bool existsInSeedersList = false;
        for (auto it : seedersList)
        {
            if (it == seeder)
            {
                existsInSeedersList = true;
                break;
            }
        }
        if (!existsInSeedersList)
        {
            this->seedersList.push_back(seeder);
        }
    }

    void setFileSize(LL fileSize)
    {
        this->fileSize = fileSize;
    }

    void setSHA1(string SHA1)
    {
        this->SHA1 = SHA1;
    }

    void setChunksSHA(vector<string> chunksHash)
    {
        this->chunksHash = chunksHash;
    }

    void setSeedersList(vector<string> seedersList)
    {
        this->seedersList = seedersList;
    }
    void setLeechersList(vector<string> leechersList)
    {
        this->leechersList = leechersList;
    }
    // GETTER METHODS
    string getFileName()
    {
        return this->fileName;
    }
    vector<string> getSeedersList()
    {
        return this->seedersList;
    }
    vector<string> getLeechersList()
    {
        return this->leechersList;
    }
    LL getFileSize()
    {
        return this->fileSize;
    }
    string getSHA1()
    {
        return this->SHA1;
    }
    vector<string> getChunksSHA()
    {
        return this->chunksHash;
    }
};