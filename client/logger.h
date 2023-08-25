#pragma once

#include <stdio.h>
#include <ctime>
#include <mutex>

class Logger
{
public:
    enum LogPriority
    {
        DebugPriority,
        InfoPriority,
        SuccessPriority,
        WarnPriority,
        ErrorPriority,
    };

private:
    LogPriority priority = DebugPriority;
    mutex log_mutex;
    const char *filepath = 0;
    FILE *file = 0;

public:
    static void SetPriority(LogPriority new_priority)
    {
        get_instance().priority = new_priority;
    }

    static void EnableFileOutput()
    {
        Logger &logger_instance = get_instance();
        logger_instance.filepath = "log.txt";
        logger_instance.enable_file_output();
    }

    template <typename... Args>
    static void DEBUG(const char *message, Args... args)
    {
        get_instance().log("[DEBUG]\t", DebugPriority, message, args...);
    }

    template <typename... Args>
    static void INFO(const char *message, Args... args)
    {
        get_instance().log("[INFO]\t", InfoPriority, message, args...);
    }

    template <typename... Args>
    static void SUCCESS(const char *message, Args... args)
    {
        get_instance().log("[SUCCESS]\t", SuccessPriority, message, args...);
    }

    template <typename... Args>
    static void WARN(const char *message, Args... args)
    {
        get_instance().log("[WARN]\t", WarnPriority, message, args...);
    }

    template <typename... Args>
    static void ERROR(const char *message, Args... args)
    {
        get_instance().log("[ERROR]\t", ErrorPriority, message, args...);
    }

private:
    Logger() {}

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    ~Logger()
    {
        free_file();
    }

    static Logger &get_instance()
    {
        static Logger logger;
        return logger;
    }

    template <typename... Args>
    void log(const char *message_priority_str, LogPriority message_priority, const char *message, Args... args)
    {
        // Logger::EnableFileOutput();
        if (priority <= message_priority)
        {
            time_t current_time = time(0);
            tm *timestamp = localtime(&current_time);
            char buffer[80];
            strftime(buffer, 80, "%c", timestamp);

            std::scoped_lock lock(log_mutex);
            printf("%s  ", buffer);
            printf("%s", message_priority_str);
            printf(message, args...);
            printf("\n");

            if (file)
            {
                fprintf(file, "%s   ", buffer);
                fprintf(file, message_priority_str);
                fprintf(file, message, args...);
                fprintf(file, "\n");
            }
        }
        // Logger::free_file();
    }

    bool enable_file_output()
    {
        free_file();

        file = fopen(filepath, "a");

        if (file == 0)
        {
            return false;
        }

        return true;
    }

    void free_file()
    {
        if (file)
        {
            fclose(file);
            file = 0;
        }
    }
};