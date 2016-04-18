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

#include <sqlite3.h> 

#include "ns3/internet-module.h"
#include "ns3/stats-module.h"
//#include "wifi-example-apps.h"



#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include <sstream>

//#include <stream> 
#include <string>
#include <stdio.h>
#include <iomanip>
#include <vector> 

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


void TxCallback (Ptr<CounterCalculator<uint32_t> > datac,
                 std::string path, Ptr<const Packet> packet) {
  NS_LOG_INFO ("Sent frame counted in " <<
               datac->GetKey ());
  datac->Update ();
  // end TxCallback
}




void writeVectorToFile(string filename, vector <string> strVector)
{
  fstream outfile; 
  outfile.open(filename.c_str(), fstream::in |  fstream::out);

  for(uint32_t i = 0; i < strVector.size(); i++){
      outfile << strVector[i] << endl;
   }
  outfile.close(); 

}

string convertFloatToString (float number){
    ostringstream buff;
    buff<<number;
    return buff.str();   
}


/******************************************************
* main
*******************************************************/
int main (int argc, char *argv[])
{

  uint32_t nMobile = 0; 			//number of mobile phones  
  uint32_t nSub = 0;  				//number of subwoofers 
  uint32_t nSat = 2 ;  				//number of satellites   
  uint32_t nAP = 1; 				//number of AP
  uint32_t nWifi = nSub + nSat + nMobile;   	//total number of wireless nodes, excluding AP
  uint32_t packetSize = 2048; 			// bytes
  uint32_t numPackets = 1; 			//320
  double interval = 1.0; 			// seconds

  bool verbose = true;
  bool tracing = true;


  string experiment ("luna-ns3-sim-test");
  string strategy ("luna-ns3-sim");
  string input;
  string runID;
  string format ("omnet");


  string output_perfThroughput = "./scratch/perfThroughput.txt";
  fstream outfile; 
  
  Time::SetResolution (Time::NS);


  vector<string> strVector; 


  {
    stringstream sstr;
    sstr << "run-" << time (NULL);
    runID = sstr.str ();
  }


  // Set up command line parameters used to control the experiment.
  CommandLine cmd;
  cmd.AddValue ("nSat", "Number of wifi STA devices, Satellites", nSat);
  cmd.AddValue ("packetSize", "Payload size", packetSize);
  cmd.AddValue ("numPackets", "Number of packets", numPackets);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("format", "Format to use for data output.", format);  
  cmd.AddValue ("experiment", "Identifier for experiment.", experiment);
  cmd.AddValue ("strategy", "Identifier for strategy.", strategy);
  cmd.AddValue ("run", "Identifier for run.", runID);
  cmd.Parse (argc,argv);


  if (format != "omnet" && format != "db") {
      NS_LOG_ERROR ("Unknown output format '" << format << "'");
      return -1;
    }

  #ifndef STATS_HAS_SQLITE3
  if (format == "db") {
      NS_LOG_ERROR ("sqlite support not compiled in.");
      return -1;
    }
  #endif

 {
    stringstream sstr ("");
    sstr << nSat;
    input = sstr.str ();
  }


  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  cout<<".........................................."<<endl;
  cout<<"nSat:"<<nSat <<endl; 
  cout<<"runID:"<<runID <<endl; 
  
  char trialID_char  =runID.at(runID.size()-1);
  cout<< "trialID char:"<<trialID_char<<"  size:"<<sizeof(trialID_char) <<endl;  
  char array [2]= {}; 
  array [0] = (char)trialID_char; 
  //array [0] = '2'; 
  int trialID = atoi(array); 
  int col = trialID; 
  int row= nSat;
  cout<<"col:"<<col<<" "<<"row:"<<row<<endl;

  string line;
  uint32_t number_of_lines=0;

  float   avgThroughput;


/********************************************
*check how many lines file has
*********************************************/
  fstream tempfile; 
  tempfile.open(output_perfThroughput.c_str(), fstream::in | fstream::out);
  if(tempfile == NULL) 
  {
  	cout<<"error opening file for counting"<<endl;
        number_of_lines=0;
  	
  }
  else 
  { 
	  while (getline(tempfile, line)){
              ++number_of_lines;
              strVector.push_back(line); 
          } 

	  std::cout << "Number of lines in text file: " << number_of_lines<<endl;

	  tempfile.close(); 
 } 


/********************************************
*check how to write data in text. new row and/or new column 
*********************************************/
      // Create a vector iterator
       vector<string>::iterator it;

// Loop through the vector from start to end and print out the string

        // the iterator is pointing to.

        for (it = strVector.begin(); it < strVector.end(); it++) {

            cout << *it << endl;

        }


  for(uint32_t i = 0; i < strVector.size(); i++){
      cout << "value of vec [" << i << "] = " << strVector[i] << endl;
   }

//   strVector[1].append("   32");

  for(uint32_t i = 0; i < strVector.size(); i++){
      cout << "new value of vec [" << i << "] = " << strVector[i] << endl;
   }


/*
  outfile.open(output_perfThroughput.c_str(), fstream::in |  fstream::out);
    
  for(uint32_t i = 0; i < strVector.size(); i++){
      outfile << strVector[i] << endl;
   }
  outfile.close();
*/


  
  cout<<".........................................."<<endl;


 /* cout<<"reading"<<endl;
  while(getline(outfile,line))
  {
    cout<<line<<".."<<endl;
  }
 */
  //outfile <<nSat<<"\t "<< nSat*4<<endl; 
 // outfile.close();



  //return 0; 

  /******************************************************
  * Nodes + AP
  *******************************************************/
  cout<<"Creating Nodes and AP ..."<<endl;
  nWifi = nSub + nSat + nMobile;   	//total number of wireless nodes, excluding AP

  // Check for valid number of wifi nodes
  if (nWifi > maxNodes )
    {
      cout << "Too many wifi, no more than 50 each." << endl;
      return 1;
    }
  cout << "nWifi =   " << nWifi << endl;  
  

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);   

  NodeContainer wifiApNode; 
  wifiApNode.Create (nAP);
  cout<<"Nodes and AP created \n"<<endl;


  /******************************************************
  * Topology: Channel & NetDevice
  *******************************************************/
  cout<<"Creating Topology and creating network devices in nodes  ..."<<endl;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  //wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  //wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
 
  //wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel",
  //                                "Exponent", DoubleValue (3.0));


  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); // ns-3 supports RadioTap and Prism tracing extensions for 802.11b

  wifiPhy.SetChannel (wifiChannel.Create ());
  cout<<"Topology and network devices installation completed \n"<<endl;


  //Configure Nodes MAC layer
  cout<<"Configuring MAC layer ..."<<endl;
  string phyMode ("DsssRate11Mbps");
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  //wifi.SetRemoteStationManager ("ns3::AarfWifiManager");  //set rate control algorithm
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));

  /*
  if (verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }
  */

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


  /******************************************************
  * Internet Stack & Ipv4Address
  *******************************************************/
  //Set up protocol stack
  cout<<"Setting up protocol stack and ipv4 address ..."<<endl;
  InternetStackHelper stack;
  stack.Install (wifiStaNodes);
  stack.Install (wifiApNode);

  //Configure IPv4 
  cout<<"Configuring IPv4 address for nodes ..."<<endl;
  Ipv4AddressHelper ipv4;
  Ipv4Address addr;

  //ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer sta_interface; 
  Ipv4InterfaceContainer ap_interface;
  ap_interface = ipv4.Assign (apDevices);
  sta_interface = ipv4.Assign (staDevices);


 //debug ip addresses 
 for(uint32_t i = 0 ; i < nWifi; i++)
 {
	addr = sta_interface.GetAddress(i);
	std::cout << " Node " << i+1 << "\t "<< "IP Address "<<addr << std::endl;
 }

 addr = ap_interface.GetAddress(0);
 std::cout << " AP " << "\t "<< "IP Address "<<addr << std::endl;
 std::cout << "Internet Stack & IPv4 address configured.." << '\n';
 cout<<"IPv4 address setup completed \n"<<endl;
  

  /******************************************************
  * Node Mobibility
  *******************************************************/
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
  //mobility.Install (wifiStaNodes); //set mobility  constant position  model on subwoofer and satellites

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  //mobility.Install (wifiStaNodes.Get(0)); //set mobility model on mobile phone
   mobility.Install (wifiStaNodes); //set mobility  constant position  model on subwoofer and satellites



  /******************************************************
  * Application & Simulation
  *******************************************************/
  uint16_t port = 4000;
  UdpEchoServerHelper echoServer (port);
  ApplicationContainer serverApps = echoServer.Install (wifiStaNodes.Get (0)); //set server on sat //1
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (sta_interface.GetAddress (0), port); //1 
  echoClient.SetAttribute ("MaxPackets", UintegerValue(numPackets));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds(interval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue(packetSize));
  //echoClient.SetAttribute ("Interval", TimeValue (Time("0.002")));

  ApplicationContainer clientApps =  echoClient.Install (wifiApNode.Get (0)); //set client on mobile phone 
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

 
#ifdef Tester
  //------------------------------------------------------------
  //-- Create a custom traffic source and sink
  //--------------------------------------------
  NS_LOG_INFO ("Create traffic source & sink.");
  Ptr<Node> appSource = NodeList::GetNode (0);
  Ptr<Sender> sender = CreateObject<Sender>();
  appSource->AddApplication (sender);
  sender->SetStartTime (Seconds (1));

  Ptr<Node> appSink = NodeList::GetNode (1);
  Ptr<Receiver> receiver = CreateObject<Receiver>();
  appSink->AddApplication (receiver);
  receiver->SetStartTime (Seconds (0));

 // Config::Set ("/NodeList/*/ApplicationList/*/$Sender/Destination",
   //            Ipv4AddressValue ("10.1.1.2"));
