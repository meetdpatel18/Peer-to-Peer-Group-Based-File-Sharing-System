void initSemaphores()
{
    sem_init(&fileInfoMapReadLock, 0, 1);
    sem_init(&fileInfoMapWriteLock, 0, 1);
}

void fileInfoReadLock()
{
    sem_wait(&fileInfoMapReadLock);
    fileInfoMapReaderCount++;

    if (fileInfoMapReaderCount == 1)
        sem_wait(&fileInfoMapWriteLock);

    sem_post(&fileInfoMapReadLock);
}

void fileInfoReadUnlock()
{
    sem_wait(&fileInfoMapReadLock);
    fileInfoMapReaderCount--;

    if (fileInfoMapReaderCount == 0)
    {
        sem_post(&fileInfoMapWriteLock);
    }
    sem_post(&fileInfoMapReadLock);
}

void initClientFileInfoObject(vector<string> response, string filePath, bool isShareable)
{
    string fileName = response[1];
    LL fileSize = atoll(response[2].c_str());
    string SHA1 = response[3];

    sem_wait(&fileInfoMapWriteLock);
    FileInfo *file = new FileInfo(fileName, filePath, fileSize, SHA1, isShareable);
    fileInfoMap[fileName] = file;
    sem_post(&fileInfoMapWriteLock);
}

void addFileMetaDataToMap(string fileName, string filePath, LL fileSize, string SHA1, bool isShareable)
{
    sem_wait(&fileInfoMapWriteLock);
    FileInfo *file = new FileInfo(fileName, filePath, fileSize, SHA1, isShareable);
    file->setChunksSHA();

    fileInfoMap[fileName] = file;
    sem_post(&fileInfoMapWriteLock);
}

string getFilePathFromFileInfo(string fileName)
{
    string filePath = "";
    fileInfoReadLock();
    // CRITICAL SECTION STARTS
    if (fileInfoMap.find(fileName) != fileInfoMap.end())
    {
        filePath = fileInfoMap[fileName]->getFilePath();
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS
    fileInfoReadUnlock();
    return filePath;
}

vector<bool> getChunkInfo(string fileName)
{
    vector<bool> chunkInfo;
    fileInfoReadLock();

    // CRITICAL SECTION STARTS
    if (fileInfoMap.find(fileName) != fileInfoMap.end())
    {
        chunkInfo = fileInfoMap[fileName]->getChunkMap();
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS

    fileInfoReadUnlock();
    return chunkInfo;
}

bool fileAlreadyShared(string fileName)
{
    bool isShared = false;
    fileInfoReadLock();

    // CRITICAL SECTION STARTS
    if (fileInfoMap.find(fileName) != fileInfoMap.end())
    {
        FileInfo *file = fileInfoMap[fileName];
        if (!file->getIsShareable())
        {
            file->setIsShareable(true);
            isShared = true;
        }
    }

    sleep(0.1);
    // CRITICAL SECTION ENDS
    fileInfoReadUnlock();
    return isShared;
}

bool isShareable(string fileName)
{
    bool canShare = false;
    fileInfoReadLock();

    // CRITICAL SECTION STARTS
    if (fileInfoMap.find(fileName) != fileInfoMap.end())
    {
        FileInfo *file = fileInfoMap[fileName];
        canShare = file->getIsShareable();
    }

    sleep(0.1);
    // CRITICAL SECTION ENDS

    fileInfoReadUnlock();
    return canShare;
}

string getChunksha(string fileName, int chunkNumber)
{
    string chunkSHA = "";
    fileInfoReadLock();

    // CRITICAL SECTION STARTS
    if (fileInfoMap.find(fileName) != fileInfoMap.end())
    {
        FileInfo *file = fileInfoMap[fileName];
        vector<string> chunkWiseSHA = file->getChunksSHA();
        chunkSHA = chunkWiseSHA[chunkNumber];
    }

    sleep(0.1);
    // CRITICAL SECTION ENDS

    fileInfoReadUnlock();
    return chunkSHA;
}

void addChunkToBitMap(string fileName, int chunkNumber)
{
    sem_wait(&fileInfoMapWriteLock);

    FileInfo *file = fileInfoMap[fileName];
    file->setChunkBitMapStatus(chunkNumber, true);
    file->setIsShareable(true);

    sem_post(&fileInfoMapWriteLock);
}

void printFileMetaData(string fileName)
{
    fileInfoReadLock();

    // CRITICAL SECTION STARTS
    if (fileInfoMap.find(fileName) != fileInfoMap.end())
    {
        FileInfo *file = fileInfoMap[fileName];
        cout << "Printing File Info local storage for client who uploaded file" << endl;
        cout << "FileName: " << file->getFileName() << endl;
        cout << "FilePath: " << file->getFilePath() << endl;
        cout << "SHA1: " << file->getSHA1() << endl;
        cout << "ChunkMap: " << endl;
        vector<bool> cmap = file->getChunkMap();
        for (auto it : cmap)
        {
            cout << it << endl;
        }
        cout << "isShareable: " << file->getIsShareable() << endl;

        vector<string> chunksha = file->getChunksSHA();
        for (auto it : chunksha)
        {
            cout << it << endl;
        }
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS
    fileInfoReadUnlock();
}

LL getFileSize(string fileName)
{
    LL fileSize = 0;
    fileInfoReadLock();

    // CRITICAL SECTION STARTS
    if (fileInfoMap.find(fileName) != fileInfoMap.end())
    {
        FileInfo *file = fileInfoMap[fileName];
        fileSize = file->getFileSize();
    }
    sleep(0.1);
    // CRITICAL SECTION ENDS

    fileInfoReadUnlock();
    return fileSize;
}

bool allChunksReceived(string fileName)
{
    bool allReceived = true;
    fileInfoReadLock();

    FileInfo *file = fileInfoMap[fileName];
    vector<bool> chunks = file->getChunkMap();
    for (auto it : chunks)
    {
        if (it == false)
        {
            allReceived = false;
            break;
        }
    }
    fileInfoReadUnlock();
    return allReceived;
}