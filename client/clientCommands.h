void createUser(string commandInfo)
{
    Logger::INFO("Please wait while user is being created...");

    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    logTrackerResponse(trackerResponse);
}

void loginUser(string commandInfo)
{
    Logger::INFO("Please wait while you are being logged in...");
    vector<string> commandArgs = stringToVector(commandInfo, ' ');

    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    // Logging Tracker's Response
    string responseStatus = trackerResponse;
    vector<string> response = stringToVector(responseStatus);
    logTrackerResponse(responseStatus);

    // Set isUserLoggedIn to true so that user cant login again
    if (response[0] == TRUE)
    {
        isUserLoggedIn = true;
        loggedInUserId = commandArgs[1];
        Logger::DEBUG("Logged In User Id: %s", loggedInUserId.c_str());
    }
}

void createGroup(string commandInfo)
{
    Logger::INFO("Please wait while group is being created...");

    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    logTrackerResponse(trackerResponse);
}

void joinGroup(string commandInfo)
{
    Logger::INFO("Please wait while we add you to the group...");

    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    logTrackerResponse(trackerResponse);
}

void leaveGroup(string commandInfo)
{
    Logger::DEBUG("Leave Group start");

    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    logTrackerResponse(trackerResponse);
    Logger::DEBUG("Leave Group end");
}

void listPendingJoinRequests(string commandInfo)
{
    Logger::INFO("Please wait while we list Pending Requests...");

    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    string responseStatus = trackerResponse;
    vector<string> response = stringToVector(responseStatus);

    int responseSize = response.size();
    if (response[0] == TRUE)
    {

        if (responseSize == 1)
        {
            string listHeader = "No Pending Requests\n";
            write(STDOUT_FILENO, listHeader.c_str(), listHeader.size());
        }
        else
        {
            string listHeader = "---LIST OF PENDING CLIENT REQUESTS---\n";
            for (int i = 1; i < responseSize; i++)
            {
                listHeader = listHeader + response[i] + "\n";
            }
            write(STDOUT_FILENO, listHeader.c_str(), listHeader.size());
        }
    }
    else
    {
        logTrackerResponse(trackerResponse);
    }
}

void acceptGroupJoinRequests(string commandInfo)
{
    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    logTrackerResponse(trackerResponse);
}

void listGroups(string commandInfo)
{
    Logger::INFO("Please wait while we list all Groups...");

    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    string responseStatus = trackerResponse;
    vector<string> response = stringToVector(responseStatus);

    int responseSize = response.size();
    if (response[0] == TRUE)
    {
        if (responseSize == 1)
        {
            string listHeader = "No Groups Exist\n";
            Logger::INFO(listHeader.c_str());
        }
        else
        {
            string listHeader = "---LIST OF ALL AVAILABLE GROUPS---\n";
            for (int i = 1; i < responseSize; i++)
            {
                listHeader = listHeader + response[i] + "\n";
            }
            write(STDOUT_FILENO, listHeader.c_str(), listHeader.size());
        }
    }
}

void listFiles(string commandInfo)
{
    Logger::INFO("Please wait while we list all Files in this Group...");

    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    string responseStatus = trackerResponse;
    vector<string> response = stringToVector(responseStatus);
    int responseSize = response.size();

    if (response[0] == TRUE)
    {
        if (responseSize == 1)
        {
            string listHeader = "No Files Exist in this Group\n";
            Logger::INFO(listHeader.c_str());
        }
        else
        {
            string listHeader = "---LIST OF ALL AVAILABLE FILES IN THIS GROUP---\n";
            for (int i = 1; i < responseSize; i++)
            {
                listHeader = listHeader + response[i] + "\n";
            }
            write(STDOUT_FILENO, listHeader.c_str(), listHeader.size());
        }
    }
    else
    {
        logTrackerResponse(trackerResponse);
    }
}

