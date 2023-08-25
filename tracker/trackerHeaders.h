#include <bits/stdc++.h>
#include <string>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <pthread.h>

#define MAXCLIENTREQUESTS 100000000
#define TRACKERTHREADIDSCOUNT 100000000
#define LIVECLIENTTHREADIDS 100000000
#define TRACKER_INFO_FILE "tracker_info.txt"
#define MAXBUFFERSIZE 1024
#define MAXDOWNLOADBUFFERSIZE 524288
#define DELIMITERCHAR '|'
#define DELIMITERSTR "|"
#define TRUE "TRUE"
#define FALSE "FALSE"
#define LL long long

using namespace std;

// trackerConnection.h -> Used to create tracker's Server which will always be
// in listening state and serve client's request
void initTrackersServer();
void createSocket();
void bindSocket();
void listen();
void accept_connection();

// respondClientRequests.h -> All client command will reach here and its respective function
// will be called in clientCommands.h
void *respondClientRequests(void *args);

// clientCommands.h -> All client function definition like create_user, login, download... are present in this file
void createUser(int client_sd, string userId, string password);
void loginUser(int client_sd, string userId, string password, string clientsServerIP, int clientsServerPORT);
void createGroup(int client_sd, string groupId, string userId);
void joinGroup(int client_sd, string groupId, string userId);
void leave_group(int client_sd, string groupId, string userId);
void listPendingJoinRequests(int client_sd, string groupId, string userId);
void acceptGroupJoinRequests(int client_sd, string groupId, string newUserId, string loggedInUserId);
void listGroups(int client_sd);
void listFiles(int client_sd, string groupId, string userId);
void uploadFile(int client_sd, string fileName, string groupId, LL fileSize, string SHA1, string loggedInUserId);
void downloadFile(int client_sd, string groupId, string fileName, string loggedInUserId);
void logoutUser(int client_sd, string loggedInUserId);
void stopShare(int client_sd, string groupId, string fileName, string loggedInUserId);
void addLeacher(int client_sd, string groupId, string fileName, string userId);
void addSeeder(int client_sd, string groupId, string fileName, string userId);

// trackerSemaphore.h -> All data structure for tracker will be accessed using semaphore
void initSemaphores();
bool doClientExist(string userId);
void insertNewClient(string userId, string password);
bool isUserValid(string userId, string password);
bool updateClientsServerIPandPORT(string userId, string clientsServerIP, int clientsServerPORT);
bool doesGroupExist(string groupId);
void createNewGroup(string groupId, string userId);
bool doesUserExistsInGroup(string groupId, string userId);
void addUserToPendingRequests(string groupId, string userId);
bool isUserGroupOwner(string groupId, string userId);
void removeGroupMember(string groupId, string userId);
void removeGroupOwner(string groupId, string ownerId);
bool isSharedByUserAlone(string groupId, string fileName, string userId);
void removeFilesFromFileInfoMap(string groupId, vector<string> filesShared);
void removeFileFromGroupInfoMap(string groupId, string fileName);
bool stopSharingFile(string groupId, string fileName, string userId);
vector<string> getListOfPendingRequests(string groupId);
void addUserToGroup(string groupId, string newUserId);
vector<string> getGroupsList();
vector<string> getFilesInGroup(string groupId);
void addUserToPeersFileList(string groupId, string fileName, string loggedInUserId);
bool fileAlreadyExistsInGroup(string groupId, string fileName);
bool SHA1MatchesWithExistingFile(string groupId, string fileName, string SHA1);
void addNewFile(string groupId, string fileName, LL fileSize, string SHA1, string loggedInUserId);
string getFileInfoIfExists(string groupId, string fileName);
vector<string> getUserMetaData(vector<string> peersList);
void removeIPandPort(string loggedInUserId);

// trackerHelper.h -> small function for reading and init of data structures, parsing etc
vector<string> getTrackerInfo(string trackerFileName);
vector<string> stringToVector(string str, char delimiter);
string vectorToString(vector<string> vect, string delimiter);
void initClientInfoFromFile();
void flushClientInfoMapTofile();
void initGroupInfoFromFile();
void flushGroupInfoMapTofile();
void flushFileInfoMapTofile();

// trackerDataStructure.h -> Stores the Data Structures for ClientInfo, GroupInfo and FileInfo
class ClientInfo;
class GroupInfo;
class FileInfo;

// ClientInfo Class and Map Data Structures
unordered_map<string, ClientInfo *> clientInfoMap;
int clientInfoMapReaderCount = 0;
sem_t clientInfoMapReadLock;
sem_t clientInfoMapWriteLock;

// GroupInfo Class and Map Data Structures (key: groupId)
unordered_map<string, GroupInfo *> groupInfoMap;
int groupInfoMapReaderCount = 0;
sem_t groupInfoMapReadLock;
sem_t groupInfoMapWriteLock;

// FileInfo Class and Map Data Structures (key: groupId|fileName)
unordered_map<string, FileInfo *> fileInfoMap;
int fileInfoMapReaderCount = 0;
sem_t fileInfoMapReadLock;
sem_t fileInfoMapWriteLock;

int liveTrackerThreads = 0;
pthread_t trackerThreadIds[TRACKERTHREADIDSCOUNT];

int liveClientThreads = 0;
pthread_t liveClientThreadIds[LIVECLIENTTHREADIDS];

string trackerFileName;

// Tracker Socket Parameters (IP, PORT on which Tracker's Server will be listening to client's request)
string TrackerIP;
int TrackerPORT;
int tracker_sd;

struct sockaddr_in trackerAddress;
socklen_t tracker_address_length;
int opt = 1;

// Used to store the log for each command and print it to tracker using Logger
string trackerLog;

// Used to send the log of the response to the client after each command executes
string responseToClient;
