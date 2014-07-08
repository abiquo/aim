#include <Aim.h>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TServer.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/PosixThreadFactory.h>

#include <cstdlib>
#include <cctype>
#include <getopt.h>

#include <AimServer.h>
#include <Service.h>
#include <Macros.h>
#include <Debug.h>

#include <vector>
#include <signal.h>
#include <sys/stat.h>

#include <version.h>
#include <asciilogo.h>

#include <INIReader.h>

#define logservice(action, name, current, all) LOG("[%d/%d] %s %s service", current, all, action, name)

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using namespace std;

bool daemonizeServer = false;
int threads = DEFAULT_THREADS;

int main(int argc, char **argv)
{
    // Parse command line arguments
    const char* configFilename = parseArguments(argc, argv);
    if (emptyString(configFilename))
    {
        configFilename = DEFAULT_CONFIG;
    }

    // Print aim's logo
    PRINT_ASCII_LOGO(aim_version);

    // Load configuration file
    LOG("Reading configuration from '%s' file", configFilename);
    INIReader configuration(configFilename);
    if (configuration.ParseError() < 0)
    {
        LOG("Unable to load '%s'", configFilename);
        exit(EXIT_FAILURE);
    }

    // Daemonize
    if (daemonizeServer)
    {
        daemonize();
    }

    // Print configuration summary
    int serverPort = configuration.GetInteger("server", "port", 60606);

    // Aim server initialization
    LOG("Initializing AIM v%s", aim_version);
    shared_ptr<TProcessor> processor(new AimProcessor(aimHandler));
    shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

    // Aim services initialization and start
    vector<Service*> services = aimHandler->getServices();
    vector<Service*>::iterator it;
    int i;

    for (it = services.begin(), i = 1; it < services.end(); ++it, i++)
    {
        Service* service = *it;

        logservice("Initializing", service->getName(), i, (int)services.size());
        if (!service->initialize(configuration))
        {
            exit(EXIT_FAILURE);
        }
    }

    for (it = services.begin(), i = 1; it < services.end(); ++it, i++)
    {
        Service* service = *it;

        logservice("Starting", service->getName(), i, (int)services.size());
        if (!service->start())
        {
            exit(EXIT_FAILURE);
        }
    }

    // Using signals to deinitialize
    signal(SIGINT, deinitialize);
    signal(SIGTERM, deinitialize);

    // Main loop
    LOG("Aim listening at port %d using %d threads", serverPort, threads);
    shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(threads);
    shared_ptr<PosixThreadFactory> threadFactory = shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
    threadManager->threadFactory(threadFactory);
    threadManager->start();
    TNonblockingServer server(processor, protocolFactory, serverPort, threadManager);
    server.serve();
    
    exit(EXIT_FAILURE);
}

void deinitialize(int param)
{
    vector<Service*> services = aimHandler->getServices();
    vector<Service*>::iterator it;
    bool done = false;
    int i;

    for (it = services.begin(), i = 1; it < services.end(); ++it, i++)
    {
        Service* service = *it;

        logservice("Stopping", service->getName(), i, (int)services.size());
        done = service->stop();
        done = (done ? service->cleanup() : false);

        if (!done)
        {
            LOG("Unable to stop properly %s service", service->getName());
        }
    }

    LOG("Bye!");
    exit(EXIT_SUCCESS);
}

const char * parseArguments(int argc, char **argv)
{
    int next_opt;
    const char * filename = "\0";

    while ((next_opt = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1)
    {
        switch (next_opt)
        {
            case 'h':
                printUsage(argv[0]);
                exit(EXIT_SUCCESS);
                break;

            case 'c':
                filename = optarg;
                break;

            case 'd':
                daemonizeServer = true;
                break;

            case 'v':
                printf("AIM server version %s\n", aim_version);
                printf("  Git: %s\n  Build: %s\n  Platform: %s\n", git_revision, build_date, build_platform);
                exit(EXIT_SUCCESS);

            case 't':
                threads = atoi(optarg);
                if (threads <= 0) { threads = DEFAULT_THREADS; }
                break;

            default:
                printUsage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    return filename;
}

void printUsage(const char* program)
{
    printf("Usage: %s options\n", program);
    printf( "    -h --help                       Show this help\n"
            "    -c --config-file=<file>         Alternate configuration file\n"
            "    -d --daemon                     Run as daemon\n"
            "    -v --version                    Show AIM server version\n"
            "    -t --threads                    Maximum threads to handle requests\n" );
}

static void daemonize(void)
{
    pid_t pid, sid;

    // Already a daemon
    if ( getppid() == 1 )
    {
        return;
    }

    // Fork off the parent process
    pid = fork();
    if (pid < 0)
    {
        exit(EXIT_FAILURE);
    }
    
    if (pid > 0)
    {
        // Exit the parent process
        exit(EXIT_SUCCESS);
    }

    // At this point we are executing as the child process

    // Change the file mode mask
    umask(0);

    // Create a new SID for the child process
    sid = setsid();
    if (sid < 0)
    {
        exit(EXIT_FAILURE);
    }

    if ((chdir("/")) < 0)
    {
        exit(EXIT_FAILURE);
    }

    // Redirect standard files to /dev/null
    freopen( "/dev/null", "r", stdin);
    freopen( "/dev/null", "w", stdout);
    freopen( "/dev/null", "w", stderr);
}
