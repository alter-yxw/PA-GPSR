#include <iostream>
#include <cmath>
#include "ns3/fanet-module.h"
#include "ns3/aodvKmeans-module.h"
#include "ns3/epidemic-module.h"
#include "ns3/biocluster-module.h"
#include "ns3/pagpsr-module.h"
#include "ns3/leach-module.h"
#include "ns3/kmeans-module.h"
#include "ns3/icra-module.h"
#include "ns3/olsr-module.h"
#include "ns3/aodv-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
// #include "ns3/udp-client-server-helper.h"
#include "ns3/udp-echo-client.h"
#include "ns3/udp-echo-server.h"
#include "ns3/udp-echo-helper.h"
// #include "ns3/flow-monitor-helper.h"
// #include "ns3/ipv4-flow-classifier.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/energy-module.h"
#include "ns3/wifi-module.h"
#include "ns3/wifi-mac-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/leach-helper.h"
#include "ns3/v4ping-helper.h"
#include "ns3/yans-wifi-helper.h"
// #include "ns3/netsimulyzer-module.h"
#include <iomanip>

using namespace ns3;

#define   ORIGINENERGY 100.0

// void CourseChanged(Ptr<netsimulyzer::XYSeries> posSeries,  Ptr<netsimulyzer::LogStream> eventLog,
//                    std::string context, Ptr<const MobilityModel> model)
// {
//   const auto position = model -> GetPosition();
//   uint16_t NodeId = model->GetObject<Node>()->GetId();
//   *eventLog << Simulator::Now().GetSeconds() <<"[Node "<< NodeId <<"] Course Changed Position: ["<< position.x<<", " << position.y << ", " << position.z << "]\n";
//   posSeries->Append(position.x, position.y, position.z);
// }

// void handler (int arg0)
// {
//   std::cout << "The simulation is now at: "<< arg0 <<" seconds~~~~~~~~~~~~~~" << std::endl;
// }



class SimulationTest 
{
public:

  struct TestFlow {  // 测试流信息
    uint16_t srcNode;
    uint16_t dstNode;
    uint16_t port;
    Ipv4Address srcAddr;
    Ipv4Address dstAddr;
  };

  struct TransmissionStats {
    uint64_t  totalTxPackets;
    uint64_t  totalRxPackets;
    uint64_t  totalRxBytes;
    uint32_t  totalHops;
    double    avgThroughput;
    double    avgHops;
    double    totalDuration;
    TransmissionStats() 
      : totalTxPackets(0), totalRxPackets(0), totalRxBytes(0), totalHops(0),
        avgThroughput(0.0), avgHops(0.0), totalDuration(0.0)
    {}
  };
  
  SimulationTest ();
  bool Configure (int argc, char **argv);
  void Run ();
  void Report (std::ostream & os);
  TransmissionStats CalculateTransmissionStats();


private:
  // parameters
  uint16_t     size;             /// Number of nodes
  uint32_t     seed;             /// random seed to keep the same
  std::string  algorithm;
  std::string  helloMethod;      /// Hello间隔优化方法
  double       MinSpeed;
  double       MaxSpeed;
  double       ReClusterInterval;
  double       totalTime;        /// Simulation time, seconds
  double       MaxRange;
  bool         pcap;             /// Write per-device PCAP traces if true
  bool         printRoutes;      /// Print routes if true
  bool         appOn;
  bool         heterogeneous;    /// Enable heterogeneous nodes
  std::string  channelModel;     /// Channel model: "simple", "realistic", "harsh"
  std::map<uint32_t, Ptr<ClusterNode>>  clusternodes;
  std::vector<TestFlow>         m_testFlows;      /// 只存储测试用的流
  NodeContainer                 nodes;
  NetDeviceContainer            devices;
  Ipv4InterfaceContainer        interfaces;
  FlowMonitorHelper             flowmon;
  ns3::Ptr<ns3::FlowMonitor>    monitor;  
  Ptr<PerformanceMetrics>       m_metrics;

  void CreateNodes ();
  void CreateDevices ();
  void InstallEnergyModel();
  void InstallInternetStack ();
  void InstallApplications ();
  void PrintTestFlowStats();  // 只打印测试流的统计

};




int main (int argc, char **argv)
{
  SimulationTest test;
  if (!test.Configure (argc, argv)) {  NS_FATAL_ERROR ("Configuration failed. Aborted."); }
  test.Run ();
  test.Report (std::cout);
  return 0;
}


