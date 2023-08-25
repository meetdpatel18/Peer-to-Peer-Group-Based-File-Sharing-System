#include "trackerHeaders.h"
#include "trackerDataStructure.h"
#include "logger.h"
#include "trackerHelper.h"
#include "trackerSemaphore.h"
#include "trackerConnection.h"
#include "respondClientRequests.h"
#include "clientCommands.h"

int main(int argc, char **argv)
{
    Logger::EnableFileOutput();
    initSemaphores();
    initClientInfoFromFile();
    initGroupInfoFromFile();
    initFileInfoFromFile();

    trackerFileName = argv[1];

    vector<string> trackerInfo = getTrackerInfo(trackerFileName);
    TrackerIP = trackerInfo[0];
    TrackerPORT = atoi(trackerInfo[1].c_str());

    initTrackersServer();
    return 0;
}