void setTrackerIPandPORT(string trackerFileName)
{
    vector<string> trackerInfo = getTrackerInfo(trackerFileName);
    TrackerIP = trackerInfo[0];
    TrackerPORT = atoi(trackerInfo[1].c_str());
}

void getClientsServerIPandPortFromArgs(char *argv)
{
    string port;
    stringstream SS(argv);
    getline(SS, clientsServerIP, ':');
    getline(SS, port, ':');
    clientsServerPORT = atoi(port.c_str());
}

int createSocket()
{
    if ((client_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        Logger::ERROR("Socket creation error");
        return -1;
    }
    return client_sd;
}

bool connectClientWithTracker()
{
    int server_sd;
    client_sd = createSocket();
    Logger::SUCCESS("client_sd on client side %d", client_sd);

    if (client_sd == -1)
        return false;

    struct sockaddr_in trackerAddress;
    trackerAddress.sin_family = AF_INET;
    trackerAddress.sin_port = htons(TrackerPORT);

    if (inet_pton(AF_INET, TrackerIP.c_str(), &trackerAddress.sin_addr) <= 0)
    {
        Logger::ERROR("Invalid Addres, Not Supported");
        return false;
    }
    if ((server_sd = connect(client_sd, (struct sockaddr *)&trackerAddress,
                             sizeof(trackerAddress))) < 0)
    {
        Logger::ERROR("Connection Failed");
        return false;
    }

    Logger::SUCCESS("Connection Established");
    Logger::DEBUG("Tracker's IP: %s", TrackerIP.c_str());
    Logger::DEBUG("Tracker's PORT: %s", to_string(TrackerPORT).c_str());
    return true;
}

int setupConnectionWithPeersServer(string PeerIP, int PeerPORT)
{
    int peer_serv_sd, my_sd;

    if ((my_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        Logger::ERROR("Socket creation error");
        return -1;
    }

    struct sockaddr_in peerServerAddress;
    peerServerAddress.sin_family = AF_INET;
    peerServerAddress.sin_port = htons(PeerPORT);

    if (inet_pton(AF_INET, PeerIP.c_str(), &peerServerAddress.sin_addr) <= 0)
    {
        Logger::ERROR("Invalid Peer Address, Not Supported");
        return -1;
    }

    if ((peer_serv_sd = connect(my_sd, (struct sockaddr *)&peerServerAddress,
                                sizeof(peerServerAddress))) < 0)
    {
        Logger::ERROR("Connection Failed with Peer");

        Logger::DEBUG("peer_serv_sd: %d", peer_serv_sd);
        return -1;
    }
    return my_sd;
}

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

string vectorToString(vector<string> vect, char delimiter = DELIMITERCHAR)
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

string boolVectToString(vector<bool> &chunkInfo)
{
    string res = "";
    for (auto it : chunkInfo)
    {
        if (it == true)
            res = res + "1";
        else
            res = res + "0";
    }
    return res;
}

vector<bool> stringToBool(string chunkInfo)
{
    vector<bool> res;
    for (auto it : chunkInfo)
    {
        if (it == '0')
            res.push_back(0);
        else
            res.push_back(1);
    }
    return res;
}
void logTrackerResponse(string responseStatus, int messageIndexInResponse = 1)
{
    vector<string> response = stringToVector(responseStatus);

    if (response[0] == TRUE)
    {
        Logger::SUCCESS(response[messageIndexInResponse].c_str());
    }
    else if (response[0] == FALSE)
    {
        Logger::ERROR(response[messageIndexInResponse].c_str());
    }
}
