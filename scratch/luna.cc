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
//
#include "ns3/core-module.h"
//#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
//#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/propagation-loss-model.h"

#include <sqlite3.h> 

#include "ns3/internet-module.h"
#include "ns3/stats-module.h"
//#include "wifi-example-apps.h"



#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <ctime>
#include <sstream>

//#include <stream> 
#include <string>
#include <stdio.h>
#include <iomanip>
#include <vector> 

#include <utility>
#include <complex>
//#include <tuple>

#include <cmath>
#include <ctgmath>


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


void writeVectorToFile(string filename, vector <string> strVector)
{
  ofstream file; 
  file.open(filename.c_str());

  for(uint32_t i = 0; i < strVector.size(); i++){
      file << strVector[i] << endl;
      //cout<< strVector[i] << endl;
   }
  file.close(); 

}


string convertFloatToString (float number){
    ostringstream buff;
    buff<<number;
    return buff.str();   
}

string convertUintToString (uint32_t number){
    ostringstream buff;
    buff<<number;
    return buff.str();
}



void printVector(vector<string> dataVector)
{

  for(uint32_t i = 0; i < dataVector.size(); i++){
      cout << "value of vec [" << i << "] = " << dataVector[i] << endl;
   }


}

int findParameterIndex(vector<string> data, string parameter)
{
  uint32_t size;

  size=data.size();
  int index=-1;

//cout<<"string received: "<<parameter<<endl;

 for(uint32_t i=0; i< size; i++)
 {
        stringstream ss(data[i]);
        string word;

       ss>>word;
     // cout<<"word: "<<" "<<word<<endl;
     //cout<<"parameter: " <<parameter<<endl;

    if( parameter.compare(word) == 0)
    {
    // cout<<"paramter found in data"<<endl;
      index= i;
      break;
    }
 }
   return index;

}

vector<string>  addNewResults(vector<string> vector, string parameter, string result)
{

  int index;
  index =  findParameterIndex(vector,parameter);
  //cout<<"index of paramter: "<<index<<endl;
  string line;


  //parameter recorded before? 
  if(vector.size()==0 || index == -1)
  {
     //cout<<"first time storing parameter"<<endl;
     line = parameter;
     line.append("  ");
     line.append(result);

     vector.push_back(line);

 }
 else{
    // cout<<"appending new results to parameter"<<endl;
   vector[index].append("  ");
   vector[index].append(result);

 }
  return vector;

}



vector<string>  addNewResultsStats(vector<string> vector, string parameter, string avg, string stdv)
{

  int index;
  index =  findParameterIndex(vector,parameter);
  //cout<<"index of paramter: "<<index<<endl;
  string line;


  //parameter recorded before? 
  if(vector.size()==0 || index == -1)
  {
     //cout<<"first time storing parameter"<<endl;
     line = parameter;
     line.append("  ");
     line.append(avg);
     line.append("  ");
     line.append(stdv);

     vector.push_back(line);

 }
 else{
    // cout<<"appending new results to parameter"<<endl;
   vector[index].append("  ");
   vector[index].append(avg);
   vector[index].append("  ");
   vector[index].append(stdv);

 }
  return vector;

}


vector<string> fetchDataFromFile (string filename, uint32_t *lineNumbers)
{

  ifstream file;
  file.open(filename.c_str());

  string line;
  vector<string> temp;

  if(file == NULL)
  {
        cout<<"error opening file for counting or file does not exist "<<endl;
        *lineNumbers=0;

  }
  else
  {
          while (getline(file, line)){
              (*lineNumbers)= (*lineNumbers) +1 ;
              temp.push_back(line);
          }

          //cout << "Number of lines in text file: " << *lineNumbers<<endl;

          file.close();
 }

  return temp;



}

