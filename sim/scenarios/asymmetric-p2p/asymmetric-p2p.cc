#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/queue.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/queue-size.h"
#include "../helper/quic-network-simulator-helper.h"
#include "../helper/quic-point-to-point-helper.h"
#include "drop-rate-error-model.h"
#include "gilbert-elliott-drop-model.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

static Ptr<Queue<Packet>> returnQueue;
static Ptr<Queue<Packet>> forwardQueue;
static constexpr unsigned int QUEUE_LOG_INTERVAL_USEC = 200000;

void timer_handler(int signum){
    cout <<
        "# return\t" <<
        returnQueue->GetNPackets() << "pkgs \t" <<
        returnQueue->GetNBytes() << "bytes \t" <<
        returnQueue->GetTotalReceivedPackets() << " pkgs\t" <<
        returnQueue->GetTotalReceivedBytes() << " bytes\t" <<
        returnQueue->GetTotalDroppedPackets() << " pkgs\t" <<
        returnQueue->GetTotalDroppedBytes() << " bytes\t" <<
        returnQueue->GetTotalDroppedPacketsBeforeEnqueue() << " pkgs\t" <<
        returnQueue->GetTotalDroppedBytesBeforeEnqueue() << " bytes\t" <<
        returnQueue->GetTotalDroppedPacketsAfterDequeue() << " pkgs\t" <<
        returnQueue->GetTotalDroppedBytesAfterDequeue() << " bytes" <<
        endl;
    cout <<
        "# forward\t" <<
        forwardQueue->GetNPackets() << "pkgs \t" <<
        forwardQueue->GetNBytes() << "bytes \t" <<
        forwardQueue->GetTotalReceivedPackets() << " pkgs\t" <<
        forwardQueue->GetTotalReceivedBytes() << " bytes\t" <<
        forwardQueue->GetTotalDroppedPackets() << " pkgs\t" <<
        forwardQueue->GetTotalDroppedBytes() << " bytes\t" <<
        forwardQueue->GetTotalDroppedPacketsBeforeEnqueue() << " pkgs\t" <<
        forwardQueue->GetTotalDroppedBytesBeforeEnqueue() << " bytes\t" <<
        forwardQueue->GetTotalDroppedPacketsAfterDequeue() << " pkgs\t" <<
        forwardQueue->GetTotalDroppedBytesAfterDequeue() << " bytes" <<
        endl;
}

