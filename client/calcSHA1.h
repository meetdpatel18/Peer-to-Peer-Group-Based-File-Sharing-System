string calcCompleteFileSHA1(string fileName)
{
    unsigned char digest[SHA_DIGEST_LENGTH];
    char str[(SHA_DIGEST_LENGTH * 2) + 1];

    FILE *pf;
    unsigned char buf[MAXDOWNLOADBUFFERSIZE];
    SHA_CTX ctxt;

    pf = fopen(fileName.c_str(), "rb");

    if (!pf)
        return "";

    SHA1_Init(&ctxt);

    while (1)
    {
        size_t len;
        len = fread(buf, 1, MAXDOWNLOADBUFFERSIZE, pf);
        if (len <= 0)
            break;
        SHA1_Update(&ctxt, buf, len);
    }

    fclose(pf);

    SHA1_Final(digest, &ctxt);

    bin2hex(digest, sizeof(digest), str);
    string fileSHA1 = str;
    return fileSHA1;
}

void bin2hex(unsigned char *src, int len, char *hex)
{
    int i, j;
    for (i = 0, j = 0; i < len; i++, j += 2)
        sprintf(&hex[j], "%02x", src[i]);
}

string calHashofchunk(char *schunk, int length1, int shorthashflag)
{
    unsigned char hash[SHA_DIGEST_LENGTH];
    char buf[SHA_DIGEST_LENGTH * 2];
    SHA1((unsigned char *)schunk, length1, hash);

    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        sprintf((char *)&(buf[i * 2]), "%02x", hash[i]);

    string ans;
    if (shorthashflag == 1)
    {
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        {
            ans += buf[i];
        }
    }
    else
    {
        for (int i = 0; i < SHA_DIGEST_LENGTH * 2; i++)
        {
            ans += buf[i];
        }
    }
    return ans;
}

vector<string> hashOfChunks(string filePath)
{
    string fileHash;
    ifstream file1(filePath, ifstream::binary);
    vector<string> chunkedHash;

    if (!file1)
    {
        cout << "File Does Not Exist : " << filePath << endl;
        return chunkedHash;
    }

    struct stat fstatus;
    stat(filePath.c_str(), &fstatus);

    long int total_size = fstatus.st_size;
    long int chunk_size = MAXDOWNLOADBUFFERSIZE;

    int total_chunks = total_size / chunk_size;
    int last_chunk_size = total_size % chunk_size;

    if (last_chunk_size != 0)
    {
        ++total_chunks;
    }
    else
    {
        last_chunk_size = chunk_size;
    }

    for (int chunk = 0; chunk < total_chunks; ++chunk)
    {
        int cur_cnk_size;
        if (chunk == total_chunks - 1)
            cur_cnk_size = last_chunk_size;
        else
            cur_cnk_size = chunk_size;

        char *chunk_data = new char[cur_cnk_size];
        file1.read(chunk_data,
                   cur_cnk_size);

        string sh1out = calHashofchunk(chunk_data, cur_cnk_size, 1);
        chunkedHash.push_back(sh1out);
    }

    return chunkedHash;
}