SimulationTest::SimulationTest():
  size (50),
  seed(2025),
  algorithm("icra"), /* [biocluster]-[leach]-[kmeans]-[aodv]-[olsr]-[aodvKmeans]-[]*/
  helloMethod("fuzzy"),
  MinSpeed(20),
  MaxSpeed(60),
  ReClusterInterval/* for [leach]、[kmeans]*/ (5.0),
  totalTime (200),
  MaxRange(500),
  pcap (false), printRoutes(false), appOn(true),
  heterogeneous(false),
  channelModel("simple")  // "simple", "realistic", "harsh"
{
}


bool SimulationTest::Configure (int argc, char **argv)
{
  // Enable aodvKmeans logs by default. Comment this if too noisy
  // LogComponentEnable("BioclusterRoutingProtocol", LOG_LEVEL_DEBUG);  
  // CommandLine cmd (__FILE__);
  CommandLine cmd;
  cmd.AddValue ("pcap"       , "Write PCAP traces."    , pcap);
  cmd.AddValue ("printRoutes", "Print routing table."  , printRoutes);
  cmd.AddValue ("size"       , "Number of nodes."      , size);
  cmd.AddValue ("time"       , "Simulation time, s."   , totalTime);
  cmd.AddValue ("seed"       , "Set Rand Seed"         , seed);
  cmd.AddValue ("AppsOn"     , "Turn on application"   , appOn);
  cmd.AddValue ("algorithm"  , "Routing Algorithm"     , algorithm);
  cmd.AddValue ("MinSpeed"   , "Minimum spped"         , MinSpeed);
  cmd.AddValue ("MaxSpeed"   , "Maximum speed"         , MaxSpeed);
  cmd.AddValue ("ReInterval" , "Recluster Duration"    , ReClusterInterval);
  cmd.AddValue ("helloMethod", "Hello interval method [qlearning|fuzzy|fixed]", helloMethod);
  cmd.AddValue ("heterogeneous", "Enable heterogeneous nodes", heterogeneous);
  cmd.AddValue ("channelModel", "Channel model [simple|realistic|harsh]", channelModel);
  cmd.Parse (argc, argv);
  return true;
}

void SimulationTest::Run ()
{
  //  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  SeedManager::SetSeed (seed);  
  CreateNodes();
  CreateDevices();
  InstallEnergyModel();
  InstallInternetStack ();

  if(appOn)     { InstallApplications (); }

  m_metrics =  CreateObject<PerformanceMetrics>();
  m_metrics -> SetNodes(nodes);
  m_metrics -> SetFlowMonitor(monitor, DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier()));

  for (double t = 10.0; t < totalTime; t += 5.0) {  // 定期收集性能数据（每5秒收集一次）
    Simulator::Schedule(Seconds(t), &PerformanceMetrics::CollectLinkMetrics, m_metrics);  
    if(!m_metrics->IsClusteringAlgorithm(algorithm)) {continue;}
    Simulator::Schedule(Seconds(t), &PerformanceMetrics::CollectClusteringMetrics, m_metrics);  
  }
    
  std::cout<<"Simulaton Starts, TotalTime: " << totalTime << " s ...\n";
  // for (int i=1; i<=totalTime; i++)
  // {
  //   if (i % 10 == 0) // at every 10s
  //   Simulator::Schedule(Seconds(i), &handler, i);
  // }
  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();

  if (algorithm == "icra")
  { // 通过节点0的routing protocol保存（会自动finalize最后一个episode）
    Ptr<icra::RoutingProtocol> masterRouting = nodes.Get(0)->GetObject<icra::RoutingProtocol>();
    std::string filename = "qmodel_" + std::to_string(size) + "nodes.csv";
    masterRouting->SaveQLearningModel(filename);
  }
  
  Simulator::Destroy ();
}


void SimulationTest::Report (std::ostream & os)
{
}


