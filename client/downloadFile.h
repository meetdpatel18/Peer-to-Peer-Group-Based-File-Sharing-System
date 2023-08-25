bool downloadFileFromOtherPeer(string fileName, LL fileSize, string PeerIP, int PeerPORT)
{
    int my_sd;
    if ((my_sd = setupConnectionWithPeersServer(PeerIP, PeerPORT)) == -1)
        return false;

    return true;
}

vector<bool> getChunkInfoFromPeers(string fileName, string PeerIP, int PeerPORT)
{
    vector<bool> chunkMap;
    int my_sd;
    if ((my_sd = setupConnectionWithPeersServer(PeerIP, PeerPORT)) == -1)
        return chunkMap;

    string command = "download_bitmap " + fileName;
    send(my_sd, command.c_str(), command.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(my_sd, trackerResponse, sizeof(trackerResponse));

    string responseStatus = trackerResponse;
    // response will contain
    // response[0] = TRUE/FALSE
    // response[1] = Bitmap of that chunk if response[0] is true
    vector<string> response = stringToVector(responseStatus);

    vector<bool> res;

    string chunkInfo = response[1];
    logTrackerResponse(responseStatus); // log client's server response
    res = stringToBool(chunkInfo);
    return res;
}

unordered_map<string, pair<string, int>> getClientInfo(vector<string> &response)
{
    int n = response.size();
    unordered_map<string, pair<string, int>> clientInfo;
    string userId, userIP;
    int userPORT;

    // response contains TRUEFALSE|fileName|fileSize|SHA1|uid1|ip1|port1|uid2|ip2|port2|........
    for (int i = 4; i < n; i += 3)
    {
        userId = response[i];
        userIP = response[i + 1];
        if (userIP == "$")
            continue;
        userPORT = atoi(response[i + 2].c_str());

        clientInfo[userId] = make_pair(userIP, userPORT);
    }
    return clientInfo;
}

struct ComparePair
{
    bool operator()(pair<int, int> a, pair<int, int> b)
    {
        return a.second < b.second;
    }
};

void setChunkToPeerMap(unordered_map<int, vector<string>> &chunkToPeer, vector<bool> chunkInfo, string userId)
{
    for (int i = 0; i < chunkInfo.size(); i++)
    {
        if (chunkInfo[i] == true)
        {
            chunkToPeer[i].push_back(userId);
        }
    }
}

void setChunkCountHeap(priority_queue<pair<int, int>, vector<pair<int, int>>, ComparePair> &chunkCnt, unordered_map<int, vector<string>> &chunkToPeer)
{
    for (auto it : chunkToPeer)
    {
        int chunkNo = it.first;
        int peersSize = it.second.size();
        chunkCnt.push(make_pair(chunkNo, peersSize));
    }
}

bool performDownloadTask(vector<string> &response, string fileName, LL fileSize, string filePath, string groupId, string userId)
{
    // stores the map of (userId, (IP,PORT))
    clientInfoMap = getClientInfo(response);

    if (clientInfoMap.empty())
    {
        Logger::ERROR("No Peers available online at this moment");
        return false;
    }

    // stores the min heap (chunk number, Number of peers having that chunk) min heap on second value
    priority_queue<pair<int, int>, vector<pair<int, int>>, ComparePair> chunkCnt;

    // stores the map of (chunk number, List of peers having that chunk)
    unordered_map<int, vector<string>> chunkToPeer;

    for (auto it : clientInfoMap)
    {
        string userId = it.first;
        string peerIP = it.second.first;
        int peerPORT = it.second.second;

        // go to this particular client and get the bitmap of the chunks he has
        vector<bool> chunkInfo = getChunkInfoFromPeers(fileName, peerIP, peerPORT);

        // this function setChunkToPeerMap will get which chunk is present with which list of users
        setChunkToPeerMap(chunkToPeer, chunkInfo, userId);

        // this function setChunkCountHeap will make heap of (chunk number, number of peers) so that we
        // can apply rarest first algorithm
        setChunkCountHeap(chunkCnt, chunkToPeer);
    }

    // at this point clientInfo contains username, IP and PORT of various peers which have some/all part of data
    // chunkToPeer contains information that which chunk is present with which users
    // chunkCnt contains information that which chunk is rarest in the group

    // Create an empty file with size=fileSize
    FILE *file = fopen((filePath + "/" + fileName).c_str(), "w");
    ftruncate(fileno(file), fileSize);
    fclose(file);

    while (!chunkCnt.empty())
    {
        int chunkNumber = chunkCnt.top().first;
        chunkCnt.pop();
        // userInfo contains the list of users that has this particular chunk
        vector<string> userList = getUsersListThatHasChunk(chunkToPeer, chunkNumber);
        chunkDownloadFromPeer(userList, clientInfoMap, fileName, chunkNumber, filePath, groupId, userId);
    }
    return true;
}

void chunkDownloadFromPeer(vector<string> userList, unordered_map<string, pair<string, int>> &clientInfo, string fileName, int chunkNumber, string filePath, string groupId, string userId)
{
    // taking users from the end of the the userList because those will be the users who have downloaded this
    // chunk most recently. Hence we will be downloading this chunk from those users first instead of taking
    // it from seeders

    for (int i = userList.size() - 1; i >= 0; i--)
    {
        string PeerIP = clientInfoMap[userList[i]].first;
        int PeerPORT = clientInfoMap[userList[i]].second;

        int my_sd;
        if ((my_sd = setupConnectionWithPeersServer(PeerIP, PeerPORT)) == -1)
            continue;

        string command = "download_chunk " + fileName + " " + to_string(chunkNumber);

        // sending download_chunk command to peersServer from whom we want to download the file
        send(my_sd, command.c_str(), command.size(), 0);

        char trackerResponse[MAXBUFFERSIZE] = {0};

        // trackerResponse will contain SHA1 of the chunk
        read(my_sd, trackerResponse, sizeof(trackerResponse));

        string responseStatus = trackerResponse;
        // response will contain
        // response[0] = TRUE/FALSE
        // response[1] = SHA1 of that chunk if response[0] is true
        vector<string> response = stringToVector(responseStatus);
        if (response[0] == TRUE)
        {
            string ACK = "TRUE|SHA1 received";

            // send ACK to peersServer
            send(my_sd, ACK.c_str(), ACK.size(), 0);

            // receive the chunk data from the peer and if true this chunk is received successfully
            // return and fetch the next chunk
            if (receiveChunkFromClientsServer(my_sd, fileName, chunkNumber, response, groupId, userId))
                return;
        }
    }
}

bool receiveChunkFromClientsServer(int my_sd, string fileName, LL chunkNumber, vector<string> &response, string groupId, string userId)
{
    string filePath = getFilePathFromFileInfo(fileName);

    LL fileSize = getFileSize(fileName);
    LL totalChunksInFile = ceil(((float)fileSize) / MAXDOWNLOADBUFFERSIZE);

    LL dataToBeRead = 0;

    // THIS WILL BE THE LAST CHUNK OF THE FILE
    if (chunkNumber == totalChunksInFile - 1)
    {
        dataToBeRead = fileSize % MAXDOWNLOADBUFFERSIZE;
    }

    // THIS WILL BE THE INTERMEDIATE CHUNK OF THE FILE
    else
    {
        dataToBeRead = MAXDOWNLOADBUFFERSIZE;
    }

    char chunk[MAXDOWNLOADBUFFERSIZE + 1] = {0};
    LL totalBytesRead = 0;
    LL bytesRead = 0;

    LL i = 0; // Pointer to chunk char array
    char temp[MAXDOWNLOADBUFFERSIZE + 1] = {0};

    while (1)
    {
        bytesRead = read(my_sd, temp, dataToBeRead);

        for (LL j = 0; j < bytesRead; j++)
        {
            chunk[i++] = temp[j];
        }

        totalBytesRead += bytesRead;

        if (totalBytesRead >= dataToBeRead)
        {
            break;
        }

        bzero(temp, sizeof(temp));
    }

    string originalChunkSHA = response[1];
    string calculatedChunkSHA = calHashofchunk(chunk, dataToBeRead, 1);
    cout << "originalChunkSHA: " << originalChunkSHA << endl;
    cout << "calculatedChunkSHA: " << calculatedChunkSHA << endl;

    if (originalChunkSHA == calculatedChunkSHA)
    {
        string log = "Chunk Number: " + to_string(chunkNumber) + " received successfully for fileName: " + fileName;
        Logger::DEBUG(log.c_str());

        int fd = open(filePath.c_str(), O_CREAT | O_WRONLY);
        if (fd == -1)
        {
            Logger::ERROR("Error in opening file at destination: %s", filePath);
            return false;
        }
        int bytesWritten = pwrite(fd, chunk, dataToBeRead, chunkNumber * MAXDOWNLOADBUFFERSIZE);
        close(fd);

        // inform the tracker that this user is now the seeder/leecher
        string command = "add_leacher " + groupId + SPACESTR + fileName + SPACESTR + userId;

        send(client_sd, command.c_str(), command.size(), 0);

        char trackerResponse[MAXBUFFERSIZE] = {0};
        read(client_sd, trackerResponse, sizeof(trackerResponse));

        addChunkToBitMap(fileName, chunkNumber);
        return true;
    }
    return false;
}

vector<string> getUsersListThatHasChunk(unordered_map<int, vector<string>> &chunkToPeer, int chunkNumber)
{
    vector<string> lst;
    if (chunkToPeer.find(chunkNumber) != chunkToPeer.end())
        lst = chunkToPeer[chunkNumber];
    return lst;
}