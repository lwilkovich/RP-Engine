#include "LoggingController.h"

#define _WHITE_FONT "\033[1;37m"
#define _RESET_FONT "\033[0m"

#define _TOGGLE_FLAG "Toggle-Logging-Flags"
#define _SCREEN "Screen"
#define _NETWORK "Network"
#define _FILE "File"

#define _THROUGHPUT_INTERVAL "Throughput-Interval"
#define _FILE_LOCATION "File-Location"
#define _CPU_USAGE "Cpu-Usage"
#define _AUTO_RESTART "Auto-Restart"

using namespace Engine;

LoggingController::LoggingController(Config *config) {
    try {
        screen.info = config->settings[TAG][_TOGGLE_FLAG][_SCREEN][_INFO];
        screen.debug = config->settings[TAG][_TOGGLE_FLAG][_SCREEN][_DEBUG];
        screen.warning = config->settings[TAG][_TOGGLE_FLAG][_SCREEN][_WARNING];
        screen.system = config->settings[TAG][_TOGGLE_FLAG][_SCREEN][_SYS];
        screen.error = config->settings[TAG][_TOGGLE_FLAG][_SCREEN][_ERROR];
        
        network.info = config->settings[TAG][_TOGGLE_FLAG][_NETWORK][_INFO];
        network.debug = config->settings[TAG][_TOGGLE_FLAG][_NETWORK][_DEBUG];
        network.warning = config->settings[TAG][_TOGGLE_FLAG][_NETWORK][_WARNING];
        network.system = config->settings[TAG][_TOGGLE_FLAG][_NETWORK][_SYS];
        network.error = config->settings[TAG][_TOGGLE_FLAG][_NETWORK][_ERROR];
        
        file.info = config->settings[TAG][_TOGGLE_FLAG][_FILE][_INFO];
        file.debug = config->settings[TAG][_TOGGLE_FLAG][_FILE][_DEBUG];
        file.warning = config->settings[TAG][_TOGGLE_FLAG][_FILE][_WARNING];
        file.system = config->settings[TAG][_TOGGLE_FLAG][_FILE][_SYS];
        file.error = config->settings[TAG][_TOGGLE_FLAG][_FILE][_ERROR];

        throughputInterval = config->settings[TAG][_THROUGHPUT_INTERVAL];
        fileLocation = config->settings[TAG][_FILE_LOCATION];
        cpuUsage = config->settings[TAG][_CPU_USAGE];
        autoRestartFlag = config->settings[TAG][_AUTO_RESTART];
    }
    catch (const json::exception& e) {
        std::cout << e.what() << std::endl;
    } 

    try {
        // _INFO(getClass(), getDesc(), "File Log Location: " + logFile);
        fout.open(fileLocation);
    }
    catch (...) {
        // _ERROR(getClass(), getDesc(), "Failed To Open Log File");
    }

    funcDict["Info"] = &LoggingController::logInfo;
    funcDict["System"] = &LoggingController::logSystem;
    funcDict["Debug"] = &LoggingController::logDebug;
    funcDict["Warning"] = &LoggingController::logWarning;
    funcDict["Error"] = &LoggingController::logError;
}

std::string LoggingController::buildLogRecord(std::array<std::string, 4> holder) {
    std::stringstream a_stream;
    a_stream << _WHITE_FONT;
    a_stream << _DateTime::grabDate();
    a_stream << " | ";
    a_stream << _DateTime::grabTime();
    a_stream << " | ";

    a_stream << std::setw(7) << std::left << holder[2];
    a_stream << " | ";
    a_stream << std::setw(27) << std::left << holder[0].substr(0,27);
    a_stream << " | ";
    if (holder[3]!= "") {
        a_stream << std::setw(15) << std::left << holder[3];
        a_stream << " | ";
    }
    a_stream << holder[1];
    a_stream << _RESET_FONT << std::endl;
    return a_stream.str();
}

json LoggingController::buildLogJson(std::array<std::string, 4> holder) { 
    json jsonObject;
    jsonObject["Packet-Type"] = "Log";
    jsonObject["Origin"] = "Engine";
    jsonObject["Destination"] = "Interface";
    
    //////////////////////////////
    
    json jsonNetwork;
    jsonNetwork["Gateway-Rcv-Stamp"] = "";
    jsonNetwork["Gateway-Snt-Stamp"] = "";
    jsonNetwork["Database-Stamp"] = "";

    //////////////////////////////

    json jsonPayload;
    jsonPayload["Date"] = _DateTime::grabDate();
    jsonPayload["Time"] = _DateTime::grabTime();
    jsonPayload["Type"] = holder[2];
    jsonPayload["Controller"] = holder[0];
    jsonPayload["Description"] = holder[3];
    jsonPayload["Message"] = holder[1];
    
    //////////////////////////////
    jsonObject["Network"] = jsonNetwork;
    jsonObject["Payload"] = jsonPayload;
    return jsonObject;
}

