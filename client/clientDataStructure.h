class FileInfo
{
private:
    string fileName, filePath, SHA1;
    LL fileSize;
    vector<bool> chunkMap;
    bool isShareable;
    vector<string> chunksSHA;

public:
    FileInfo(string fileName, string filePath, LL fileSize, string SHA1, bool isShareable)
    {
        this->fileName = fileName;
        this->filePath = filePath;
        this->fileSize = fileSize;
        this->SHA1 = SHA1;
        this->isShareable = isShareable;
        this->chunkMap = setChunkMap();
    }

    // SETTER METHODS
    vector<bool> setChunkMap()
    {
        LL totalChunks = ceil((float)fileSize / MAXDOWNLOADBUFFERSIZE);
        vector<bool> chunkMap(totalChunks);
        for (LL i = 0; i < totalChunks; i++)
        {
            chunkMap[i] = isShareable;
        }
        return chunkMap;
    }

    void setFileName(string fileName)
    {
        this->fileName = fileName;
    }
    void setFilePath(string filePath)
    {
        this->filePath = filePath;
    }
    void setSHA1(string SHA1)
    {
        this->SHA1 = SHA1;
    }
    void setFileSize(LL fileSize)
    {
        this->fileSize = fileSize;
    }

    void setChunksSHA()
    {
        LL totalChunks = ceil(float(fileSize) / MAXDOWNLOADBUFFERSIZE);
        vector<string> chunksHash = hashOfChunks(filePath);
        this->chunksSHA = chunksHash;
    }

    void setIsShareable(bool isShareable)
    {
        this->isShareable = isShareable;
    }

    void setChunkBitMapStatus(int chunkNumber, bool exists)
    {
        this->chunkMap[chunkNumber] = exists;
    }

    // GETTER METHODS
    string getFileName()
    {
        return this->fileName;
    }
    string getFilePath()
    {
        return this->filePath;
    }

    LL getFileSize()
    {
        return this->fileSize;
    }
    string getSHA1()
    {
        return this->SHA1;
    }

    vector<bool> getChunkMap()
    {
        return this->chunkMap;
    }

    bool getIsShareable()
    {
        return this->isShareable;
    }

    vector<string> getChunksSHA()
    {
        return this->chunksSHA;
    }
};