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