vector<string> calculateStats(vector<string> data)
{

  unsigned int size;

  size=data.size();
  string parameterString;
  //float array [size-1];
  int wordNum;
  vector<string> tempVec;

   vector<string> results;

 for(unsigned int i=0; i< size; i++)
 {
        //read the line 
        stringstream ss(data[i]);
        string word;
        unsigned int length;
        float sum=0;
        float numbers;
        float avg;
        float standard_dev=0;

        length = data[i].length();

        float array[length];
        //float array[7]; 

        for (wordNum = 0; ss >> word; wordNum++)
        {
                if(wordNum==0){
                        parameterString=word;
                }
                else{
                       // cout << " " << wordNum << "." << word<<endl;
                        array[wordNum-1]=stof(word);
               }
        }

       // cout<<"float results:"<<endl;
        numbers = wordNum-1;
        for(int i=0; i<numbers; i++)
        {
                sum += array[i];
               // cout<<"array["<<i<<"]: "<<array[i]<<endl;
        }
       avg= sum/numbers;
       //cout<<"avg ="<<avg<<" = "<<sum<<" /"<<numbers<<endl;

        for(int i=0; i<numbers; i++)
        {
                standard_dev += pow(array[i]-avg,2);
               // cout<<"array["<<i<<"]: "<<array[i]<<endl;
        }
       float temp=standard_dev;
       standard_dev = sqrt(temp/numbers);
       //cout<<"standard_dev ="<<standard_dev<<endl;

      //results =  addNewResults(results, parameterString, convertFloatToString(avg));
      results =  addNewResultsStats(results, parameterString, convertFloatToString(avg), convertFloatToString(standard_dev));


  }

 return results;


}








/******************************************************
* main
*******************************************************/
int main (int argc, char *argv[])
{

  double interval = 1.0; 			// seconds
  uint32_t nMobile = 0; 			//number of mobile phones  
  uint32_t nSub = 0;  				//number of subwoofers 
  uint32_t nSat = 1 ;  				//number of satellites   
  uint32_t nAP = 1; 				//number of AP

  /* Parameters to change */
  uint32_t nWifi = nSub + nSat + nMobile;   	//total number of wireless nodes, excluding AP //segmentation fault accurs after 17 nodes
  uint32_t packetSize = 160; 	 	// bytes
  uint32_t numPackets = 320; 			//320
  double distance = 10;
  //string dataRate = "DsssRate1Mbps";
  //string dataRate = "DsssRate2Mbps";
  //string dataRate = "DsssRate5_5Mbps";
  string dataRate = "DsssRate11Mbps";

  /* Qos parameters to observe */
  float avgThroughput;
  float avgJitter;
  float avgDelay;
  float packetlossRatio;

  bool verbose = true;
  bool tracing = true;


  string experiment ("luna-ns3-sim-test");
  string strategy ("luna-ns3-sim");
  string runID;
  string format ("omnet");

// Create randomness based on time
time_t timex;
time(&timex);
RngSeedManager::SetSeed(timex);
RngSeedManager::SetRun(1);


  fstream file; 
  ifstream infile; 
  ofstream outfile; 
  //string output_perfThroughput = "./scratch/test.txt";
  //string output_perfThroughputStats = "./scratch/testStats.txt";
  //string output_perfThroughput = "./scratch/perfThroughput.txt";
  //string output_perfThroughputStats = "./scratch/perfThroughputAvg.txt";
  string output_perfDelay = "./scratch/perfDelay.txt";
  string output_perfDelayStats = "./scratch/perfDelayStats.txt";
  string fileID;
  string filenameResults;
  string filenameStats;

  vector<string> strVector; 
  string line;
  uint32_t number_of_lines=0;
  vector < pair<int, string> > vectorPair;
  pair<int,string> intStringData; 
  vector<string> tempVec = strVector;
  
  Time::SetResolution (Time::NS);


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
  cmd.AddValue("distance", "Distance apart to place nodes (in meters).", distance); 
  cmd.AddValue("dataRate", "Link Bandwidth (in Mbps).", dataRate); 

  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("format", "Format to use for data output.", format);  
  cmd.AddValue ("experiment", "Identifier for experiment.", experiment);
  cmd.AddValue ("strategy", "Identifier for strategy.", strategy);
  cmd.AddValue ("run", "Identifier for run.", runID);
  cmd.Parse (argc,argv);


  fileID ="./scratch/"; 
  //fileID.append(runID); 
  fileID.append(dataRate); 
  filenameResults ="./scratch/"; 
  //filenameResults.append(runID); 
  filenameResults.append(dataRate); 
  filenameResults.append(".txt"); 
  filenameStats ="./scratch/"; 
  //filenameStats.append(runID); 
  filenameStats.append(dataRate); 
  filenameStats.append("-Stats.txt"); 
  cout<<"run ID: "<<fileID<<endl;
  cout<<"Filename : "<<filenameResults<<endl;
  cout<<"Filename Stats : "<<filenameStats<<endl;


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


  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }


/********************************************
*capture data from files & read number of lines
*********************************************/
//strVector= fetchDataFromFile(output_perfThroughput.c_str(),&number_of_lines);
//strVector= fetchDataFromFile(output_perfDelay.c_str(),&number_of_lines);
strVector= fetchDataFromFile(filenameResults.c_str(),&number_of_lines);

cout<<"Data read from file "<<endl;
printVector(strVector);
cout<<"number of lines: "<<number_of_lines<<endl;


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

  NodeContainer nodes; 
  nodes.Add(wifiStaNodes.Get(0)); 
  nodes.Add(wifiApNode.Get(0)); 

  NodeContainer nodesAh; 
  nodesAh.Create(2);


  cout<<"Nodes and AP created \n"<<endl;

  /******************************************************
  * Topology: Channel & NetDevice
  *******************************************************/
  cout<<"Creating Topology and creating network devices in nodes  ..."<<endl;
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  //wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (2));
 
  //wifiChannel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel",
  //                                "Exponent", DoubleValue (3.0));

  //wifiChannel.SetPropagationDelay ("ns3::PropagationDelayModel");
  wifiChannel.SetPropagationDelay ("ns3::RandomPropagationDelayModel");
  //wifiChannel.AddPropagationLoss ("ns3::PropogationDelayModel");


  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); // ns-3 supports RadioTap and Prism tracing extensions for 802.11b

  wifiPhy.SetChannel (wifiChannel.Create ());
  cout<<"Topology and network devices installation completed \n"<<endl;


  //Configure Nodes MAC layer
  cout<<"Configuring MAC layer ..."<<endl;
  string phyMode; 
  phyMode= dataRate;
  //phyMode= "DsssRate1Mbps";


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


  wifiMac.SetType ("ns3::AdhocWifiMac");
 NetDeviceContainer nodeDevices = wifi.Install(wifiPhy, wifiMac, nodesAh); 


  /******************************************************
  * Internet Stack & Ipv4Address
  *******************************************************/
  //Set up protocol stack
  cout<<"Setting up protocol stack and ipv4 address ..."<<endl;
  InternetStackHelper stack;
  stack.Install (wifiStaNodes);
  stack.Install (wifiApNode);
  stack.Install (nodesAh);

  //Configure IPv4 
  cout<<"Configuring IPv4 address for nodes ..."<<endl;
  Ipv4AddressHelper ipv4;
  Ipv4Address addr;

  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer sta_interface; 
  Ipv4InterfaceContainer ap_interface;
  ap_interface = ipv4.Assign (apDevices);
  sta_interface = ipv4.Assign (staDevices);

  Ipv4InterfaceContainer nodes_interface;
  nodes_interface = ipv4.Assign (nodeDevices);

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
  

 for(uint32_t i = 0 ; i < 2; i++)
 {
	addr = nodes_interface.GetAddress(i);
	std::cout << " Node " << i+1 << "\t "<< "IP Address "<<addr << std::endl;
 }


  /******************************************************
  * Node Mobibility
  *******************************************************/
