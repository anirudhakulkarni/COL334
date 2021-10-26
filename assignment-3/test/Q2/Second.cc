/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SixthScriptExample");
// vector of total packet loss
std::vector<int> total_packet_losses_a;
std::vector<int> total_packet_losses_b;

int curr_packet_loss = 0;
// ===========================================================================
//
//         node 0                 node 1
//   +----------------+    +----------------+
//   |    ns-3 TCP    |    |    ns-3 TCP    |
//   +----------------+    +----------------+
//   |    10.1.1.1    |    |    10.1.1.2    |
//   +----------------+    +----------------+
//   | point-to-point |    | point-to-point |
//   +----------------+    +----------------+
//           |                     |
//           +---------------------+
//                5 Mbps, 2 ms
//
//
// We want to look at changes in the ns-3 TCP congestion window.  We need
// to crank up a flow and hook the CongestionWindow attribute on the socket
// of the sender.  Normally one would use an on-off application to generate a
// flow, but this has a couple of problems.  First, the socket of the on-off
// application is not created until Application Start time, so we wouldn't be
// able to hook the socket (now) at configuration time.  Second, even if we
// could arrange a call after start time, the socket is not public so we
// couldn't get at it.
//
// So, we can cook up a simple version of the on-off application that does what
// we want.  On the plus side we don't need all of the complexity of the on-off
// application.  On the minus side, we don't have a helper, so we have to get
// a little more involved in the details, but this is trivial.
//
// So first, we create a socket and do the trace connect on it; then we pass
// this socket into the constructor of our simple application which we then
// install in the source node.
// ===========================================================================
//
class MyApp : public Application
{
public:
    MyApp();
    virtual ~MyApp();

    /**
     * Register this type.
     * \return The TypeId.
     */
    static TypeId GetTypeId(void);
    void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void ScheduleTx(void);
    void SendPacket(void);

    Ptr<Socket> m_socket;
    Address m_peer;
    uint32_t m_packetSize;
    uint32_t m_nPackets;
    DataRate m_dataRate;
    EventId m_sendEvent;
    bool m_running;
    uint32_t m_packetsSent;
};

MyApp::MyApp()
    : m_socket(0),
      m_peer(),
      m_packetSize(0),
      m_nPackets(0),
      m_dataRate(0),
      m_sendEvent(),
      m_running(false),
      m_packetsSent(0)
{
}

MyApp::~MyApp()
{
    m_socket = 0;
}

/* static */
TypeId MyApp::GetTypeId(void)
{
    static TypeId tid = TypeId("MyApp")
                            .SetParent<Application>()
                            .SetGroupName("Tutorial")
                            .AddConstructor<MyApp>();
    return tid;
}

void MyApp::Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
    m_socket = socket;
    m_peer = address;
    m_packetSize = packetSize;
    m_nPackets = nPackets;
    m_dataRate = dataRate;
}

void MyApp::StartApplication(void)
{
    m_running = true;
    m_packetsSent = 0;
    m_socket->Bind();
    m_socket->Connect(m_peer);
    SendPacket();
}

void MyApp::StopApplication(void)
{
    m_running = false;

    if (m_sendEvent.IsRunning())
    {
        Simulator::Cancel(m_sendEvent);
    }

    if (m_socket)
    {
        m_socket->Close();
    }
}

void MyApp::SendPacket(void)
{
    Ptr<Packet> packet = Create<Packet>(m_packetSize);
    m_socket->Send(packet);

    if (++m_packetsSent < m_nPackets)
    {
        ScheduleTx();
    }
}

void MyApp::ScheduleTx(void)
{
    if (m_running)
    {
        Time tNext(Seconds(m_packetSize * 8 / static_cast<double>(m_dataRate.GetBitRate())));
        m_sendEvent = Simulator::Schedule(tNext, &MyApp::SendPacket, this);
    }
}

static void
CwndChange(Ptr<OutputStreamWrapper> stream, uint32_t oldCwnd, uint32_t newCwnd)
{
    NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "\t" << newCwnd);
    *stream->GetStream() << Simulator::Now().GetSeconds() << "\t" << oldCwnd << "\t" << newCwnd << std::endl;
}

static void
RxDrop(Ptr<PcapFileWrapper> file, Ptr<const Packet> p)
{
    NS_LOG_UNCOND("RxDrop at " << Simulator::Now().GetSeconds());
    curr_packet_loss++;
    file->Write(Simulator::Now(), p);
}

