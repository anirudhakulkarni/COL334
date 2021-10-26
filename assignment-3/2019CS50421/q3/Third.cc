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
int total_packet_loss = 0;
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
    total_packet_loss++;
    file->Write(Simulator::Now(), p);
}

void solve_a(std::string conf_num)
{

    NodeContainer n13;
    n13.Create(2);
    PointToPointHelper pointToPoint13;
    pointToPoint13.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
    pointToPoint13.SetChannelAttribute("Delay", StringValue("3ms"));
    NetDeviceContainer n13devices;
    n13devices = pointToPoint13.Install(n13);

    NodeContainer n23;
    n23.Create(1);
    n23.Add(n13.Get(1));
    PointToPointHelper pointToPoint23;
    pointToPoint23.SetDeviceAttribute("DataRate", StringValue("9Mbps"));
    pointToPoint23.SetChannelAttribute("Delay", StringValue("3ms"));
    NetDeviceContainer n23devices;
    n23devices = pointToPoint23.Install(n23);
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
    em->SetAttribute("ErrorRate", DoubleValue(0.00001));
    n13devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
    n23devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

    InternetStackHelper stack;
    stack.Install(n13);
    stack.Install(n23.Get(0));
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4AddressHelper address2;
    address2.SetBase("10.1.2.0", "255.255.255.0");
    NS_LOG_UNCOND("Assigning address \n");
    Ipv4InterfaceContainer n13interfaces = address.Assign(n13devices);
    Ipv4InterfaceContainer n23interfaces = address2.Assign(n23devices);
    NS_LOG_UNCOND("Assigned address =" << n13interfaces.GetAddress(0) << "\n");
    NS_LOG_UNCOND("Assigned address =" << n13interfaces.GetAddress(1) << "\n");
    NS_LOG_UNCOND("Assigned address =" << n23interfaces.GetAddress(0) << "\n");
    NS_LOG_UNCOND("Assigned address =" << n23interfaces.GetAddress(1) << "\n");
    // Ipv4InterfaceContainer n23interfaces = address.Assign(n23devices.Get(0));
    // NS_LOG_UNCOND("Assigned address =" << n23interfaces.GetAddress(0) << "\n");
    // NS_LOG_UNCOND("Assigned address =" << n23interfaces.GetAddress(1) << "\n");

    uint16_t sinkPort = 8080;
    Address sinkAddress1(InetSocketAddress(n13interfaces.GetAddress(1), sinkPort));
    Address sinkAddress2(InetSocketAddress(n23interfaces.GetAddress(1), sinkPort));
    PacketSinkHelper sinkHelper("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), sinkPort));
    ApplicationContainer sinkApp = sinkHelper.Install(n13.Get(1));
    sinkApp.Start(Seconds(0.0));
    sinkApp.Stop(Seconds(30.0));
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpNewReno::GetTypeId()));

    if (conf_num == "3")
    {
        TypeId tid = TypeId::LookupByName("ns3::TcpNewRenoCSE");
        std::stringstream nodeId;
        nodeId << n13.Get(0)->GetId();
        std::string specificNode = "/NodeList/" + nodeId.str() + "/$ns3::TcpL4Protocol/SocketType";
        Config::Set(specificNode, TypeIdValue(tid));
        nodeId << n13.Get(1)->GetId();
        specificNode = "/NodeList/" + nodeId.str() + "/$ns3::TcpL4Protocol/SocketType";
        Config::Set(specificNode, TypeIdValue(tid));
        nodeId << n23.Get(0)->GetId();
        specificNode = "/NodeList/" + nodeId.str() + "/$ns3::TcpL4Protocol/SocketType";
        Config::Set(specificNode, TypeIdValue(tid));
    }
    else if (conf_num == "2")
    {
        TypeId tid = TypeId::LookupByName("ns3::TcpNewRenoCSE");
        std::stringstream nodeId;
        nodeId << n23.Get(0)->GetId();
        std::string specificNode = "/NodeList/" + nodeId.str() + "/$ns3::TcpL4Protocol/SocketType";
        Config::Set(specificNode, TypeIdValue(tid));
    }

    // source preparation
    Ptr<Socket> ns3TcpSocket1 = Socket::CreateSocket(n13.Get(0), TcpSocketFactory::GetTypeId());
    Ptr<Socket> ns3TcpSocket2 = Socket::CreateSocket(n13.Get(0), TcpSocketFactory::GetTypeId());
    Ptr<Socket> ns3TcpSocket3 = Socket::CreateSocket(n23.Get(0), TcpSocketFactory::GetTypeId());
    Ptr<MyApp> app1 = CreateObject<MyApp>();
    Ptr<MyApp> app2 = CreateObject<MyApp>();
    Ptr<MyApp> app3 = CreateObject<MyApp>();

    // source sink connection
    app1->Setup(ns3TcpSocket1, sinkAddress1, 3000, 10000, DataRate("1.5Mbps"));
    app2->Setup(ns3TcpSocket2, sinkAddress1, 3000, 10000, DataRate("1.5Mbps"));
    app3->Setup(ns3TcpSocket3, sinkAddress2, 3000, 10000, DataRate("1.5Mbps"));
    n13.Get(0)->AddApplication(app1);
    n13.Get(0)->AddApplication(app2);
    n23.Get(0)->AddApplication(app3);

    app1->SetStartTime(Seconds(1.0));
    app1->SetStopTime(Seconds(20.0));
    app3->SetStartTime(Seconds(15.0));
    app3->SetStopTime(Seconds(30.0));
    app2->SetStartTime(Seconds(5.0));
    app2->SetStopTime(Seconds(25.0));

    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream1 = asciiTraceHelper.CreateFileStream("q3/q3-1-conf" + conf_num + ".cwnd");
    Ptr<OutputStreamWrapper> stream2 = asciiTraceHelper.CreateFileStream("q3/q3-2-conf" + conf_num + ".cwnd");
    Ptr<OutputStreamWrapper> stream3 = asciiTraceHelper.CreateFileStream("q3/q3-3-conf" + conf_num + ".cwnd");
    PcapHelper pcapHelper;
    Ptr<PcapFileWrapper> file1 = pcapHelper.CreateFile("q3/q3-13-conf" + conf_num + ".pcap", std::ios::out, PcapHelper::DLT_PPP);
    Ptr<PcapFileWrapper> file2 = pcapHelper.CreateFile("q3/q3-23-conf" + conf_num + ".pcap", std::ios::out, PcapHelper::DLT_PPP);

    ns3TcpSocket1->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback(&CwndChange, stream1));
    ns3TcpSocket2->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback(&CwndChange, stream2));
    ns3TcpSocket3->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback(&CwndChange, stream3));
    // ns3TcpSocket1->TraceConnectWithoutContext("Drop", MakeBoundCallback(&RxDrop, file1));
    // ns3TcpSocket2->TraceConnectWithoutContext("Drop", MakeBoundCallback(&RxDrop, file2));
    // ns3TcpSocket3->TraceConnectWithoutContext("Drop", MakeBoundCallback(&RxDrop, file3));
    n13devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeBoundCallback(&RxDrop, file1));
    n23devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeBoundCallback(&RxDrop, file2));

    Simulator::Stop(Seconds(50));
    Simulator::Run();
    NS_LOG_UNCOND("Assigned address =" << n13interfaces.GetAddress(0) << "\n");
    NS_LOG_UNCOND("Assigned address =" << n13interfaces.GetAddress(1) << "\n");
    NS_LOG_UNCOND("Assigned address =" << n23interfaces.GetAddress(0) << "\n");
    NS_LOG_UNCOND("Assigned address =" << n23interfaces.GetAddress(1) << "\n");

    Simulator::Destroy();
    NS_LOG_UNCOND("Total Packet Loss:");
    NS_LOG_UNCOND(total_packet_loss);
}

int main(int argc, char *argv[])
{
    CommandLine cmd;
    std::string conf_num;
    cmd.AddValue("conf_num", "Type of TCP congestion control", conf_num);

    cmd.Parse(argc, argv);
    Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpNewReno::GetTypeId()));
    solve_a(conf_num);
}
