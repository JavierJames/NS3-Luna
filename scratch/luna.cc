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

#include "ns3/core-module.h"
//#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
//#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>


// Default Network Topology
//
// Number of wifi or csma nodes can be increased up to 250
//                          |
//                 Rank 0   |   Rank 1
// -------------------------|----------------------------
//   Wifi 10.1.1.0
//                 AP   *--m0 
//  *    *    *    *
//  |    |    |    |    
// s0   n2   n1   n0 
//
/*

Phase A: Setup environment 
1. Create Nodes 
2. Create Topology 
3. Create network device drivers on nodes 
4. install protocol stack on nodes 
5. give ip addresses to network device drivers on nodes 

Phase B: Execute and Simulate 
1. Create application on server (UDP server)  
2. Create application on client (UDP client)  
3. Simulate   
*/

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("Luna NS-3 Project: Wifi Infrastructur (AP + Nodes)");


uint32_t maxNodes = 50; //maximum number of nodes, excluding AP


int main (int argc, char *argv[])
{
  uint32_t packetSize = 1024; // bytes
  uint32_t numPackets = 320;
  double interval = 1.0; // seconds


  bool verbose = true;
  bool tracing = true;

  uint32_t nSub = 1;  	//number of subwoofers 
  uint32_t nSat = 5;  	//number of satellites   
  uint32_t nMobile = 1; //number of mobile phones  
  uint32_t nAP = 1; 	//number of AP
  uint32_t nWifi = 0;   //total number of wireless nodes, excluding AP 



  Time::SetResolution (Time::NS);

  CommandLine cmd;
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.Parse (argc,argv);

  // Check for valid number of wifi nodes
  if (nWifi > maxNodes )
    {
      cout << "Too many wifi, no more than 50 each." << endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }


  /*Phase A: Setup environment*/
 
  //Step 1: create Nodes + AP
  cout<<"Creating Nodes and AP ..."<<endl;
  nWifi = nSub + nSat + nMobile;   

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);   

  NodeContainer wifiApNode; 
  wifiApNode.Create (nAP);
  cout<<"Nodes and AP created \n"<<endl;


  //Step 2: Create Topology 
  cout<<"Creating Topology and creating network devices in nodes  ..."<<endl;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  //wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  //wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  //wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel",
  //                                "Exponent", DoubleValue (3.0));


  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  //wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); // ns-3 supports RadioTap and Prism tracing extensions for 802.11b


  wifiPhy.SetChannel (wifiChannel.Create ());
  cout<<"Topology and network devices installation completed \n"<<endl;


  //Configure Nodes MAC layer
  cout<<"Configuring MAC layer ..."<<endl;
  string phyMode ("DsssRate11Mbps");
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");  //set rate control algorithm
  //wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
  //                              "DataMode",StringValue (phyMode),
  //                              "ControlMode",StringValue (phyMode));


  if (verbose)
    {
     // wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }



  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  Ssid ssid = Ssid ("luna-airplay");

  //configure STA upper mac
  wifiMac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (wifiPhy, wifiMac, wifiStaNodes);


  //Configure AP upper mac
  wifiMac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (wifiPhy, wifiMac, wifiApNode);



  //Set up protocol stack
  cout<<"Setting up protocol stack ..."<<endl;
  InternetStackHelper stack;
  stack.Install (wifiStaNodes);
  stack.Install (wifiApNode);
  cout<<"Protocl stack setup completed \n"<<endl;

  //Configure IPv4 
  cout<<"Configuring IPv4 address for nodes ..."<<endl;
  Ipv4AddressHelper ipv4;

  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer sta_interface; 
  Ipv4InterfaceContainer ap_interface;
  sta_interface = ipv4.Assign (staDevices);
  ap_interface = ipv4.Assign (apDevices);


  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  cout<<"IPv4 address setup completed \n"<<endl;
  

  /*Setting up mobility*/
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode); //set constant position mobility model on AP 
//  mobility.Install (wifiStaNodes); //set mobility  constant position  model on subwoofer and satellites

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-1500, 1500, -1500, 1500)));
  //mobility.Install (wifiStaNodes.Get(0)); //set mobility model on mobile phone
  mobility.Install (wifiStaNodes); //set mobility  constant position  model on subwoofer and satellites


  /*Phase B: Execute and simulate */
  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (wifiStaNodes.Get (nWifi-1)); //set server on sat
  //ApplicationContainer serverApps = echoServer.Install (wifiStaNodes.Get (1)); //set server on sat
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (sta_interface.GetAddress (nWifi-1), 9); 
  //UdpEchoClientHelper echoClient (sta_interface.GetAddress (1), 9); 
  echoClient.SetAttribute ("MaxPackets", UintegerValue(numPackets));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds(interval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue(packetSize));

  ApplicationContainer clientApps =  echoClient.Install (wifiStaNodes.Get (0)); //set client on mobile phone 
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  
  //Throughput calculated using Flowmon
  FlowMonitorHelper flowmon; 
  Ptr<FlowMonitor>monitor=flowmon.InstallAll();
   

 
  Simulator::Stop (Seconds (10.0));

  if (tracing == true)
    {
      //wifiPhy.EnablePcap ("third", apDevices.Get (0));
      wifiPhy.EnablePcapAll ("wifiPhyPcapAll");
    }

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();

   monitor->CheckForLostPackets ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      //if ((t.sourceAddress=="10.1.1.1" && t.destinationAddress == "10.1.1.7"))
      if ((t.sourceAddress=="10.1.1.1") )
      {
          std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      	  std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
      }
     }

  monitor->SerializeToXmlFile("luna.flowmon", true, true);

  Simulator::Destroy ();
  
 cout<<"program done "<<endl;
return 0;



}