void solve_a(std::string channelrate)
{
    curr_packet_loss = 0;
    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    // parameter 2: channel data rate and propagation delay
    pointToPoint.SetDeviceAttribute("DataRate", StringValue(channelrate));
    pointToPoint.SetChannelAttribute("Delay", StringValue("3ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    // parameter 3: error rate 0.00001
    em->SetAttribute("ErrorRate", DoubleValue(0.00001));
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    InternetStackHelper stack;
    stack.Install(nodes);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    uint16_t sinkPort = 8080;
    Address sinkAddress(InetSocketAddress(interfaces.GetAddress(1), sinkPort));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    ApplicationContainer sinkApps = packetSinkHelper.Install(nodes.Get(1));
    sinkApps.Start(Seconds(1.));
    sinkApps.Stop(Seconds(50.));
    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());

    Ptr<MyApp> app = CreateObject<MyApp>();
    // parameter application datarate 1mbps, packet size 3000
    app->Setup(ns3TcpSocket, sinkAddress, 3000, 20000, DataRate("2Mbps"));
    nodes.Get(0)->AddApplication(app);
    app->SetStartTime(Seconds(1.));
    app->SetStopTime(Seconds(30.));

    AsciiTraceHelper asciiTraceHelper;
    std::string outname = "q2/q2-a-" + channelrate + ".cwnd";
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream(outname);
    PcapHelper pcapHelper;
    outname = "q2/q2-a-" + channelrate + ".pcap";
    Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(outname, std::ios::out, PcapHelper::DLT_PPP);
    ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback(&CwndChange, stream));
    devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeBoundCallback(&RxDrop, file));

    Simulator::Stop(Seconds(50));
    Simulator::Run();
    Simulator::Destroy();
    total_packet_losses_a.push_back(curr_packet_loss);
}

void solve_b(std::string apprate)
{
    curr_packet_loss = 0;
    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    // parameter 2: channel data rate and propagation delay
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("6Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("3ms"));

    NetDeviceContainer devices;
    devices = pointToPoint.Install(nodes);
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    // parameter 3: error rate 0.00001
    em->SetAttribute("ErrorRate", DoubleValue(0.00001));
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    InternetStackHelper stack;
    stack.Install(nodes);
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);
    uint16_t sinkPort = 8080;
    Address sinkAddress(InetSocketAddress(interfaces.GetAddress(1), sinkPort));
    PacketSinkHelper packetSinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    ApplicationContainer sinkApps = packetSinkHelper.Install(nodes.Get(1));
    sinkApps.Start(Seconds(1.));
    sinkApps.Stop(Seconds(50.));
    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());

    Ptr<MyApp> app = CreateObject<MyApp>();
    // parameter application datarate 1mbps, packet size 3000
    app->Setup(ns3TcpSocket, sinkAddress, 3000, 20000, DataRate(apprate));
    nodes.Get(0)->AddApplication(app);
    app->SetStartTime(Seconds(1.));
    app->SetStopTime(Seconds(30.));

    AsciiTraceHelper asciiTraceHelper;
    std::string outname = "q2/q2-b-" + apprate + ".cwnd";
    Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream(outname);
    PcapHelper pcapHelper;
    outname = "q2/q2-b-" + apprate + ".pcap";
    Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(outname, std::ios::out, PcapHelper::DLT_PPP);
    ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback(&CwndChange, stream));
    devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeBoundCallback(&RxDrop, file));

    Simulator::Stop(Seconds(50));
    Simulator::Run();
    Simulator::Destroy();
    total_packet_losses_b.push_back(curr_packet_loss);
}
int main(int argc, char *argv[])
{
    CommandLine cmd;

    std::string part, rate;
    cmd.AddValue("part", "Part a or b", part);
    cmd.AddValue("rate", "rate", rate);
    cmd.Parse(argc, argv);

    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpNewReno::GetTypeId()));
    // list of channel datarates 2Mbps, 4Mbps, 10 Mbps, 20Mbps, 50 Mbps)
    if (part == "a")
    {
        solve_a(rate);
        NS_LOG_UNCOND("Printing losses for a");
        for (auto p : total_packet_losses_a)
        {
            NS_LOG_UNCOND(p);
        }
    }
    else
    {
        solve_b(rate);
        NS_LOG_UNCOND("Printing losses for b");
        for (auto p : total_packet_losses_b)
        {
            NS_LOG_UNCOND(p);
        }
    }
    // std::vector<std::string> channelrates = {"2Mbps", "4Mbps", "10Mbps", "20Mbps", "50Mbps"};
    // for (auto rate : channelrates)
    // {
    //     solve_a(rate);
    // }
    // // list of application datarates (0.5 Mbps, 1Mbps, 2Mbps, 4Mbps, 10 Mbps)
    // std::vector<std::string> apprates = {"0.5Mbps", "1Mbps", "2Mbps", "4Mbps", "10Mbps"};
    // for (auto rate : apprates)
    // {
    //     solve_b(rate);
    // }
    return 0;
}
