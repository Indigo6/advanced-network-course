#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/tcp-l4-protocol.h"

#include <ostream>
#include "ns3/ipv4-address.h"
#include "ns3/netanim-module.h"

//8 servers: n1~8; 4 ToR switches: t1~4;
//2 aggregation switches: a1~2; 1 core switch: c1
//The network is partitioned/ into two clusters
//
//                                          c1
//                                       -     -  
//                                     -          -
//                 1.5Mbps           -              -       1.5Mbps
//              192.168.1.0/24     -                  -  192.168.2.0/24
//                               -                      -
//                             -                           -
//                           -                                -
//     Cluster1            -                       clusters2     -
//                      a1                                         a2
//      2.0Mbps         |                          2.0Mbps          |
//   10.1.1.0/24  ==============                 10.2.1.0/24  ============== 
//                  |         |                                 |         |
//                  t1        t2                                t3        t4
//     2.0Mbps      |         |      1.0Mbps      1.0Mbps      |         |       1.0Mbps    
//   10.0.1.0/24 ========  ======== 10.0.2.0/24  10.0.3.0/24 ========  ======== 10.0.4.0/24  
//                |    |    |    |                            |    |    |    |
//                n1   n2   n3   n4                           n5   n6   n7   n8


using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("DataCenter_Network");

