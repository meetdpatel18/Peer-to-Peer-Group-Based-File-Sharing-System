vector<string> getTrackerInfo(string trackerFileName)
{
    vector<string> trackerInfo(2);
    fstream file;
    file.open(trackerFileName, ios::in);
    getline(file, trackerInfo[0]);
    getline(file, trackerInfo[1]);
    file.close();

    return trackerInfo;
}

vector<string> stringToVector(string str, char delimiter = DELIMITERCHAR)
{
    vector<string> res;
    string token;
    stringstream ss(str);

    while (getline(ss, token, delimiter))
    {
        if (token != "")
            res.push_back(token);
    }
    return res;
}

string vectorToString(vector<string> vect, string delimiter = DELIMITERSTR)
{
    int n = vect.size();
    string res = vect[0];
    if (n == 1)
        return res;

    res = res + delimiter;
    for (int i = 1; i < n; i++)
    {
        if (i == n - 1)
        {
            res = res + vect[i];
        }
        else
        {
            res = res + vect[i] + delimiter;
        }
    }
    return res;
}

void initClientInfoFromFile()
{
    ifstream file;
    file.open("clientInfo.txt", ios::in);
    if (!file.is_open())
    {
        return;
    }
    string line;
    while (getline(file, line))
    {
        vector<string> vect = stringToVector(line, '|');
        if (vect.size() != 5)
            break;
        string user = vect[0];

        ClientInfo *client = new ClientInfo(vect[0], vect[1]);
        clientInfoMap[user] = client;
    }
    file.close();

    flushClientInfoMapTofile();
}

void flushClientInfoMapTofile()
{
    ofstream file;
    file.open("clientInfo.txt", ios::trunc);
    if (!file.is_open())
    {
        Logger::DEBUG("Error in flushing clientInfoMap to file");
        return;
    }

    for (auto it : clientInfoMap)
    {
        ClientInfo *client = it.second;
        file << client->getClientId() << "|";
        file << client->getPassword() << "|";

        string IP = client->getIP();
        if (IP == "$")
            file << "$|";
        else
            file << IP << "|";

        int PORT = client->getPORT();
        if (PORT == -1)
            file << "$|";
        else
            file << PORT << "|";
        file << client->getIsUserLoggedIn() << "\n";
    }
    file.close();
}

void initGroupInfoFromFile()
{
    ifstream file;
    file.open("groupInfo.txt", ios::in);
    if (!file.is_open())
    {
        return;
    }
    string line;

    while (getline(file, line))
    {
        // get groupId
        string groupId = line;
        string ownerId;
        vector<string> membersList, pendingRequests, filesList;

        // get ownerId
        if (getline(file, line))
            ownerId = line;
        else
            break;

        // get membersList
        if (getline(file, line))
            membersList = stringToVector(line, '|');
        else
            break;

        // get pending List
        if (getline(file, line))
            pendingRequests = stringToVector(line, '|');
        else
            break;

        // get files List
        if (getline(file, line))
            filesList = stringToVector(line, '|');
        else
            break;

        GroupInfo *group = new GroupInfo();
        group->setGroupId(groupId);
        group->setOwnerId(ownerId);
        group->setMembersList(membersList);
        group->setPendingRequestsList(pendingRequests);
        group->setFilesList(filesList);

        groupInfoMap[groupId] = group;
    }
    file.close();
}

void flushGroupInfoMapTofile()
{
    ofstream file;
    file.open("groupInfo.txt", ios::trunc);
    if (!file.is_open())
    {
        Logger::DEBUG("Error in flushing groupInfoMap to file");
        return;
    }

    for (auto it : groupInfoMap)
    {
        GroupInfo *group = it.second;
        file << group->getGroupId() << "\n";
        file << group->getOwnerId() << "\n";

        vector<string> groupMembers = group->getGroupMembers();
        if (groupMembers.size() == 0)
            file << "|\n";
        else
            file << vectorToString(groupMembers) << "\n";

        vector<string> requestList = group->getPendingRequestsList();
        if (requestList.size() == 0)
            file << "|\n";
        else
            file << vectorToString(requestList) << "\n";

        vector<string> filesList = group->getGroupFiles();
        if (filesList.size() == 0)
            file << "|\n";
        else
            file << vectorToString(filesList) << "\n";
    }
    file.close();
}

void initFileInfoFromFile()
{
    ifstream file;
    file.open("fileInfo.txt", ios::in);
    if (!file.is_open())
    {
        return;
    }
    string line;

    while (getline(file, line))
    {
        // get key
        string key = line;
        string fileName, SHA1;
        LL fileSize;
        vector<string> seedersList, leechersList;

        // get fileName
        if (getline(file, line))
            fileName = line;
        else
            break;

        // get SHA1
        if (getline(file, line))
            SHA1 = line;
        else
            break;

        // get SHA1
        if (getline(file, line))
            fileSize = atoll(line.c_str());
        else
            break;

        // get seedersList
        if (getline(file, line))
            seedersList = stringToVector(line, '|');
        else
            break;

        // get leechersList
        if (getline(file, line))
            leechersList = stringToVector(line, '|');
        else
            break;

        FileInfo *f = new FileInfo(fileName, fileSize, SHA1, "");
        f->setSeedersList(seedersList);
        f->setLeechersList(leechersList);
        fileInfoMap[key] = f;
    }
    file.close();
}

void flushFileInfoMapTofile()
{
    ofstream file;
    file.open("fileInfo.txt", ios::trunc);
    if (!file.is_open())
    {
        Logger::DEBUG("Error in flushing fileInfoMap to file");
        return;
    }

    for (auto it : fileInfoMap)
    {
        FileInfo *f = it.second;

        file << it.first << "\n";
        file << f->getFileName() << "\n";
        file << f->getSHA1() << "\n";
        file << f->getFileSize() << "\n";

        vector<string> seedersList = f->getSeedersList();
        if (seedersList.size() == 0)
            file << "|\n";
        else
            file << vectorToString(seedersList) << "\n";

        vector<string> leechersList = f->getLeechersList();
        if (leechersList.size() == 0)
            file << "|\n";
        else
            file << vectorToString(leechersList) << "\n";
    }
    file.close();
}