#endif 


// Simulator::Stop (Seconds (10.0));
  


  /******************************************************
  * Performance measurements
  *******************************************************/
  FlowMonitorHelper flowmon; 
  Ptr<FlowMonitor>monitor=flowmon.InstallAll();
   

  if (tracing == true)
    {
      //wifiPhy.EnablePcap ("third", staDevices.Get (1));
      //wifiPhy.EnablePcap ("third", apDevices.Get (0));
      wifiPhy.EnablePcapAll ("wifiPhyPcapAll");
    }



  //------------------------------------------------------------
  //-- Setup stats and data collection
  //--------------------------------------------
  // Create a DataCollector object to hold information about this run.
  DataCollector luna_ns3_sim_data;
  luna_ns3_sim_data.DescribeRun (experiment,
                    strategy,
                    input,
                    runID);


  // Add any information we wish to record about this run.
  luna_ns3_sim_data.AddMetadata ("author", "javier");


  

#ifdef TEST
// Create a counter to track how many frames are generated.  Updates
  // are triggered by the trace signal generated by the WiFi MAC model
  // object.  Here we connect the counter to the signal via the simple
  // TxCallback() glue function defined above.
  Ptr<CounterCalculator<uint32_t> > totalTx =
    CreateObject<CounterCalculator<uint32_t> >();
  totalTx->SetKey ("wifi-tx-frames");
  totalTx->SetContext ("node[0]");
