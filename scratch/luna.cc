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
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"

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

NS_LOG_COMPONENT_DEFINE ("Luna NS-3 Project");


int
main (int argc, char *argv[])
{

  bool verbose = true;
  uint32_t nCsma = 0;

  uint32_t nSub = 1;  	// number of subwoofers 
  uint32_t nSat = 5;  	//number of satellites   
  uint32_t nMobile = 1; //number of mobile phones  
  uint32_t nAP = 1; 	//number of AP
  uint32_t nWifi = 0;  

  bool tracing = false;

 

  Time::SetResolution (Time::NS);

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.Parse (argc,argv);

  // Check for valid number of csma or wifi nodes
  // 250 should be enough, otherwise IP addresses 
  // soon become an issue
//  cout<<"Program start \n nWifi"<<nWifi<<endl;
  if (nWifi > 50 )
    {
      std::cout << "Too many wifi, no more than 50 each." << std::endl;
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
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());
  cout<<"Topology and network devices installation completed \n"<<endl;

  //Configure MAC layer
  cout<<"Configuring MAC layer ..."<<endl;
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");  //set rate control algorithm

  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

  Ssid ssid = Ssid ("luna-airplay");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);


  /*Setting up mobility*/
  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-1500, 1500, -1500, 1500)));
  mobility.Install (wifiStaNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  //Set up protocol stack
  cout<<"Setting up protocol stack ..."<<endl;
  InternetStackHelper stack;
  stack.Install (wifiStaNodes);
  stack.Install (wifiApNode);
  cout<<"Protocl stack setup completed \n"<<endl;

  //Configure IPv4 
  cout<<"Configuring IPv4 address for nodes ..."<<endl;
  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer sta_interface; 
  Ipv4InterfaceContainer ap_interface;
  sta_interface = address.Assign (staDevices);
  ap_interface = address.Assign (apDevices);
  cout<<"IPv4 address setup completed \n"<<endl;
  
  /*Phase B: Execute and simulate */
  UdpEchoServerHelper echoServer (9);
  ApplicationContainer serverApps = echoServer.Install (wifiStaNodes.Get (nWifi-1)); //set server on mobile phone 
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (sta_interface.GetAddress (nWifi-1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps =  echoClient.Install (wifiStaNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //FlowMonitorHelper flowmon; 
  //Ptr<FLowMonitor>monitor=flowmon.InstallAll();
   

 
  Simulator::Stop (Seconds (10.0));

  if (tracing == true)
    {
      //pointToPoint.EnablePcapAll ("third");
      phy.EnablePcap ("third", apDevices.Get (0));
      //csma.EnablePcap ("third", csmaDevices.Get (0), true);
    }

  Simulator::Run ();
  Simulator::Destroy ();
  
 cout<<"program done "<<endl;
return 0;



}