void uploadFile(string commandInfo)
{
    // commandInfo = "upload_file file_path group_id userId"
    vector<string> args = stringToVector(commandInfo, ' ');
    string filePath = args[1];
    string userId = args[3];

    char buf[PATH_MAX];

    char *res = realpath(filePath.c_str(), buf);
    if (!res)
    {
        Logger::ERROR("No such file exists at this path");
        return;
    }

    filePath = res;
    string fileName = filePath.substr(filePath.find_last_of("/") + 1, filePath.size());
    if (fileAlreadyShared(fileName))
    {
        Logger::INFO("This File is already Shared");
        return;
    }
    struct stat sb
    {
    };

    if (stat(filePath.c_str(), &sb))
    {
        Logger::ERROR("Error in getting file size");
        return;
    }
    string SHA1 = calcCompleteFileSHA1(filePath);

    LL fileSize = sb.st_size;

    // fileMetaData = upload_file fileName group_id fileSize SHA1 userId
    string fileMetaData = args[0] + SPACESTR + fileName + SPACESTR + args[2] + SPACESTR + to_string(fileSize) + SPACESTR + SHA1 + SPACESTR + userId;
    Logger::DEBUG(fileMetaData.c_str());

    send(client_sd, fileMetaData.c_str(), fileMetaData.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    string responseStatus = trackerResponse;
    vector<string> response = stringToVector(responseStatus);

    // response[0] = TRUE -> Upload file has been a success operation
    // response[1] = Message from tracker
    if (response[0] == TRUE)
    {
        addFileMetaDataToMap(fileName, filePath, fileSize, SHA1, true);

        // TESTING PURPOSE ONLY
        // printFileMetaData(fileName);
    }

    logTrackerResponse(trackerResponse);
}

void downloadFile(string commandInfo)
{
    // commandInfo = "download_file group_id file_name destination_path userId"

    vector<string> args = stringToVector(commandInfo, ' ');
    string groupId = args[1];
    string fileName = args[2];
    string filePath = args[3];
    string userId = args[4];
    cout << "userId: " << userId << endl;
    char buf[PATH_MAX];

    char *res = realpath(filePath.c_str(), buf);
    if (!res)
    {
        Logger::ERROR("No such directory exists, Please try some other destination");
        return;
    }

    // send command to tracker to get the list of peers who has the file
    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXDOWNLOADBUFFERSIZE] = {0};
    // trackerResponse contains TRUEFALSE|fileName|fileSize|SHA1|uid1|ip1|port1|uid2|ip2|port2|........
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    string responseStatus = trackerResponse;
    Logger::DEBUG(responseStatus.c_str());
    vector<string> response = stringToVector(responseStatus);

    if (response[0] == TRUE)
    {
        // Initialize local File Info Map for each individual client
        string destPath = res;
        destPath += "/" + fileName;

        initClientFileInfoObject(response, destPath, false);

        string fileName = response[1];
        LL fileSize = atoll(response[2].c_str());
        if (performDownloadTask(response, fileName, fileSize, filePath, groupId, userId))
        {
            // CHECK FILE SHA1 of entire file received
            // if received correctly add user to seeder list
            string originalFileSHA1 = response[3];
            string downloadedFileSHA1 = calcCompleteFileSHA1(destPath);
            if (originalFileSHA1 == downloadedFileSHA1 && allChunksReceived(fileName))
            {
                // inform the tracker that this user is now the seeder
                string command = "add_seeder " + groupId + SPACESTR + fileName + SPACESTR + userId;

                send(client_sd, command.c_str(), command.size(), 0);

                char trackerResponse[MAXBUFFERSIZE] = {0};
                read(client_sd, trackerResponse, sizeof(trackerResponse));
                responseStatus = trackerResponse;
                logTrackerResponse(responseStatus);
                string log = "File: " + fileName + " Downloaded Succesfully at Destination: " + filePath;
                Logger::SUCCESS(log.c_str());
            }
            else
            {
                string log = "File: " + fileName + " Download Failed";
                Logger::ERROR(log.c_str());
            }
        }
        else
        {
            string log = "File: " + fileName + " Download Failed";
            Logger::ERROR(log.c_str());
        }
    }
    else
    {
        logTrackerResponse(trackerResponse);
    }
}

void logoutUser(string commandInfo)
{
    Logger::INFO("Please wait while you are being logged out...");
    vector<string> commandArgs = stringToVector(commandInfo, ' ');

    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    // Logging Tracker's Response
    string responseStatus = trackerResponse;
    vector<string> response = stringToVector(responseStatus);
    logTrackerResponse(responseStatus);

    // Set isUserLoggedIn to true so that user cant login again
    if (response[0] == TRUE)
    {
        isUserLoggedIn = false;
        loggedInUserId = "";
    }
}

void stopShare(string commandInfo)
{
    vector<string> commandArgs = stringToVector(commandInfo, ' ');
    send(client_sd, commandInfo.c_str(), commandInfo.size(), 0);

    char trackerResponse[MAXBUFFERSIZE] = {0};
    read(client_sd, trackerResponse, sizeof(trackerResponse));

    string responseStatus = trackerResponse;
    logTrackerResponse(responseStatus);
}

vector<string> getChunkWiseSHA(int client_sd, vector<string> response)
{
    LL fileSize = atoll(response[2].c_str());
    vector<string> chunksHash;
    LL totalChunks = ceil(float(fileSize) / MAXDOWNLOADBUFFERSIZE);
    char hashOfChunk[MAXBUFFERSIZE] = {0};

    for (LL i = 0; i < totalChunks; i++)
    {
        read(client_sd, hashOfChunk, sizeof(hashOfChunk));
        chunksHash.push_back(hashOfChunk);
        bzero(hashOfChunk, sizeof(hashOfChunk));
    }
    return chunksHash;
}

bool isCommandArgsValid(string command, int argsLength, int reqLen)
{
    if (argsLength < reqLen)
    {
        string response = "Too few arguments for " + command + " command";
        Logger::WARN(response.c_str());
        return false;
    }
    else if (argsLength > reqLen)
    {
        string response = "Too many arguments for " + command + " command";
        Logger::WARN(response.c_str());
        return false;
    }
    return true;
}