void SimulationTest::CreateNodes ()
{
  std::cout << "Creating " << (unsigned)size << " nodes, "<<"Algorithm: "<<algorithm<<"\n";
  nodes.Create (size);
  // Name nodes
  for (uint16_t i = 0; i < size; i++)
  {
    std::ostringstream os;
    os << "node-" << i;
    Names::Add (os.str (), nodes.Get (i));
    Ptr<ns3::ClusterNode> clusternode = CreateObject<ns3::ClusterNode>();
    if(clusternode == nullptr){
      std::cout<<"Error Creating clusternode for node "<< i<< std::endl;
      continue;
    }
    clusternode->SetNodeId(i);
    clusternode->SetClusterId(0xFFFF);
    clusternode->SetNetworkScale(size);
    clusternode->BindNode(nodes.Get(i));
    clusternode->SetRole(Idle);
    nodes.Get(i)->AggregateObject(clusternode);
    clusternodes[i] = clusternode;
  }

  MobilityHelper mobility;
  std::ostringstream vel;
  vel << "ns3::UniformRandomVariable[Min=" << MinSpeed << "|Max=" << MaxSpeed << "]";
  mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
    "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=2000]"),
    "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=2000]"),
    "Z", StringValue ("ns3::UniformRandomVariable[Min=0|Max=500]"));
  mobility.SetMobilityModel ("ns3::GaussMarkovMobilityModel",
    /* Notice: Need to Keep the Box Range same with the define at the head of biocluster-election.cc */
    "Bounds", BoxValue (Box (0, 2000, 0, 2000, 0, 500)), 
    "TimeStep", TimeValue (Seconds (0.5)),
    "Alpha", DoubleValue (0.85),
    "MeanVelocity", StringValue (vel.str()),//"ns3::UniformRandomVariable[Min=20.0|Max=60.0]"), // 
    "MeanDirection", StringValue ("ns3::UniformRandomVariable[Min=0|Max=6.283185307]"),
    "MeanPitch", StringValue ("ns3::UniformRandomVariable[Min=0.05|Max=0.05]"),
    "NormalVelocity", StringValue ("ns3::NormalRandomVariable[Mean=0.8|Variance=0.2|Bound=0.4]"), // this is of normal distribution
    "NormalDirection", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.4|Bound=1.0]"),
    "NormalPitch", StringValue ("ns3::NormalRandomVariable[Mean=0.0|Variance=0.02|Bound=0.04]"));
  mobility.Install (nodes);
  // /* Visualizer Setting*/
  // AsciiTraceHelper ascii;
  // MobilityHelper::EnableAsciiAll(ascii.CreateFileStream("mobility-trace-myFanet.mob"));
  // auto orchest = CreateObject<netsimulyzer::Orchestrator>("FanetScenario.json");
  // netsimulyzer::NodeConfigurationHelper nodeHelper{orchest};
  // nodeHelper.Set("Model", netsimulyzer::models::QUADCOPTER_UAV_VALUE);
  // nodeHelper.Set("Scale", DoubleValue(1.5));
  // nodeHelper.Install (nodes);
  // /*Visualize the logic Link between two nodes*/
  // // netsimulyzer::LogicalLinkHelper LinkHelper{orchest};
  // // LogicLinkPointer = &LinkHelper;
  // // LogicLinkPointer->Set("Color", netsimulyzer::BLUE_VALUE);
  // /*position visual*/
  // Ptr<netsimulyzer::LogStream> eventLog = CreateObject<netsimulyzer::LogStream>(orchest);
  // eventLog->SetAttribute("Name", StringValue("Event Log"));
  // Ptr<netsimulyzer::XYSeries> poSeries = CreateObject<netsimulyzer::XYSeries>(orchest);
  // poSeries->SetAttribute("Name", StringValue("Node Postion"));
  // poSeries->SetAttribute("LabelMode", StringValue("Hidden"));
  // poSeries->SetAttribute("Color", netsimulyzer::BLUE_VALUE);
  // poSeries->GetXAxis()->SetAttribute("Name", StringValue("X position (m)"));
  // poSeries->GetYAxis()->SetAttribute("Name", StringValue("Y position (m)"));
  // poSeries->GetZAxis()->SetAttribute("Name", StringValue("Z position (m)"));
  // Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeBoundCallback(&CourseChanged,poSeries, eventLog));
}


