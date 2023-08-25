
void *respondClientRequests(void *args)
{
    int client_sd = *(int *)args;

    while (1)
    {
        char commandInfo[MAXBUFFERSIZE] = {0};
        read(client_sd, commandInfo, sizeof(commandInfo));
        string input = commandInfo;

        vector<string> commandArgs = stringToVector(input, ' ');

        string command = commandArgs[0];

        if (command == "create_user")
        {
            string userId = commandArgs[1];
            string password = commandArgs[2];
            createUser(client_sd, userId, password);
        }

        else if (command == "login")
        {
            string userId = commandArgs[1];
            string password = commandArgs[2];
            string clientsServerIP = commandArgs[3];
            int clientsServerPORT = atoi(commandArgs[4].c_str());
            loginUser(client_sd, userId, password, clientsServerIP, clientsServerPORT);
        }
        else if (command == "create_group")
        {
            string groupId = commandArgs[1];
            string userId = commandArgs[2];
            createGroup(client_sd, groupId, userId);
        }
        else if (command == "join_group")
        {
            string groupId = commandArgs[1];
            string userId = commandArgs[2];
            joinGroup(client_sd, groupId, userId);
        }
        else if (command == "leave_group")
        {
            string groupId = commandArgs[1];
            string userId = commandArgs[2];
            leave_group(client_sd, groupId, userId);
        }
        else if (command == "list_requests")
        {
            string groupId = commandArgs[1];
            string userId = commandArgs[2];
            listPendingJoinRequests(client_sd, groupId, userId);
        }
        else if (command == "accept_request")
        {
            string groupId = commandArgs[1];
            string userId = commandArgs[2];
            string loggedInUserId = commandArgs[3];
            acceptGroupJoinRequests(client_sd, groupId, userId, loggedInUserId);
        }
        else if (command == "list_groups")
        {
            listGroups(client_sd);
        }
        else if (command == "list_files")
        {
            string groupId = commandArgs[1];
            string userId = commandArgs[2];
            listFiles(client_sd, groupId, userId);
        }
        else if (command == "upload_file")
        {
            Logger::DEBUG("Inside upload_file tracker");
            // upload_file fileName group_id fileSize SHA1 userId
            string fileName = commandArgs[1];
            string groupId = commandArgs[2];
            LL fileSize = atoll(commandArgs[3].c_str());
            string SHA1 = commandArgs[4];
            string loggedInUserId = commandArgs[5];

            uploadFile(client_sd, fileName, groupId, fileSize, SHA1, loggedInUserId);
        }
        else if (command == "download_file")
        {
            //  "download_file group_id file_name destination_path userId"
            string groupId = commandArgs[1];
            string fileName = commandArgs[2];
            string loggedInUserId = commandArgs[4];

            downloadFile(client_sd, groupId, fileName, loggedInUserId);
        }
        else if (command == "logout")
        {
            string loggedInUserId = commandArgs[1];

            logoutUser(client_sd, loggedInUserId);
        }
        else if (command == "stop_share")
        {
            string groupId = commandArgs[1];
            string fileName = commandArgs[2];
            string loggedInUserId = commandArgs[3];

            stopShare(client_sd, groupId, fileName, loggedInUserId);
        }
        else if (command == "add_leacher")
        {
            string groupId = commandArgs[1];
            string fileName = commandArgs[2];
            string userId = commandArgs[3];
            addLeacher(client_sd, groupId, fileName, userId);
        }
        else if (command == "add_seeder")
        {
            string groupId = commandArgs[1];
            string fileName = commandArgs[2];
            string userId = commandArgs[3];
            addSeeder(client_sd, groupId, fileName, userId);
        }
    }
}
