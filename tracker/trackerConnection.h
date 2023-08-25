void initTrackersServer()
{
    createSocket();
    bindSocket();
    listen();
    accept_connection();
}

void createSocket()
{
    if ((tracker_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        Logger::ERROR("Tracker Socket Creation Failed");
        exit(EXIT_FAILURE);
    }
    Logger::SUCCESS("Tracker Socket Created Successfully");
}

void bindSocket()
{
    trackerAddress.sin_family = AF_INET;
    inet_pton(AF_INET, TrackerIP.c_str(), &(trackerAddress.sin_addr));
    trackerAddress.sin_port = htons(TrackerPORT);
    if (setsockopt(tracker_sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (::bind(tracker_sd, (struct sockaddr *)&trackerAddress, sizeof(trackerAddress)) < 0)
    {
        Logger::ERROR("Tracker Socket Bind Failed");
        exit(EXIT_FAILURE);
    }
    Logger::SUCCESS("Tracker Socket Bind Successful");
}

void listen()
{
    if (listen(tracker_sd, MAXCLIENTREQUESTS) < 0)
    {
        Logger::ERROR("Tracker Listen Failed");
        exit(EXIT_FAILURE);
    }
    Logger::INFO("Tracker in Listen State");

    Logger::DEBUG("Tracker Running on IP: %s", TrackerIP.c_str());
    Logger::DEBUG("Tracker Running on PORT: %s", to_string(TrackerPORT).c_str());
}

void accept_connection()
{
    while (1)
    {
        int client_sd;
        if ((client_sd = accept(tracker_sd, (struct sockaddr *)&trackerAddress, &tracker_address_length)) < 0)
        {
            Logger::ERROR("Client Connection with Tracker Failed");
            exit(EXIT_FAILURE);
        }
        Logger::SUCCESS("Client Connected Successfully with Tracker");
        Logger::SUCCESS("client_sd on tracker side %d", client_sd);
        if (pthread_create(&trackerThreadIds[liveTrackerThreads++], NULL, respondClientRequests, &client_sd) < 0)
        {
            Logger::ERROR("Could not create thread");
        }
    }
}