void SimulationTest::CreateDevices ()
{
  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

  // 根据channelModel配置不同的信道模型
  if (channelModel == "simple") {
    // 简单模型：仅范围判定（当前默认配置）
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel", "MaxRange", DoubleValue(MaxRange));

  } else if (channelModel == "realistic") {
    // 真实模型：添加路径损耗 + 阴影衰落
    wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                                   "Exponent", DoubleValue(2.5),        // 路径损耗指数 (降低从3.0到2.5)
                                   "ReferenceDistance", DoubleValue(1.0),
                                   "ReferenceLoss", DoubleValue(40.0)); // @ 1m for 2.4GHz (降低从46.6777到40.0)
    wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
                                   "m0", DoubleValue(1.5),              // Nakagami m parameter (提高从1.0到1.5，减小衰落)
                                   "m1", DoubleValue(1.5),
                                   "m2", DoubleValue(1.5));
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                   "MaxRange", DoubleValue(MaxRange));  // 硬限制范围

  } else if (channelModel == "harsh") {
    // 恶劣环境：强衰落 + 阴影 + 范围限制
    wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel",
                                   "Exponent", DoubleValue(3.5),        // 更强的路径损耗
                                   "ReferenceDistance", DoubleValue(1.0),
                                   "ReferenceLoss", DoubleValue(46.6777));
    wifiChannel.AddPropagationLoss("ns3::NakagamiPropagationLossModel",
                                   "m0", DoubleValue(0.5),              // 更强的衰落（m越小衰落越强）
                                   "m1", DoubleValue(0.75),
                                   "m2", DoubleValue(1.0));
    wifiChannel.AddPropagationLoss("ns3::RangePropagationLossModel",
                                   "MaxRange", DoubleValue(MaxRange));  // 硬限制范围
  }

  WifiHelper wifi;
  wifi.SetStandard(WIFI_STANDARD_80211g);
  wifi.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode",StringValue("ErpOfdmRate6Mbps"));
  // wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));

  YansWifiPhyHelper wifiPhy;
  wifiPhy.SetChannel(wifiChannel.Create());
  wifiPhy.Set("TxPowerStart", DoubleValue(20.0));
  wifiPhy.Set("TxPowerEnd", DoubleValue(20.0));
  devices = wifi.Install (wifiPhy, wifiMac, nodes);

  // Verify all nodes have devices
  // std::cout << "Created " << devices.GetN() << " devices for " << nodes.GetN() << " nodes" << std::endl;
  // for (uint32_t i = 0; i < nodes.GetN(); i++) {
  //   std::cout << "Node " << i << " has " << nodes.Get(i)->GetNDevices() << " devices" << std::endl;
  //   if (nodes.Get(i)->GetNDevices() == 0) {
  //     NS_FATAL_ERROR("Node " << i << " has no devices after installation!");
  //   }
  // }

  if (pcap) {  wifiPhy.EnablePcapAll (std::string ("fanettest")); }

}


void SimulationTest::InstallEnergyModel()
{
  BasicEnergySourceHelper energyhelper;
  WifiRadioEnergyModelHelper RadioEnergyHelper;

  if (!heterogeneous) {
    // 同构节点配置（原有配置）
    energyhelper.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(ORIGINENERGY));
    EnergySourceContainer energySources = energyhelper.Install(nodes);

    RadioEnergyHelper.Set("TxCurrentA",    DoubleValue(0.174));
    RadioEnergyHelper.Set("RxCurrentA",    DoubleValue(0.098));
    RadioEnergyHelper.Set("IdleCurrentA",  DoubleValue(0.025));
    RadioEnergyHelper.Install(devices, energySources);

  } else {
    // 异构节点配置：创建3类节点
    // 类型1: 强节点（20%）- 高能量、高功率，适合做簇头
    // 类型2: 普通节点（60%）- 中等配置
    // 类型3: 弱节点（20%）- 低能量、低功率

    uint16_t numStrongNodes = size * 0.2;
    uint16_t numWeakNodes = size * 0.2;

    // 先为所有节点设置不同的能量和能耗参数
    EnergySourceContainer energySources;

    for (uint16_t i = 0; i < size; i++) {
      double initialEnergy;
      double txCurrent;
      double txPower;

      if (i < numStrongNodes) {
        // 强节点：3倍能量
        initialEnergy = ORIGINENERGY * 3.0;
        txCurrent = 0.174 * 1.2;
        txPower = 25.0;
      } else if (i >= size - numWeakNodes) {
        // 弱节点：0.5倍能量
        initialEnergy = ORIGINENERGY * 0.5;
        txCurrent = 0.174 * 0.8;
        txPower = 15.0;
      } else {
        // 普通节点：正常能量
        initialEnergy = ORIGINENERGY;
        txCurrent = 0.174;
        txPower = 20.0;
      }

      // 安装能量源
      BasicEnergySourceHelper energyHelper_i;
      energyHelper_i.Set("BasicEnergySourceInitialEnergyJ", DoubleValue(initialEnergy));
      NodeContainer singleNode(nodes.Get(i));
      EnergySourceContainer singleSource = energyHelper_i.Install(singleNode);
      auto cNode = clusternodes.at(i);
      NS_ASSERT(cNode!=nullptr);
      cNode->SetOriginEnergy(initialEnergy);
      cNode->updateResidualEnergy(initialEnergy);
      energySources.Add(singleSource.Get(0));

      // 设置WiFi能耗模型
      WifiRadioEnergyModelHelper radioHelper_i;
      radioHelper_i.Set("TxCurrentA",    DoubleValue(txCurrent));
      radioHelper_i.Set("RxCurrentA",    DoubleValue(0.098));
      radioHelper_i.Set("IdleCurrentA",  DoubleValue(0.025));

      NetDeviceContainer singleDevice(devices.Get(i));
      radioHelper_i.Install(singleDevice, singleSource);

      // 调整发射功率
      Ptr<WifiNetDevice> wifiDev = DynamicCast<WifiNetDevice>(devices.Get(i));
      if (wifiDev) {
        Ptr<YansWifiPhy> phy = DynamicCast<YansWifiPhy>(wifiDev->GetPhy());
        if (phy) {
          phy->SetTxPowerStart(txPower);
          phy->SetTxPowerEnd(txPower);
        }
      }
    }

    std::cout << "异构节点配置: 强节点=" << numStrongNodes
              << ", 普通节点=" << (size - numStrongNodes - numWeakNodes)
              << ", 弱节点=" << numWeakNodes << std::endl;
  }
}


