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
  ofstream file; 
  file.open(filename.c_str());

  cout<<"testing file results"<<endl; 
  cout<<filename<<endl; 
  for(uint32_t i = 0; i < strVector.size(); i++){
      file << strVector[i] << endl;
      cout<< strVector[i] << endl;
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

cout<<"string received: "<<parameter<<endl;

 for(uint32_t i=0; i< size; i++)
 {
        stringstream ss(data[i]);
        string word;

       ss>>word;
      cout<<"word: "<<" "<<word<<endl;
     cout<<"parameter: " <<parameter<<endl;

    if( parameter.compare(word) == 0)
    {
     cout<<"paramter found in data"<<endl;
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
  cout<<"index of paramter: "<<index<<endl;
  string line;


  //parameter recorded before? 
  if(vector.size()==0 || index == -1)
  {
     cout<<"first time storing parameter"<<endl;
     line = parameter;
     line.append("  ");
     line.append(result);

     vector.push_back(line);

 }
 else{
     cout<<"appending new results to parameter"<<endl;
   vector[index].append("  ");
   vector[index].append(result);

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

vector<string> calculateAvg(vector<string> data)
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

        length = data[i].length();

        float array[length];
        //float array[7]; 

        for (wordNum = 0; ss >> word; wordNum++)
        {
                if(wordNum==0){
                        parameterString=word;
                }
                else{
                        cout << " " << wordNum << "." << word<<endl;
                        array[wordNum-1]=stof(word);
               }
        }

        cout<<"float results:"<<endl;
        numbers = wordNum-1;
        for(int i=0; i<numbers; i++)
        {
                sum += array[i];
                cout<<"array["<<i<<"]: "<<array[i]<<endl;
        }
       avg= sum/numbers;
       cout<<"avg ="<<avg<<" = "<<sum<<" /"<<numbers<<endl;

      results =  addNewResults(results, parameterString, convertFloatToString(avg));


  }

 return results;


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


  fstream file; 
  ifstream infile; 
  ofstream outfile; 
  fstream file2; 
  ifstream infile2; 
  string output_perfThroughput = "./scratch/perfThroughput.txt";
  string output_perfThroughputAvg = "./scratch/perfThroughputAvg.txt";
  string output_perfThroughput2 = "./scratch/perfThroughput2.txt";
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
  int trialID = atoi(array); 
  int col = trialID; 
  int row= nSat;
  cout<<"col:"<<col<<" "<<"row:"<<row<<endl;


  float   avgThroughput;




/********************************************
*capture data from files & read number of lines
*********************************************/
strVector= fetchDataFromFile(output_perfThroughput.c_str(),&number_of_lines);
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
      	  cout << " Average Throughput: " << avgThroughput << " kbps\n";
     }

  monitor->SerializeToXmlFile("luna.flowmon", true, true);


/*store new data in data vector */
tempVec = strVector;
strVector=  addNewResults(tempVec, convertUintToString(nSat),convertFloatToString(avgThroughput));

cout<<"Data to store in file...."<<endl; 
printVector(strVector);

/*store run results to file */
writeVectorToFile(output_perfThroughput.c_str(),strVector);


vector<string> strVectorAvg;
 strVectorAvg= calculateAvg(strVector);
cout<<"printing AVG "<<endl;
printVector(strVectorAvg);

writeVectorToFile(output_perfThroughputAvg.c_str(),strVectorAvg);





  Simulator::Destroy ();







  
  cout<<"program done "<<endl;
  return 0;

}
