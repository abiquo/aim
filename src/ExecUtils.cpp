#include <ExecUtils.h>
#include <Debug.h>
#include <sys/wait.h>
#include <cstdlib>

bool commandExist(string& command)
{
    return (executeCommand(command, true) != 127);
}

int executeCommand(string command, bool redirect)
{
    if (redirect)
    {
        command.append(" > /dev/null");
    }

    LOG("Executing '%s'", command.c_str());

    int status = system(command.c_str());

    return WEXITSTATUS(status);
}

std::string exec(std::string cmd)
{
    LOG("Executing: '%s'", cmd.c_str());
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        LOG("Error while executing command '%s'", cmd.c_str());
        return "";
    }

    char buffer[128];
    std::string result = "";
    while (!feof(pipe))
    {
        if (fgets(buffer, 128, pipe) != NULL)
        {
            result += buffer;
        }
    }
    pclose(pipe);

    LOG("Command result: '%s'", result.c_str());
    return result;
}


    