void SimulationTest::InstallInternetStack ()
{
  InternetStackHelper stack;
  Ipv4ListRoutingHelper list;   
    
  if(algorithm == "icra") {
    icraHelper icra;
    icra.SetQLearningParameters(0.1, 0.5, 0.1);  // alpha, lambda, epsilon
    list.Add(icra,0);
    // 可选：设置LHTP阈值等ICRA特有参数
    // biocluster.Set("LHTPThreshold", DoubleValue(2.0));
    // std::cout << "Using ICRA clustering algorithm" << std::endl;
  } 
  if(algorithm == "biocluster") {
    bioclusterHelper biocluster;
    biocluster.SetHelloIntervalMethod(helloMethod);
    // biocluster.SetHelloIntervalMethod(biocluster::HelloIFUZZY_HELLO);
    // std::cout << "Using ABC (Artificial Bee Colony) clustering algorithm" << std::endl;
    list.Add(biocluster, 0);
  } 
  if( algorithm == "leach"){   
    LeachHelper leach;
    leach.EnableLeachMode(true);
    leach.SetClusterHeadPercentage(0.1);
    leach.SetRoundDuration(Seconds(ReClusterInterval));
    list.Add(leach, 0);
    // std::cout << "Using LEACH clustering algorithm with " << "p=" << 0.1 << ", round duration=" << ReClusterInterval<< "s" << std::endl;
  }
  if(algorithm == "kmeans"){
    KmeansHelper kmeans;
    list.Add(kmeans, 0);
  }
  if(algorithm == "aodvKmeans"){
    aodvKmeansHelper aodvKmeans;
    list.Add(aodvKmeans, 0);
  }
  if(algorithm == "aodv")   {
    AodvHelper aodv;
    // aodv.Set("HelloInterval",TimeValue(Seconds(0.25)));
    list.Add(aodv, 0);
  }
  if(algorithm == "olsr")   {
    OlsrHelper olsr;
    olsr.Set("HelloInterval",TimeValue(Seconds(1.0)));
    list.Add(olsr, 0);
  }
  if(algorithm == "pagpsr") {  
    PAGpsrHelper pagpsr; 
    list.Add(pagpsr, 0);
  }
  if(algorithm == "dsdv")   {
    DsdvHelper dsdv;
    list.Add(dsdv, 0);
  }
  if(algorithm == "dsr")    {
    DsrHelper dsr;
    DsrMainHelper dsrMain;
    stack.Install (nodes);
    dsrMain.Install(dsr, nodes);    
    Ipv4AddressHelper address;
    address.SetBase ("10.0.0.0","255.0.0.0"); // maxDevices: 1022 ,broadcast addr: 192.168.3.255 
    interfaces = address.Assign (devices);
    return;
  }
  if(algorithm == "epidemic") {
    EpidemicHelper epidemic;
    
    epidemic.Set ("HopCount", UintegerValue (10));
    epidemic.Set ("QueueLength", UintegerValue (64));
    epidemic.Set ("QueueEntryExpireTime",TimeValue (Seconds(20)));
    epidemic.Set ("BeaconInterval", TimeValue (Seconds(1.0)));   
    list.Add(epidemic, 0); 
  }

  stack.SetRoutingHelper(list);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0","255.0.0.0"); // maxDevices: 1022 ,broadcast addr: 192.168.3.255
  stack.Install (nodes);
  interfaces = address.Assign (devices);

  // Verify all nodes still have devices after Internet stack installation
  // std::cout << "After InstallInternetStack:" << std::endl;
  // for (uint32_t i = 0; i < nodes.GetN(); i++) {
  //   uint32_t nDevices = nodes.Get(i)->GetNDevices();
  //   if (nDevices == 0) {
  //     std::cout << "WARNING: Node " << i << " has 0 devices after Internet stack!" << std::endl;
  //   }
  // }

  // pagpsr.Install();
  // if (printRoutes)
  // {
  //   Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("aodvKmeans.routes", std::ios::out);
  //   aodvKmeans.PrintRoutingTableAllAt (Seconds (8), routingStream);
  // }
}