/* 
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

 // mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
  //                           "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-100, 100, -100, 100)));
  //mobility.Install (wifiStaNodes.Get(0)); //set mobility model on mobile phone
   mobility.Install (wifiStaNodes); //set mobility  constant position  model on subwoofer and satellites
*/

  NS_LOG_INFO("Installing static mobility; distance " << distance << " .");
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
  positionAlloc->Add(Vector(0.0, 0.0, 0.0));
  positionAlloc->Add(Vector(0.0, distance, 0.0));
  //positionAlloc->Add(Vector( distance,0.0, 0.0));
  mobility.SetPositionAllocator(positionAlloc);
  //NodeContainer nodes( wifiStaNodes, wifiApNode); 
//  NodeContainer nodes; 
 // nodes.Add(wifiStaNodes.Get(0)); 
 // nodes.Add(wifiApNode.Get(0)); 
  //mobility.Install (nodes); //set constant position mobility model on AP 
   mobility.InstallAll();

   mobility.Install(nodesAh);



  /******************************************************
  * Application & Simulation
  *******************************************************/
  uint16_t port = 4000;
  UdpEchoServerHelper echoServer (port);
  //ApplicationContainer serverApps = echoServer.Install (wifiStaNodes.Get (0)); //set server on sat //1
  ApplicationContainer serverApps = echoServer.Install (nodesAh.Get (0)); //set server on sat //1
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  //UdpEchoClientHelper echoClient (sta_interface.GetAddress (0), port); //1 
  UdpEchoClientHelper echoClient (nodes_interface.GetAddress (0), port); //1 
  echoClient.SetAttribute ("MaxPackets", UintegerValue(numPackets));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds(interval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue(packetSize));

  //ApplicationContainer clientApps =  echoClient.Install (wifiApNode.Get (0)); //set client on mobile phone 
  ApplicationContainer clientApps =  echoClient.Install (nodesAh.Get (1)); //set client on mobile phone 
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();


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
          cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";

           avgThroughput=i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/nWifi ;
      	  avgDelay =(i->second.delaySum.GetMilliSeconds())/(i->second.rxPackets) ; 
          if(numPackets ==1) avgJitter=0; 
          else avgJitter = (i->second.jitterSum.GetMilliSeconds())/(i->second.rxPackets-1); 
          packetlossRatio = (i->second.lostPackets)/ ( i->second.lostPackets + i->second.rxPackets);


      	  cout << " Average Throughput: " << avgThroughput << " kbps"<<endl;
      	  cout << " Average Delay: " << avgDelay  << " ms"<<endl;
          cout << " Average Jitter: " << avgJitter << " ms"<<endl;
          cout << " Packet Loss Ratio: " << packetlossRatio <<endl;


     }

  monitor->SerializeToXmlFile("luna.flowmon", true, true);

/*store new data in data vector */
tempVec = strVector;
//strVector=  addNewResults(tempVec, convertUintToString(nSat),convertFloatToString(avgThroughput));
strVector=  addNewResults(tempVec, convertUintToString(packetSize),convertFloatToString(avgDelay));

cout<<"Data to store in file...."<<endl; 
printVector(strVector);


/*store run results to file of DataSet*/
//writeVectorToFile(output_perfThroughput.c_str(),strVector);
//writeVectorToFile(output_perfDelay.c_str(),strVector);
writeVectorToFile(filenameResults.c_str(),strVector);


vector<string> strVectorStats;
 strVectorStats= calculateStats(strVector);
cout<<"printing Stats"<<endl;
printVector(strVectorStats);

//writeVectorToFile(output_perfThroughputStats.c_str(),strVectorStats);
//writeVectorToFile(output_perfDelayStats.c_str(),strVectorStats);
writeVectorToFile(filenameStats.c_str(),strVectorStats);

Simulator::Destroy ();

cout<<"run parameters value"<<endl;
cout<<"nWifi : "<<nWifi<<endl;
cout<<"packetSize : "<<packetSize<<endl;
cout<<"numPackets : "<<numPackets<<endl;
cout<<"dataRate : "<<phyMode<<endl;
  
  cout<<"program done "<<endl;
  return 0;

}