void LoggingController::cutBatchEntry(LogThroughput *batchEntry, float elapsedTime) {
    SYSTEM(getTag(), getDesc(),
        stringbuilder() << "Total Log Count: " << batchEntry->getTotalCount());
    SYSTEM(getTag(), getDesc(),
        stringbuilder() << "Sys Log Count: " << batchEntry->getSystemCount());
    SYSTEM(getTag(), getDesc(),
        stringbuilder() << "Info Log Count: " << batchEntry->getInfoCount());
    SYSTEM(getTag(), getDesc(),
        stringbuilder() << "Debug Log Count: " << batchEntry->getDebugCount());
    SYSTEM(getTag(), getDesc(),
        stringbuilder() << "Warning Log Count: " << batchEntry->getWarningCount());
    SYSTEM(getTag(), getDesc(),
        stringbuilder() << "Error Log Count: " << batchEntry->getErrorCount());
    SYSTEM(getTag(), getDesc(),
        stringbuilder() << "Log Throughput: ~" << (batchEntry->size() / elapsedTime) << " Bytes / Second");
    SYSTEM(getTag(), getDesc(),
        stringbuilder() << "Log Mem Size: " << (batchEntry->size()) << " Bytes");
}

void LoggingController::screenLog(std::string record) {
    std::cout << record;
}
void LoggingController::networkLog(std::string record) {
    record += '\r';
    // network.socketDesc[socketIndex].writeBuffer.push(record);
}
void LoggingController::fileLog(std::string record) {
    fout << record;
    fout.flush();
}

void LoggingController::logInfo(std::array<std::string, 4> holder, LogThroughput *batchEntry) {
    std::string record = buildLogRecord(holder);
    json logJson = buildLogJson(holder);
    batchEntry->pushInfoLog(record);
    if (screen.info) {
        screenLog(record);
    }
    if (network.info) {
        networkLog(logJson.dump(4));
    }
    if (file.info) {
        fileLog(record);
    }
}

void LoggingController::logSystem(std::array<std::string, 4> holder, LogThroughput *batchEntry) {
    std::string record = buildLogRecord(holder);
    json logJson = buildLogJson(holder);
    batchEntry->pushSysLog(record);
    if (screen.system) {
        screenLog(record);
    }
    if (network.system) {
        networkLog(logJson.dump(4));
    }
    if (file.system) {
        fileLog(record);
    }
}

void LoggingController::logDebug(std::array<std::string, 4> holder, LogThroughput *batchEntry) {
    std::string record = buildLogRecord(holder);
    json logJson = buildLogJson(holder);
    batchEntry->pushDebugLog(record);
    if (screen.debug) {
        screenLog(record);
    }
    if (network.debug) {
        networkLog(logJson.dump(4));
    }
    if (file.debug) {
        fileLog(record);
    }
}

void LoggingController::logWarning(std::array<std::string, 4> holder, LogThroughput *batchEntry) {
    std::string record = buildLogRecord(holder);
    json logJson = buildLogJson(holder);
    batchEntry->pushWarningLog(record);
    if (screen.warning) {
        screenLog(record);
    }
    if (network.warning) {
        networkLog(logJson.dump(4));
    }
    if (network.warning) {
        fileLog(record);
    }
}

void LoggingController::logError(std::array<std::string, 4> holder, LogThroughput *batchEntry) {
    std::string record = buildLogRecord(holder);
    json logJson = buildLogJson(holder);
    batchEntry->pushErrorLog(record);
    if (screen.error) {
        screenLog(record);
    }
    if (network.error) {
        networkLog(logJson.dump(4));
    }
    if (file.error) {
        fileLog(record);
    }
}

int LoggingController::startThread() {
    CpuLimiter limiter(cpuUsage);

    LogThroughput *batchEntry = new LogThroughput();
    while (1) {
        if (mLogger.empty() == false) {
            // std::cout << mLogger.empty() << std::endl;
            std::array<std::string, 4> holder = mLogger.pop();
            // std::string logRecord = buildLogRecord(holder);
            // json logJson = buildLogJson(holder);

            MFP func = funcDict[holder[2]];
            (this->*func)(holder, batchEntry);
        }
        auto end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsedSeconds = end - batchEntry->getQTime();
        float elapsedTime = 60.0;
        if (elapsedSeconds.count() > elapsedTime) {
            batchEntry->close(end);
            cutBatchEntry(batchEntry, elapsedTime);
            batchEntry = new LogThroughput();
        }

        limiter.CalculateAndSleep();
    }
    return 0;
}