#include <bits/stdc++.h>
#include <string>
#include <fstream>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <openssl/sha.h>

#define LL long long
#define DELIMITERCHAR '|'
#define DELIMITERSTR "|"
#define SPACESTR " "
#define TRUE "TRUE"
#define FALSE "FALSE"

#define MAXCLIENTREQUESTS 100000000
#define MAXPEERTHREADS 100000000
#define MAXBUFFERSIZE 1024
#define MAXDOWNLOADBUFFERSIZE 524288

using namespace std;

// Client's Server Variables
string clientsServerIP;
int clientsServerPORT;
int clientsServerSD;
struct sockaddr_in clientsServerAddress;
socklen_t client_address_length;
pthread_t clientsServerThreadId;

//
//

// socket descriptor of client which will be used to connect and send/receive data to/from tracker
int client_sd;

// Client's Global Variables
int peerThreads = 0;
pthread_t peerThreadIds[MAXPEERTHREADS];
bool isUserLoggedIn = false;
string loggedInUserId;

// Client's FileInfo storage Data Structure and semaphore locks
class FileInfo;
unordered_map<string, FileInfo *> fileInfoMap;
int fileInfoMapReaderCount = 0;
sem_t fileInfoMapReadLock;
sem_t fileInfoMapWriteLock;

// Tracker's IP and PORT
string TrackerIP;
int TrackerPORT;

// calcSHA1.h -> Contains function that return SHA1 of chunks as well as entire file
string calcCompleteFileSHA1(string fileName);
void bin2hex(unsigned char *src, int len, char *hex);
vector<string> hashOfChunks(string filePath);

// clientCommands.h -> Contains all the commands a user can type in terminal
void createUser(string commandInfo);
void loginUser(string commandInfo);
void createGroup(string commandInfo);
void joinGroup(string commandInfo);
void leaveGroup(string commandInfo);
void listPendingJoinRequests(string commandInfo);
void acceptGroupJoinRequests(string commandInfo);
void listGroups(string commandInfo);
void listFiles(string commandInfo);
void uploadFile(string commandInfo);
void downloadFile(string commandInfo);
void logoutUser(string commandInfo);
vector<string> getChunkWiseSHA(int client_sd, vector<string> response);
bool isCommandArgsValid(string command, int argsLength, int reqLen);

// stores the map of (userId, (IP,PORT))

unordered_map<string, pair<string, int>> clientInfoMap;

// clientDataStructure.h -> Contains FileInfo class to store the files info on client side
class FileInfo;

// clientHelper.h -> Contains small functions which will be used for connection with tracker
//  and also contains other parsing functions and logger for logging responses
void setTrackerIPandPORT(string trackerFileName);
void getClientsServerIPandPortFromArgs(char *argv);
int createSocket();
bool connectClientWithTracker();
int setupConnectionWithPeersServer(string PeerIP, int PeerPORT);
vector<string> getTrackerInfo(string trackerFileName);
vector<string> stringToVector(string str, char delimiter);
string vectorToString(vector<string> vect, char delimiter);
string boolVectToString(vector<bool> &chunkInfo);
vector<bool> stringToBool(string chunkInfo);
void logTrackerResponse(string responseStatus, int messageIndexInResponse);

// clientSemaphore.h -> Contains access to FileInfoMap using semaphores
void initSemaphores();
void initClientFileInfoObject(vector<string> response, string filePath);
void addFileMetaDataToMap(string fileName, string filePath, LL fileSize, string SHA1);
string getFilePathFromFileInfo(string fileName);
vector<bool> getChunkInfo(string fileName);
bool fileAlreadyShared(string fileName);
bool isShareable(string fileName);
string getChunksha(string fileName, int chunkNumber);
void addChunkToBitMap(string fileName, int chunkNumber);

// clientsServer.h -> Contains function for creating client's Server which will server its peer's download request
void createServerSocket();
void bindServerSocket();
void listenServer();
void accept_connectionServer();
void *createClientsServer(void *param);
void *respondPeerRequests(void *args);
void sendChunkToClient(int peer_sd, string fileName, int chunkNumber);

// downloadFile.h -> Contains function which downloads the file from other peers
bool downloadFileFromOtherPeer(string fileName, LL fileSize, string PeerIP, int PeerPORT);
vector<bool> getChunkInfoFromPeers(string fileName, string PeerIP, int PeerPORT);
unordered_map<string, pair<string, int>> getClientInfo(vector<string> &response);
struct ComparePair;
void setChunkToPeerMap(unordered_map<int, vector<string>> &chunkToPeer, vector<bool> chunkInfo, string userId);
void setChunkCountHeap(priority_queue<pair<int, int>, vector<pair<int, int>>, ComparePair> &chunkCnt, unordered_map<int, vector<string>> &chunkToPeer);
bool performDownloadTask(vector<string> &response, string fileName, LL fileSize, string filePath, string groupId, string userId);
void chunkDownloadFromPeer(vector<string> userList, unordered_map<string, pair<string, int>> &clientInfo, string fileName, int chunkNumber, string filePath, string groupId, string userId);
vector<string> getUsersListThatHasChunk(unordered_map<int, vector<string>> &chunkToPeer, int chunkNumber);
bool receiveChunkFromClientsServer(int my_sd, string fileName, LL chunkNumber, vector<string> &response, string groupId, string userId);