void SimulationTest::InstallApplications ()
{
  m_testFlows.clear();
  Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable>();
  uv->SetStream(888);

  uint16_t numTestFlows = 100;  // 生成多组测试数据对
  if(algorithm == "kmeans") {numTestFlows = 50;}
  std::set<std::pair<uint16_t, uint16_t>> usedPairs;

  // 如果启用异构，部分节点将产生更高的负载
  // uint16_t numHighLoadNodes = heterogeneous ? (size * 0.15) : 0;  // 15%高负载节点

  for (uint16_t i = 0; i < numTestFlows; i++) {
    uint16_t src, dst;
    do {    // 生成不重复的测试对
      src = uv->GetValue(0, size - 1);
      dst = uv->GetValue(0, size - 1);
    } while (src == dst || usedPairs.count({src, dst}) > 0);

    usedPairs.insert({src, dst});
    uint16_t port = 9000 + i;  // 每个测试流使用不同端口
    TestFlow flow;      // 记录测试流信息
    flow.srcNode = src;
    flow.dstNode = dst;
    flow.port = port;
    flow.srcAddr = interfaces.GetAddress(src);
    flow.dstAddr = interfaces.GetAddress(dst);
    m_testFlows.push_back(flow);
    Ptr<Node> srcNode = nodes.Get(src);      // 安装UDP Echo应用
    Ptr<Node> dstNode = nodes.Get(dst);
    UdpServerHelper server(port);      // 服务器端
    // UdpEchoServerHelper server(port);      // 服务器端
    ApplicationContainer serverApp = server.Install(dstNode);
    serverApp.Start(Seconds(9.0));
    serverApp.Stop(Seconds(totalTime));

    // 根据节点类型配置不同的流量负载
    UdpClientHelper client(flow.dstAddr, port);    // 客户端
    // if (heterogeneous && src < numHighLoadNodes) {
    //   // 高负载节点：更多数据包、更短间隔、更大包
    //   client.SetAttribute("MaxPackets", UintegerValue(500));       // 5倍数据量
    //   client.SetAttribute("Interval", TimeValue(Seconds(0.5)));    // 2倍发送频率
    //   client.SetAttribute("PacketSize", UintegerValue(128));       // 2倍包大小
    // } else {
      // 普通负载节点
      client.SetAttribute("MaxPackets", UintegerValue(200));
      client.SetAttribute("Interval", TimeValue(Seconds(1.0)));
      client.SetAttribute("PacketSize", UintegerValue(100));
    // }

    ApplicationContainer clientApp = client.Install(srcNode);
    clientApp.Start(Seconds(10.0 + i * 0.05));  // 错开启动
    clientApp.Stop(Seconds(totalTime - 0.1));
  }

  // if (heterogeneous) {
  //   std::cout << "异构负载配置: 高负载节点数=" << numHighLoadNodes << std::endl;
  // }

  monitor = flowmon.InstallAll();  // 安装FlowMonitor
  // m_metrics->SetFlowMonitor(monitor, DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier()));
  // std::vector<std::tuple<uint16_t, uint16_t, uint16_t, Ipv4Address, Ipv4Address>> testFlowsForMetrics;
  // for (const auto& flowa : m_testFlows) {
  //   testFlowsForMetrics.push_back( std::make_tuple(flowa.srcNode, flowa.dstNode, flowa.port, flowa.srcAddr, flowa.dstAddr) );
  // }
  // if (m_metrics) { m_metrics->SetTestFlows(testFlowsForMetrics); }
  
  Simulator::Schedule(Seconds(totalTime - 0.1),  &SimulationTest::PrintTestFlowStats, this);  // 在仿真结束前打印统计
}