//  Config::Connect ("/NodeList/0/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTx",
 //                  MakeBoundCallback (&TxCallback, totalTx));
  luna_ns3_sim_data.AddDataCalculator (totalTx);

  // This is similar, but creates a counter to track how many frames
  // are received.  Instead of our own glue function, this uses a
  // method of an adapter class to connect a counter directly to the
  // trace signal generated by the WiFi MAC.
  Ptr<PacketCounterCalculator> totalRx =
    CreateObject<PacketCounterCalculator>();
 totalRx->SetKey ("wifi-rx-frames");
  totalRx->SetContext ("node[1]");
 // Config::Connect ("/NodeList/1/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx",
 //                  MakeCallback (&PacketCounterCalculator::PacketUpdate,
  //                               totalRx));
  luna_ns3_sim_data.AddDataCalculator (totalRx);




  // This counter tracks how many packets---as opposed to frames---are
  // generated.  This is connected directly to a trace signal provided
  // by our Sender class.
  Ptr<PacketCounterCalculator> appTx =
    CreateObject<PacketCounterCalculator>();
  appTx->SetKey ("sender-tx-packets");
  appTx->SetContext ("node[0]");
 // Config::Connect ("/NodeList/0/ApplicationList/*/$Sender/Tx",
 //                  MakeCallback (&PacketCounterCalculator::PacketUpdate,
  //                               appTx));
  luna_ns3_sim_data.AddDataCalculator (appTx);


  // Here a counter for received packets is directly manipulated by
  // one of the custom objects in our simulation, the Receiver
  // Application.  The Receiver object is given a pointer to the
  // counter and calls its Update() method whenever a packet arrives.
  Ptr<CounterCalculator<> > appRx =
    CreateObject<CounterCalculator<> >();
  appRx->SetKey ("receiver-rx-packets");
  appRx->SetContext ("node[1]");
  receiver->SetCounter (appRx);
  luna_ns3_sim_data.AddDataCalculator (appRx);

 // This DataCalculator connects directly to the transmit trace
  // provided by our Sender Application.  It records some basic
  // statistics about the sizes of the packets received (min, max,
  // avg, total # bytes), although in this scenaro they're fixed.
  Ptr<PacketSizeMinMaxAvgTotalCalculator> appTxPkts =
    CreateObject<PacketSizeMinMaxAvgTotalCalculator>();
  appTxPkts->SetKey ("tx-pkt-size");
  appTxPkts->SetContext ("node[0]");
  //Config::Connect ("/NodeList/0/ApplicationList/*/$Sender/Tx",
  //                 MakeCallback
  //                   (&PacketSizeMinMaxAvgTotalCalculator::PacketUpdate,
   //                  appTxPkts));
  luna_ns3_sim_data.AddDataCalculator (appTxPkts);

  // Here we directly manipulate another DataCollector tracking min,
  // max, total, and average propagation delays.  Check out the Sender
  // and Receiver classes to see how packets are tagged with
  // timestamps to do this.
  Ptr<TimeMinMaxAvgTotalCalculator> delayStat =
    CreateObject<TimeMinMaxAvgTotalCalculator>();
  delayStat->SetKey ("delay");
  delayStat->SetContext (".");
  receiver->SetDelayTracker (delayStat);
  luna_ns3_sim_data.AddDataCalculator (delayStat);