int main(int argc, char *argv[])
{
    int i;
    bool verbose = true;
    bool debug = true;

    CommandLine cmd;
    
    cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
    cmd.AddValue ("debug", "Log program if true", debug);

    cmd.Parse (argc,argv);
    
    if (verbose){
        LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    /************Building Nodes*************/
    //c1,  a1,a2,  t1,t2, t3,t4,  n1,n2,n3,n4, n5,n6,n7,n8
    //0,    1,2     3,4    5,6     7,8,9,10    11,12,13,14
    if(debug)    printf("\n\n/****Begin to build nodes****/\n");
    NodeContainer Nodes;
    Nodes.Create(15);   

    NodeContainer c2aNodes[2];
    for(i=0; i<2; i++){
        c2aNodes[i].Add(Nodes.Get(0));//c1
        c2aNodes[i].Add(Nodes.Get(i+1));//a1, a2
    }

    NodeContainer a2tNodes[2];
    for(i=0; i<2; i++){
        a2tNodes[i].Add(Nodes.Get(i+1));//a1, a2
        a2tNodes[i].Add(Nodes.Get(2*i+3));//t1, t3
        a2tNodes[i].Add(Nodes.Get(2*i+4));//t2, t4
    }

    NodeContainer t2nNodes[4];
    for(i=0; i<4; i++){
        t2nNodes[i].Add(Nodes.Get(i+3));//t1, t2, t3, t4
        t2nNodes[i].Add(Nodes.Get(i*2+7));//n1, n3, n5, n7
        t2nNodes[i].Add(Nodes.Get(i*2+8));//n2, n4, n6, n8
    }

    if(debug){
        printf("/****Finish building nodes****/\n");
        printf("\n\n/****Begin to build channels****/\n");
        printf("    /****Begin to build p2p channels****/\n");
    }
    /************Building Channels*************/
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1.5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay"  , TimeValue(NanoSeconds(500)) );

    NetDeviceContainer p2pDevices[2];
    for(i=0; i<2; i++){
        p2pDevices[i] = pointToPoint.Install(c2aNodes[i]);
    }

    if(debug)   printf("    /****Begin to build csma channels****/\n");
    CsmaHelper csma[3]; // csma[2]: ethernet for t1,n1,n2
    for(i=0; i<2; i++){
        if(i==0)    csma[i].SetChannelAttribute("DataRate", StringValue("2Mbps"));
        else if(i==1)    csma[i].SetChannelAttribute("DataRate", StringValue("1Mbps"));
        else    csma[i].SetChannelAttribute("DataRate", StringValue("4Mbps"));
        csma[i].SetChannelAttribute("Delay"   , TimeValue(NanoSeconds(500)) );
    }

    if(debug)   printf("    /****Begin to install nodes on csma channel 1****/\n");
    NetDeviceContainer a2tNodesDevices[2];
    for(i=0; i<2; i++){
        a2tNodesDevices[i] = csma[0].Install(a2tNodes[i]);
    }

    if(debug)   printf("    /****Begin to install nodes on csma channel 2****/\n");
    NetDeviceContainer t2nNodesDevices[4];
    t2nNodesDevices[0] = csma[2].Install(t2nNodes[0]);
    for(i=1; i<4; i++){
        t2nNodesDevices[i] = csma[1].Install(t2nNodes[i]);
    }
    if(debug)   printf("/****Finish building channels****/\n");

    /************Installing Stack*************/
    InternetStackHelper stack;
    stack.Install (Nodes);

    /************Assigning Adresses*************/
    Ipv4AddressHelper address;
    Ipv4InterfaceContainer c2aInterfaces[2], a2tInterfaces[2], t2nInterfaces[4];

    if(debug)    printf("\n\n/****Begin to assign adresses****/\n");
    
    for(i=0; i<2; i++){
        char base_addr[30];
        sprintf(base_addr, "192.168.%d.0", i+1);
        address.SetBase(base_addr,"255.255.255.0");
        c2aInterfaces[i] = address.Assign(p2pDevices[i]);
    }

    
    for(i=0; i<2; i++){
        char base_addr[30];
        sprintf(base_addr, "10.1.%d.0", i+1);
        address.SetBase(base_addr,"255.255.255.0");
        a2tInterfaces[i] = address.Assign(a2tNodesDevices[i]);
    }
    for(i=0; i<4; i++){
        char base_addr[30];
        sprintf(base_addr, "10.0.%d.0", i+1);
        address.SetBase(base_addr,"255.255.255.0");
        t2nInterfaces[i] = address.Assign(t2nNodesDevices[i]);
    }
    if(debug)    printf("/****Finish assigning addresses****/\n");


    /**************Building Applications**************/
    //Pattern 2 :  many_to_one traffic
    //Select one server as the sink, and all the other servers communicate to it
    //For example : select n1 as sinkApp, the other servers communicate to it
    const int sinkPort = 8080;
    const int interval = 60;
    
    ApplicationContainer sinkApp;
    ApplicationContainer clientApp[4];
 
    if(debug)    printf("\n\n/****Begin to build listeners****/\n");   
    // listen on n1
    PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(t2nInterfaces[0].GetAddress(1), sinkPort));
    sinkApp = packetSinkHelper.Install (Nodes.Get(7));
    sinkApp.Start(Seconds (0));
    sinkApp.Stop(Seconds (1.0 + interval));
    if(debug){
        printf("/****Finish building listeners****/\n");
        printf("\n\n/****Begin to build senders****/\n");
    }

    // send from  n2, n3, n4, n5, n6, n7, n8
    for (int i=0; i<7; i++) {
        if(debug){
            printf("    Building no.%d client\n", i);
            printf("    IPv4 address: ");
            Ipv4Address tmp_address = t2nInterfaces[0].GetAddress(1);
            tmp_address.Print(cout);
            printf(", Port: %d\n", sinkPort);
        }
        OnOffHelper client("ns3::TcpSocketFactory",InetSocketAddress(t2nInterfaces[0].GetAddress(1), sinkPort));
        client.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=50]"));
        client.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        client.SetAttribute ("DataRate", DataRateValue (DataRate ("1.0Mbps")));
        client.SetAttribute ("PacketSize", UintegerValue (4096));
        clientApp[i] = client.Install (Nodes.Get(i+2+6));
        clientApp[i].Start(Seconds (1.0 ));
        clientApp[i].Stop (Seconds (interval));
    }
    if(debug)    printf("/****Finish building senders****/\n");


    if(debug)    printf("\n\n/****Begin to build route table****/\n");
    /***********Building route table**************/
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    if(debug)    printf("/****Finish building route table****/\n");

    /***********Enable pcap**************/
    pointToPoint.EnablePcapAll ("Pattern2_p2p_improve");
    // csma[0].EnablePcapAll ("Pattern2_csma1_improve");
    csma[1].EnablePcapAll ("Pattern2_csma2_improve");


    if(debug)    printf("\n\n/****Begin to run****/\n");
    // AnimationInterface anim("DC_Pattern2.xml");  
    Simulator::Run();
    Simulator::Destroy();
    if(debug)    printf("/****Finish running****/\n");
    return 0;
    
}
