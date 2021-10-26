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
int total_drops = 0;

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
    total_drops++;
    file->Write(Simulator::Now(), p);
}

int main(int argc, char *argv[])
{
    CommandLine cmd;
    std::string type;
    cmd.AddValue("type", "Type of TCP congestion control", type);
    cmd.Parse(argc, argv);
    // TcpNewReno, TcpHighSpeed, TcpVeno, TcpVegas
    if (type == "NewReno")
    {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpNewReno::GetTypeId()));
    }
    else if (type == "HighSpeed")
    {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpHighSpeed::GetTypeId()));
    }
    else if (type == "Veno")
    {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpVeno::GetTypeId()));
    }
    else if (type == "Vegas")
    {
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpVegas::GetTypeId()));
    }
    else
    {
        NS_LOG_UNCOND("Invalid type");
        return 0;
    }

    //  set qsize
    //  Config::SetDefault("ns3::PfifoFastQueueDisc::Limit", UintegerValue(queuesize));

    NodeContainer nodes;
    nodes.Create(2);

    PointToPointHelper pointToPoint;
    // parameter 2: data rate and propagation delay
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("8Mbps"));
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
    sinkApps.Stop(Seconds(30.));

    Ptr<Socket> ns3TcpSocket = Socket::CreateSocket(nodes.Get(0), TcpSocketFactory::GetTypeId());

    Ptr<MyApp> app = CreateObject<MyApp>();
    // parameter application datarate 1mbps, packet size 3000
    app->Setup(ns3TcpSocket, sinkAddress, 3000, 100000, DataRate("1Mbps"));
    nodes.Get(0)->AddApplication(app);
    app->SetStartTime(Seconds(1.));
    app->SetStopTime(Seconds(30.));

    AsciiTraceHelper asciiTraceHelper;
    Ptr<OutputStreamWrapper> stream;
    PcapHelper pcapHelper;
    Ptr<PcapFileWrapper> file;
    if (type == "NewReno")
    {
        stream = asciiTraceHelper.CreateFileStream("q1/q1-NewReno.cwnd");
        file = pcapHelper.CreateFile("q1/q1-NewReno.pcap", std::ios::out, PcapHelper::DLT_PPP);
    }
    else if (type == "HighSpeed")
    {
        stream = asciiTraceHelper.CreateFileStream("q1/q1-HighSpeed.cwnd");
        file = pcapHelper.CreateFile("q1/q1-HighSpeed.pcap", std::ios::out, PcapHelper::DLT_PPP);
    }
    else if (type == "Veno")
    {
        stream = asciiTraceHelper.CreateFileStream("q1/q1-Veno.cwnd");
        file = pcapHelper.CreateFile("q1/q1-Veno.pcap", std::ios::out, PcapHelper::DLT_PPP);
    }
    else if (type == "Vegas")
    {
        stream = asciiTraceHelper.CreateFileStream("q1/q1-Vegas.cwnd");
        file = pcapHelper.CreateFile("q1/q1-Vegas.pcap", std::ios::out, PcapHelper::DLT_PPP);
    }
    ns3TcpSocket->TraceConnectWithoutContext("CongestionWindow", MakeBoundCallback(&CwndChange, stream));
    devices.Get(1)->TraceConnectWithoutContext("PhyRxDrop", MakeBoundCallback(&RxDrop, file));

    Simulator::Stop(Seconds(50));
    Simulator::Run();
    Simulator::Destroy();
    NS_LOG_UNCOND("Total Packet Drops:");
    NS_LOG_UNCOND(total_drops);
    return 0;
}