// 只统计测试流的简洁输出函数
void SimulationTest::PrintTestFlowStats()
{ 
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> allStats = monitor->GetFlowStats();  
  // 表头
  // std::cout << std::left  << std::setw(8) << "Flow#" << std::setw(15) << "Source->Dest" << std::setw(8) << "Port"
  //           << std::setw(10) << "TxPkts"  << std::setw(10) << "RxPkts"<< std::setw(10) << "LostPkts" 
  //           << std::setw(10) << "PDR(%)"  << std::setw(12) << "Delay(ms)" << std::setw(10) << "Hops" << std::endl;
  // std::cout << std::string(95, '-') << std::endl;  
  // 统计每个测试流
  int       flowIndex  = 0;
  double    totalPDR   = 0.0;
  double    totalDelay = 0.0;
  uint64_t  totalHops  = 0;
  int       validFlows = 0;  
  for (const auto& testFlow : m_testFlows) {  // 查找匹配的FlowMonitor统计（根据端口和地址匹配）
      flowIndex++;
      uint32_t txPackets = 0, rxPackets = 0, lostPackets = 0;
      double   delaySum = 0.0;
      uint32_t timesForwarded = 0;
      bool     flowFound = false;    
      for (const auto& stat : allStats) {
          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(stat.first);      
          // 只统计匹配测试流的数据（通过端口号识别）
          if (t.sourceAddress == testFlow.srcAddr && t.destinationAddress == testFlow.dstAddr &&
            (t.destinationPort == testFlow.port  || t.sourcePort == testFlow.port)) 
          {        
            txPackets   += stat.second.txPackets;
            rxPackets   += stat.second.rxPackets;
            lostPackets += stat.second.lostPackets;
            delaySum    += stat.second.delaySum.GetMilliSeconds();
            timesForwarded += stat.second.timesForwarded;
            flowFound = true;
          }
      }    
      double pdr = (txPackets > 0) ? (double)rxPackets / txPackets * 100.0 : 0.0;
      double avgDelay = (rxPackets > 0) ? delaySum / rxPackets : 0.0;
      // double avgHops = (rxPackets > 0) ? (double)timesForwarded / rxPackets : 0.0;    
      uint64_t Hops = (rxPackets > 0) ? timesForwarded /rxPackets : 0;    
      if (flowFound && txPackets > 0) {
            totalPDR += pdr;
            totalDelay += avgDelay;
            totalHops += Hops;
            validFlows++;
      }
  }  
  ns3::PerformanceMetrics::CommonMetrics commonMetrics;  // 构建通用性能指标
  if (validFlows > 0) {
    commonMetrics.pdr        = totalPDR   / validFlows;
    commonMetrics.avgDelay   = totalDelay / validFlows;
    commonMetrics.avgHops    = totalHops  / validFlows;
    commonMetrics.validFlows = validFlows;
  }
  // 收集能耗信息
  double totalConsumed = 0.0;
  uint16_t deadNodes = 0;
  for (uint16_t i = 0; i < size; i++) {
        Ptr<Node> node = nodes.Get(i);
        Ptr<EnergySourceContainer> sources = node->GetObject<EnergySourceContainer>();
        if (sources && sources->GetN() > 0) {
          Ptr<BasicEnergySource> source = DynamicCast<BasicEnergySource>(sources->Get(0));
          if (source) {
            double remaining = source->GetRemainingEnergy();
            double initial = source->GetInitialEnergy();
            totalConsumed += (initial - remaining);
            if (remaining <= 0.0001) deadNodes++;
          }
        }
  }
  commonMetrics.totalEnergyConsumed = totalConsumed;
  commonMetrics.avgEnergyPerNode = (size > 0) ? totalConsumed / size : 0.0;
  commonMetrics.deadNodes = deadNodes;  
  // 计算吞吐量和控制开销
  TransmissionStats txStats = CalculateTransmissionStats();
  commonMetrics.throughput = txStats.avgThroughput;  
  // 计算控制开销比
  uint64_t totalControlPackets = 0;
  double totalAverageLinkLifetime = 0.0;
  double controlThroughput = 0.0;
  uint64_t hellopacketCounts = 0;
  for (uint32_t i = 0; i < size; i++) {
    if(algorithm == "biocluster"){
      Ptr<biocluster::RoutingProtocol> routing = nodes.Get(i)->GetObject<biocluster::RoutingProtocol>();
      totalControlPackets += routing->GetControlPacketCount();
      controlThroughput += routing->GetControlThroughput();
      hellopacketCounts += routing->GetHelloCount();

    } else if(algorithm == "leach"){
      Ptr<leach::RoutingProtocol> routing = nodes.Get(i)->GetObject<leach::RoutingProtocol>();
      totalControlPackets += routing->GetControlPacketCount();
      controlThroughput += routing->GetControlThroughput();
      hellopacketCounts += routing->GetHelloCount();

    } else if(algorithm == "icra"){
      Ptr<icra::RoutingProtocol> routing = nodes.Get(i)->GetObject<icra::RoutingProtocol>();
      totalControlPackets += routing->GetControlPacketCount();
      controlThroughput += routing->GetControlThroughput();
      hellopacketCounts += routing->GetHelloCount();

    } else if(algorithm == "kmeans") {
      Ptr<kmeans::RoutingProtocol> routing = nodes.Get(i)->GetObject<kmeans::RoutingProtocol>();
      totalControlPackets += routing->GetControlPacketCount();
      controlThroughput += routing->GetControlThroughput();
      hellopacketCounts += routing->GetHelloCount();

    } else if(algorithm == "aodv"){
      Ptr<aodv::RoutingProtocol> routing = nodes.Get(i)->GetObject<aodv::RoutingProtocol>();
      totalControlPackets += routing->GetTotalControlPackets();
      double avgLinkLifetime = routing->GetAverageLinkLifetime();      
      totalAverageLinkLifetime += avgLinkLifetime;
      hellopacketCounts += routing->GetHELLOCount();
      controlThroughput += routing->GetControlThroughout();

    } else if(algorithm == "olsr"){
      Ptr<olsr::RoutingProtocol> routing = nodes.Get(i)->GetObject<olsr::RoutingProtocol>();
      totalControlPackets += routing->GetTotalControlPackets();
      double avgLinkLifetime = routing->GetAverageLinkLifetime();
      totalAverageLinkLifetime += avgLinkLifetime;
      hellopacketCounts += routing->GetHelloCount();
      controlThroughput += routing->GetControlThroughput();
    }


  }  
  commonMetrics.controlOverheadRatio = (txStats.totalTxPackets > 0) ? (double)totalControlPackets / txStats.totalTxPackets : 0.0;
  commonMetrics.controlPacketNum = totalControlPackets;
  commonMetrics.ControlPacketTHP = controlThroughput / totalTime;
  commonMetrics.helloPacketCount = hellopacketCounts;
  // std::cout<<"控制包吞吐量: "<< commonMetrics.ControlPacketTHP<<"kbps, Hello个数"<<hellopacketCounts<< "个, Control 个数: "<<totalControlPackets<<"\n";
  commonMetrics.AvergeLinkLength = totalAverageLinkLifetime / size;
  if (m_metrics) {  // 传递给 PerformanceMetrics
    m_metrics->SetCommonMetrics(commonMetrics);
    m_metrics->CollectClusteringMetrics();
    m_metrics->CollectLinkMetrics();
    m_metrics->PrintComprehensiveReport(std::cout, algorithm, size, totalTime);
    std::string csvFilename = "benchmark_tests_" + std::to_string(size) + "nodes.csv";
    m_metrics->SaveToCSV(csvFilename, algorithm,seed, size);
    std::cout << "性能指标已保存至: " << csvFilename << std::endl;
  }




}


