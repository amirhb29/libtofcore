#include "tofcore/tof_sensor.hpp"

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <csignal>
#include <atomic>
#include <ctime> // for std::time_t, std::localtime, std::strftime
#include <chrono>
#include <thread>
#include <boost/program_options.hpp>

using namespace std::chrono_literals;
using namespace std::chrono;
using namespace tofcore;
namespace po = boost::program_options;

static std::ofstream outfile;

const uint16_t DEFAULT_INTEGRATION_TIME = 500; // Default integration time

static uint16_t integrationTime{DEFAULT_INTEGRATION_TIME};
static uint32_t baudRate{DEFAULT_BAUD_RATE};
static std::string devicePort{"/dev/ttyACM0"};
static uint16_t minAmplitude{0};
static bool enableBinning{false};
static uint16_t modulation{0};

std::atomic<bool> exitRequested{false}; // Atomic flag to indicate interrupt

static void signalHandler(int signum)
{
    (void)signum;
    exitRequested.store(true);
}

static void measurement_callback(std::shared_ptr<tofcore::Measurement_T> pData)
{
    using DataType = tofcore::Measurement_T::DataType;

    if (pData->type() == DataType::DISTANCE_AMPLITUDE)
    {
        auto ts = std::chrono::system_clock::now();
        long timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(ts.time_since_epoch()).count();
        outfile << std::fixed << std::setprecision(3) << timestamp_ms;

        size_t n = pData->distance().size();
        auto dist = pData->distance().data();
        auto amp = pData->amplitude().data();

        for (size_t i = 0; i < n; ++i)
        {
            outfile << ',' << std::dec << dist[i] << "," << amp[i];
        }
        outfile << "\n";
        outfile.flush();
    }
}

static void parseArgs(int argc, char *argv[])
{
    po::options_description desc("illuminator board test");
    desc.add_options()("help,h", "produce help message")("device-uri,p", po::value<std::string>(&devicePort))("min-amplitude,m", po::value<uint16_t>(&minAmplitude)->default_value(0))("baud-rate,b", po::value<uint32_t>(&baudRate)->default_value(DEFAULT_BAUD_RATE))("integration,i", po::value<uint16_t>(&integrationTime)->default_value(DEFAULT_INTEGRATION_TIME), "Set integration time to this value (uS)")("Binning,B", po::bool_switch(&enableBinning)->default_value(false), "Enable full binning")("modulation,m", po::value<uint16_t>(&modulation)->default_value(0), "Set modulation frequency to this value (kHz)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help"))
    {
        std::cout << desc << "\n";
        exit(0);
    }
}

int main(int argc, char *argv[])
{

    try
    {
        parseArgs(argc, argv);
    }
    catch (po::error &x)
    {
        std::cerr << x.what() << "\n";
        return EXIT_FAILURE;
    }
    /*
     * Change default action of ^C, ^\ from abnormal termination in order to
     * perform a controlled shutdown.
     */
    signal(SIGINT, signalHandler);
#if defined(SIGQUIT)
    signal(SIGQUIT, signalHandler);
#endif

    {
        tofcore::Sensor sensor{devicePort, baudRate};
        // std::vector<double> rays_x, rays_y, rays_z;
        // try
        // {
        //     sensor.getLensInfo(rays_x, rays_y, rays_z);
        //     // cartesianTransform_.initLensTransform(320, 240, rays_x, rays_y, rays_z);
        // }
        // catch (std::exception &e)
        // {
        //     std::cout << "Error retreiving lens info..." << std::endl;
        //     return -1;
        // }
        sensor.stopStream();
        std::this_thread::sleep_for(std::chrono::seconds(2));

        if (enableBinning)
        {
            sensor.setBinning(true);
        }
        else
        {
            sensor.setBinning(false);
        }

        if (modulation)
        {
            sensor.setModulation(modulation);
        }

        if (integrationTime)
        {
            sensor.setIntegrationTime(integrationTime);
        }

        // Open a file to save all data
        // Get current time
        std::time_t now = std::time(nullptr);
        std::tm *ptm = std::localtime(&now);

        // Format: "cloud_data_YYYY-MM-DD_HH-MM.csv"
        char filename[64];
        std::strftime(filename, sizeof(filename), "dist_amp_mojave_%Y-%m-%d_%H-%M.csv", ptm);

        // We'll use CSV format: timestamp, distance_i, amplitude_i, distance_j, amplitude_j, ...
        outfile.open(filename);
        if (!outfile.is_open())
        {
            std::cerr << "Failed to open file to save data." << std::endl;
            return EXIT_FAILURE;
        }

        // Write CSV header
        // outfile << "timestamp,dist_x,dist_y,amp_x,amp_y,x,y,z" << std::endl;

        sensor.subscribeMeasurement(&measurement_callback); // callback is called from background thread
        sensor.streamDistanceAmplitude();

        while (!exitRequested.load())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        std::cout << "Keyboard interrupt received. Shutting down..." << std::endl;
        sensor.stopStream();

        // Close the outfile after breaking from the loop
        outfile.close();
        std::this_thread::sleep_for(std::chrono::seconds(3));
    } // when scope is exited, sensor connection is cleaned up

    return EXIT_SUCCESS;
}