int main(int argc, char *argv[]) {

    string channelDelay;
    string forwardDataRate;
    string forwardQueueSize;
    string returnDataRate;
    string returnQueueSize;
    string goodBadProbToClient;
    string badGoodProbToClient;
    string goodBadProbToServer;
    string badGoodProbToServer;
    string dropRateToClient;
    string dropRateToServer;
    bool linkErrorModels = false;

    // for dropping packets:
    random_device rand_dev;
    mt19937 generator(rand_dev());  // Seed random number generator first

    CommandLine cmd;
    cmd.AddValue("delay", "delay of the channel in both directions (RTT = 2 * delay)", channelDelay);
    cmd.AddValue("forward-data-rate", "data rate of the forward link (right -> left)", forwardDataRate);
    cmd.AddValue("forward-queue", "queue size of the forward link (right -> left) (in packets)", forwardQueueSize);
    cmd.AddValue("return-data-rate", "data rate of the return link (left -> right)", returnDataRate);
    cmd.AddValue("return-queue", "queue size of the return link (left -> right) (in packets)", returnQueueSize);
    cmd.AddValue("good-bad-prob-to-client", "Gilber-Elliott Probability for Transition from Good to Bad (towards client)", goodBadProbToClient);
    cmd.AddValue("bad-good-prob-to-client", "Gilber-Elliott Probability for Transition from Bad to Good (towards client)", badGoodProbToClient);
    cmd.AddValue("good-bad-prob-to-server", "Gilber-Elliott Probability for Transition from Good to Bad (towards server)", goodBadProbToServer);
    cmd.AddValue("bad-good-prob-to-server", "Gilber-Elliott Probability for Transition from Bad to Good (towards server)", badGoodProbToServer);
    cmd.AddValue("link-error-models", "Use the same error model instance in both directions (-> produce errors during the same time)", linkErrorModels);
    cmd.AddValue("drop-rate-to-client", "random packet drop rate (towards client)", dropRateToClient);
    cmd.AddValue("drop-rate-to-server", "random packet drop rate (towards server)", dropRateToServer);
    cmd.Parse(argc, argv);

    NS_ABORT_MSG_IF(channelDelay.length() == 0, "Missing parameter: delay");
    NS_ABORT_MSG_IF(forwardDataRate.length() == 0, "Missing parameter: forward-data-rate");
    NS_ABORT_MSG_IF(forwardQueueSize.length() == 0, "Missing parameter: forward-queue");
    NS_ABORT_MSG_IF(returnDataRate.length() == 0, "Missing parameter: return-data-rate");
    NS_ABORT_MSG_IF(returnQueueSize.length() == 0, "Missing parameter: return-queue");

    QuicNetworkSimulatorHelper sim;

    QuicPointToPointHelper p2p;
    p2p.SetChannelAttribute("Delay", StringValue(channelDelay));

    NetDeviceContainer devices = p2p.Install(sim.GetLeftNode(), sim.GetRightNode());

    auto leftP2PNetDev = devices.Get(0)->GetObject<PointToPointNetDevice>();
    auto rightP2PNetDev = devices.Get(1)->GetObject<PointToPointNetDevice>();

    leftP2PNetDev->SetDataRate(DataRate(returnDataRate));
    rightP2PNetDev->SetDataRate(DataRate(forwardDataRate));

    returnQueue = leftP2PNetDev->GetQueue();
    forwardQueue = rightP2PNetDev->GetQueue();
    returnQueue->SetMaxSize(QueueSize(returnQueueSize + "p"));
    forwardQueue->SetMaxSize(QueueSize(forwardQueueSize + "p"));

    struct sigaction sa;
    struct itimerval timer;

    /* Install timer_handler as the signal handler for SIGVTALRM. */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &timer_handler;
    sigaction(SIGVTALRM, &sa, NULL);
    cout << "# queue\t" <<
        "size pkgs\tsize bytes\t" <<
        "rx pkgs\trx bytes\t" <<
        "dropped pkgs\tdropped bytes\t" <<
        "dr b enq pkgs\tdr b enq bytes\t" <<
        "dr a deq pkgs\tdr a deq pkgs bytes" <<
        endl;

    /* Configure the timer to expire after x usec... */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = QUEUE_LOG_INTERVAL_USEC;
    /* ... and every x usec after that. */
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = QUEUE_LOG_INTERVAL_USEC;
    /* Start a virtual timer. It counts down whenever this process is executing. */
    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    // Set client and server drop rates.
    if (linkErrorModels && (goodBadProbToClient.length() == 0 || badGoodProbToClient.length() == 0 || dropRateToClient.length() != 0 || goodBadProbToServer.length() != 0 || badGoodProbToServer.length() != 0 || dropRateToServer.length() != 0)) {
        NS_ABORT_MSG_IF(true, "--link-error-models must be used with --good-bad-prob-to-client and --bad-good-prob-to-client only.");
    } else if (linkErrorModels && goodBadProbToClient.length() != 0 && badGoodProbToClient.length() != 0) {
        // link error models
        auto probGB = stof(goodBadProbToClient);
        auto probBG = stof(badGoodProbToClient);
        cout << "Using Gilbert-Elliot-Loss-Model for both directions with P(b|g) = " << probGB << " \% and P(g|b) = " << probBG << " \%" << endl;
        Ptr<GilbertElliottDropModel> drops = CreateObject<GilbertElliottDropModel>();
        drops->SetProbabilities(probGB, probBG);
        devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(drops));
        devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(drops));
    } else {

        if (goodBadProbToClient.length() > 0 && badGoodProbToClient.length() > 0 && dropRateToClient.length() == 0) {
            auto probGB = stof(goodBadProbToClient);
            auto probBG = stof(badGoodProbToClient);
            cout << "Using Gilbert-Elliot-Loss-Model for forward path with P(b|g) = " << probGB << " \% and P(g|b) = " << probBG << " \%" << endl;
            Ptr<GilbertElliottDropModel> dropsToClient = CreateObject<GilbertElliottDropModel>();
            dropsToClient->SetProbabilities(probGB, probBG);
            devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(dropsToClient));
        } else if (goodBadProbToClient.length() == 0 && badGoodProbToClient.length() == 0 && dropRateToClient.length() > 0) {
            cout << "Using uniform random loss model in forward path with P('err') = " << dropRateToClient << " \%" << endl;
            Ptr<DropRateErrorModel> dropsToClient = CreateObject<DropRateErrorModel>();
            dropsToClient->SetDropRate(stoi(dropRateToClient));
            devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(dropsToClient));
        } else if (goodBadProbToClient.length() == 0 && badGoodProbToClient.length() == 0 && dropRateToClient.length() == 0) {
            cout << "Using drop rate to client: 0 %. (Use --drop-rate-to-client or --good-bad-prob-to-client and --bad-good-prob-to-client to change)" << endl;
        } else {
            cerr << "Cannot mix random loss model (--drop-rate-to-client) and Gilbert-Elliott-Loss-Model (--good-bad-prob-to-client and --bad-good-prob-to-client)" << endl;
            NS_ABORT_MSG_IF(true, "Fix arguments!");
        }

        if (goodBadProbToServer.length() > 0 && badGoodProbToServer.length() > 0 && dropRateToServer.length() == 0) {
            auto probGB = stof(goodBadProbToServer);
            auto probBG = stof(badGoodProbToServer);
            cout << "Using Gilbert-Elliot-Loss-Model for return path with P(b|g) = " << probGB << " \% and P(g|b) = " << probBG << " \%" << endl;
            Ptr<GilbertElliottDropModel> dropsToServer = CreateObject<GilbertElliottDropModel>();
            dropsToServer->SetProbabilities(probGB, probBG);
            devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(dropsToServer));
        } else if (goodBadProbToServer.length() == 0 && badGoodProbToServer.length() == 0 && dropRateToServer.length() > 0) {
            cout << "Using uniform random loss model in return path with P('err') = " << dropRateToServer << " \%" << endl;
            Ptr<DropRateErrorModel> dropsToServer = CreateObject<DropRateErrorModel>();
            dropsToServer->SetDropRate(stoi(dropRateToServer));
            devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(dropsToServer));
        } else if (goodBadProbToServer.length() == 0 && badGoodProbToServer.length() == 0 && dropRateToServer.length() == 0) {
            cout << "Using drop rate to server: 0 %. (Use --drop-rate-to-server or --good-bad-prob-to-server and --bad-good-prob-to-server to change)" << endl;
        } else {
            cerr << "Cannot mix random loss model (--drop-rate-to-server) and Gilbert-Elliott-Loss-Model (--good-bad-prob-to-server and --bad-good-prob-to-server)" << endl;
            NS_ABORT_MSG_IF(true, "Fix arguments!");
        }
    }

    sim.Run(Seconds(36000));
}
