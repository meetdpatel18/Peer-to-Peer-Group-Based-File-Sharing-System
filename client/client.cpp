#include "clientHeader.h"
#include "logger.h"
#include "clientDataStructure.h"
#include "clientHelper.h"
#include "clientSemaphore.h"
#include "clientsServer.h"
#include "clientCommands.h"
#include "calcSHA1.h"
#include "downloadFile.h"

int main(int argc, char **argv)
{
    // Initialize semaphore variables for local Map Data Structures
    initSemaphores();

    // Enable Logger
    Logger::EnableFileOutput();

    // Get IP and Port of Client's Server on which the server of the client will be listening
    getClientsServerIPandPortFromArgs(argv[1]);

    // Get IP and Port of tracker and save it in global vars TrackerIP and TrackerPORT
    setTrackerIPandPORT(argv[2]);

    // Creating Client's Server which will always be in listening state and serve other peer's request
    pthread_create(&clientsServerThreadId, NULL, createClientsServer, NULL);

    // Create a connection with tracker only once at the beginning and keep it connected always
    if (!connectClientWithTracker())
        exit(1);

    sleep(0.8);

    string input;
    while (1)
    {
        cout << "\nCommand $: ";
        input = "";
        getline(cin, input);

        vector<string> args = stringToVector(input, ' ');
        int argsLength = args.size();
        if (argsLength == 0)
        {
            Logger::ERROR("Enter a command and try again");
            continue;
        }
        string command = args[0];

        if (command == "create_user")
        {
            if (!isCommandArgsValid(command, argsLength, 3))
                continue;

            createUser(input);
        }
        else if (command == "login")
        {
            if (!isCommandArgsValid(command, argsLength, 3))
                continue;

            if (isUserLoggedIn)
            {
                Logger::WARN("User already logged in");
                continue;
            }
            input = input + " " + clientsServerIP + " " + to_string(clientsServerPORT);
            loginUser(input);
        }

        else if (command == "quit")
        {
            Logger::SUCCESS("Quit Executed Sucessfully");
            exit(0);
        }

        else if (isUserLoggedIn)
        {
            if (command == "create_group")
            {
                if (!isCommandArgsValid(command, argsLength, 2))
                    continue;

                input = input + " " + loggedInUserId;
                createGroup(input);
            }
            else if (command == "join_group")
            {
                if (!isCommandArgsValid(command, argsLength, 2))
                    continue;

                input = input + " " + loggedInUserId;
                joinGroup(input);
            }

            else if (command == "leave_group")
            {
                if (!isCommandArgsValid(command, argsLength, 2))
                    continue;

                input = input + " " + loggedInUserId;
                leaveGroup(input);
            }

            else if (command == "list_requests")
            {
                if (!isCommandArgsValid(command, argsLength, 2))
                    continue;

                input = input + " " + loggedInUserId;
                listPendingJoinRequests(input);
            }
            else if (command == "accept_request")
            {
                if (!isCommandArgsValid(command, argsLength, 3))
                    continue;

                input = input + " " + loggedInUserId;
                acceptGroupJoinRequests(input);
            }

            else if (command == "list_groups")
            {
                if (!isCommandArgsValid(command, argsLength, 1))
                    continue;

                listGroups(input);
            }
            else if (command == "list_files")
            {
                if (!isCommandArgsValid(command, argsLength, 2))
                    continue;

                input = input + " " + loggedInUserId;
                listFiles(input);
            }

            else if (command == "upload_file")
            {
                if (!isCommandArgsValid(command, argsLength, 3))
                    continue;

                input = input + " " + loggedInUserId;
                uploadFile(input);
            }
            else if (command == "download_file")
            {
                if (!isCommandArgsValid(command, argsLength, 4))
                    continue;

                input = input + " " + loggedInUserId;
                downloadFile(input);
            }
            else if (command == "logout")
            {
                if (!isCommandArgsValid(command, argsLength, 1))
                    continue;

                input = input + " " + loggedInUserId;
                logoutUser(input);
            }
            else if (command == "stop_share")
            {
                if (!isCommandArgsValid(command, argsLength, 3))
                    continue;

                input = input + " " + loggedInUserId;
                stopShare(input);
            }
            else
            {
                Logger::ERROR("Command Invalid. Please try again");
            }
        }
        else
        {
            Logger::WARN("Please login first to execute the command");
        }
    }
    return 0;
}