#endif 




  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();


 //------------------------------------------------------------
  //-- Generate FlowMonitor statistics output.
  //--------------------------------------------
  monitor->CheckForLostPackets ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  cout<<endl<<endl<<"**Performance analys**"<<endl;
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      //if ((t.sourceAddress=="10.1.1.1" && t.destinationAddress == "10.1.1.7"))
      //if ((t.sourceAddress=="10.1.1.1") )
     // {
          cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      	  //std::cout << " Average Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/nWifi  << " kbps\n";
          avgThroughput=i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/nWifi ;
      	  cout << " Average Throughput: " << avgThroughput << " kbps\n";
         // }
     }

  monitor->SerializeToXmlFile("luna.flowmon", true, true);

  string temp;
  temp = convertFloatToString(avgThroughput);

  strVector[1].append("  ");
  strVector[1].append(temp);

  
  //outfile.open(output_perfThroughput.c_str(), fstream::in |  fstream::out);
   
  writeVectorToFile(output_perfThroughput.c_str(),strVector);
 
 /*
 for(uint32_t i = 0; i < strVector.size(); i++){
      outfile << strVector[i] << endl;
   }
  outfile.close();
*/ 
  
/*
 //------------------------------------------------------------
  //-- Generate statistics output.
  //--------------------------------------------
  
  






  // Pick an output writer based in the requested format.
  Ptr<DataOutputInterface> output = 0;
  if (format == "omnet") {
      NS_LOG_INFO ("Creating omnet formatted data output.");
      output = CreateObject<OmnetDataOutput>();
    } else if (format == "db") {
    #ifdef STATS_HAS_SQLITE3
      NS_LOG_INFO ("Creating sqlite formatted data output.");
      output = CreateObject<SqliteDataOutput>();
    #endif
    } else {
      NS_LOG_ERROR ("Unknown output format " << format);
    }

  // Finally, have that writer interrogate the DataCollector and save
  // the results.
  if (output != 0)
    output->Output (luna_ns3_sim_data);

*/








  Simulator::Destroy ();





  
  cout<<"program done "<<endl;
  return 0;

}