SimulationTest::TransmissionStats SimulationTest::CalculateTransmissionStats()
{
  TransmissionStats stats;  
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
  std::map<FlowId, FlowMonitor::FlowStats> allStats = monitor->GetFlowStats();  
  double minStartTime = std::numeric_limits<double>::max();
  double maxStopTime = 0.0;
  
  // 只统计测试流的数据
  for (const auto& testFlow : m_testFlows) {
    for (const auto& stat : allStats) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(stat.first);      
      if (t.sourceAddress == testFlow.srcAddr &&  t.destinationAddress == testFlow.dstAddr &&
        (t.destinationPort == testFlow.port || t.sourcePort == testFlow.port)) {
        
        stats.totalTxPackets += stat.second.txPackets;
        stats.totalRxPackets += stat.second.rxPackets;
        stats.totalRxBytes += stat.second.rxBytes;
        stats.totalHops += stat.second.timesForwarded;        
        minStartTime = std::min(minStartTime, stat.second.timeFirstTxPacket.GetSeconds());
        maxStopTime = std::max(maxStopTime, stat.second.timeLastRxPacket.GetSeconds());
      }
    }
  }  
  // 计算吞吐量和平均跳数
  stats.totalDuration = maxStopTime - minStartTime;
  if (stats.totalDuration > 0) {
    stats.avgThroughput = (stats.totalRxBytes * 8.0) / (stats.totalDuration * 1000.0); // kbps
  }  
  if (stats.totalRxPackets > 0) {
    stats.avgHops = (double)stats.totalHops / stats.totalRxPackets;
  }
  
  return stats;
}