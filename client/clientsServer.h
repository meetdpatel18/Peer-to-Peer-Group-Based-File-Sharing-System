void createServerSocket()
{
    if ((clientsServerSD = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        Logger::ERROR("Client's Server Socket Creation Failed");
        exit(EXIT_FAILURE);
    }
    Logger::SUCCESS("Client's Server Socket Created Successfully");
}

void bindServerSocket()
{
    clientsServerAddress.sin_family = AF_INET;
    inet_pton(AF_INET, clientsServerIP.c_str(), &(clientsServerAddress.sin_addr));
    clientsServerAddress.sin_port = htons(clientsServerPORT);

    if (::bind(clientsServerSD, (struct sockaddr *)&clientsServerAddress, sizeof(clientsServerAddress)) < 0)
    {
        Logger::ERROR("Client's Server Socket Bind Failed");
        exit(EXIT_FAILURE);
    }
    Logger::SUCCESS("Client's Server Socket Bind Successful");
}

void listenServer()
{
    if (listen(clientsServerSD, MAXCLIENTREQUESTS) < 0)
    {
        Logger::ERROR("Client's Server Listen Failed");
        exit(EXIT_FAILURE);
    }
    string log = "Client's Server in Listen State on IP: " + clientsServerIP + " and PORT : " + to_string(clientsServerPORT);
    Logger::SUCCESS(log.c_str());
}

void accept_connectionServer()
{
    while (1)
    {
        Logger::DEBUG("Inside accept_connection of client's server");
        int peer_sd;
        if ((peer_sd = accept(clientsServerSD, (struct sockaddr *)&clientsServerAddress, &client_address_length)) < 0)
        {
            Logger::ERROR("Client Connection with Peer Failed");
            exit(EXIT_FAILURE);
        }
        Logger::SUCCESS("Client Connected Successfully with Peer");

        pthread_create(&peerThreadIds[peerThreads++], NULL, respondPeerRequests, &peer_sd);
    }
}

void *createClientsServer(void *param)
{
    createServerSocket();
    bindServerSocket();
    listenServer();
    accept_connectionServer();
}

void *respondPeerRequests(void *args)
{
    int peer_sd = *(int *)args;

    char commandInfo[MAXBUFFERSIZE] = {0};
    read(peer_sd, commandInfo, sizeof(commandInfo));

    string input = commandInfo;
    vector<string> commandArgs = stringToVector(input, ' ');

    string command = commandArgs[0];
    Logger::DEBUG(command.c_str());

    string response;

    if (command == "download_bitmap")
    {
        string responseToClient, log;

        string fileName = commandArgs[1];
        if (isUserLoggedIn && isShareable(fileName))
        {
            vector<bool> chunkInfo = getChunkInfo(fileName);

            responseToClient = "TRUE|" + boolVectToString(chunkInfo);
            log = "Bitmap sent successfully to client";
            Logger::SUCCESS(log.c_str());
            send(peer_sd, responseToClient.c_str(), responseToClient.size(), 0);
        }
        else
        {
            responseToClient = "FALSE|Peer is not logged in or File is not Shareable at this moment";
            log = "I am not logged in or file is not Shareable at this moment";
            Logger::ERROR(log.c_str());
            send(peer_sd, responseToClient.c_str(), responseToClient.size(), 0);
        }
    }

    else if (command == "download_chunk")
    {
        Logger::DEBUG("Inside download_chunk on client's Server");

        string fileName = commandArgs[1];
        int chunkNumber = atoi(commandArgs[2].c_str());

        string responseToClient, log;

        if (isUserLoggedIn && isShareable(fileName))
        {
            string chunkSHA = getChunksha(fileName, chunkNumber);

            if (chunkSHA == "")
            {
                responseToClient = "FALSE|Error in Chunk SHA for file: " + fileName + " and Chunk: " + to_string(chunkNumber);
                log = "Error in Chunk SHA for file: " + fileName + " and Chunk: " + to_string(chunkNumber);
                Logger::ERROR(log.c_str());
                send(peer_sd, responseToClient.c_str(), responseToClient.size(), 0);
            }
            else
            {
                responseToClient = "TRUE|" + chunkSHA;
                log = "fileName: " + fileName + " chunkNumber: " + to_string(chunkNumber) + " SHA: " + chunkSHA;
                Logger::SUCCESS(log.c_str());

                // send chunk SHA1 to client
                send(peer_sd, responseToClient.c_str(), responseToClient.size(), 0);

                char clientResponse[MAXBUFFERSIZE] = {0};
                // read response from client if SHA1 is received successfully or not
                read(peer_sd, clientResponse, sizeof(clientResponse));

                string responseStatus = clientResponse;
                vector<string> response = stringToVector(responseStatus);
                if (response[0] == TRUE)
                {
                    // if SHA1 is received successfully by the client then send the chunk data to client
                    sendChunkToClient(peer_sd, fileName, chunkNumber);
                }
            }
        }
        else
        {
            responseToClient = "FALSE|Peer is not logged in or File is not Shareable at this moment";
            log = "Not logged in or file is not Shareable at this moment";
            Logger::ERROR(log.c_str());
            send(peer_sd, responseToClient.c_str(), responseToClient.size(), 0);
        }
    }
}

void sendChunkToClient(int peer_sd, string fileName, int chunkNumber)
{
    LL fileSize = getFileSize(fileName);
    LL totalChunksInFile = ceil(((float)fileSize) / MAXDOWNLOADBUFFERSIZE);

    string filePath = getFilePathFromFileInfo(fileName);
    int fd = open(filePath.c_str(), O_RDONLY);

    char chunkData[MAXDOWNLOADBUFFERSIZE + 1] = {0};

    LL dataToBeRead = 0;
    if (chunkNumber == totalChunksInFile - 1)
    {
        dataToBeRead = fileSize % MAXDOWNLOADBUFFERSIZE;
    }

    // THIS WILL BE THE INTERMEDIATE CHUNK OF THE FILE
    else
    {
        dataToBeRead = MAXDOWNLOADBUFFERSIZE;
    }
    LL bytesRead = pread(fd, chunkData, dataToBeRead, chunkNumber * MAXDOWNLOADBUFFERSIZE);
    close(fd);

    send(peer_sd, chunkData, dataToBeRead, 0);
    // Logger::DEBUG(chunkData);
}
