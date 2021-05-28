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
//                              c1          c2
//                             -   -     -     -  
//                            -        -          -
//             3.0Mbps       -       -     -        -        3.0Mbps
//          192.168.1.0/24  -      -          -       -    192.168.4.0/24
//                         -     -               -      -
//                        -    -                    -      -
//                       -   -                         -      -
//     Cluster1         -  -                                -    -  Clusters2
//                      a1                                         a2
//      1.0Mbps         |                          1.0Mbps          |
//   10.1.1.0/24  ==============                 10.2.1.0/24  ============== 
//                  |         |                                 |         |
//                  t1        t2                                t3        t4
//     1.0Mbps      |         |       1.0Mbps      1.0Mbps      |         |       1.0Mbps    
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
    //c1,c2  a1,a2,  t1,t2, t3,t4,  n1,n2,n3,n4, n5,n6,n7,n8
    //0,1    2,3     4,5    6,7     8,9,10,11    12,13,14,15
    if(debug)    printf("\n\n/****Begin to build nodes****/\n");
    NodeContainer Nodes;
    Nodes.Create(16);   

    // c1,a1 | c1,a2 | c2,a1 | c2,a2
    NodeContainer c2aNodes[4];
    for(i=0; i<4; i++){
        c2aNodes[i].Add(Nodes.Get(i/2));//c1, c2
        c2aNodes[i].Add(Nodes.Get(i%2+2));//a1, a2
    }

    NodeContainer a2tNodes[2];
    for(i=0; i<2; i++){
        a2tNodes[i].Add(Nodes.Get(i+2));//a1, a2
        a2tNodes[i].Add(Nodes.Get(2*i+4));//t1, t3
        a2tNodes[i].Add(Nodes.Get(2*i+5));//t2, t4
    }

    NodeContainer t2nNodes[4];
    for(i=0; i<4; i++){
        t2nNodes[i].Add(Nodes.Get(i+4));//t1, t2, t3, t4
        t2nNodes[i].Add(Nodes.Get(i*2+8));//n1, n3, n5, n7
        t2nNodes[i].Add(Nodes.Get(i*2+9));//n2, n4, n6, n8
    }

    if(debug){
        printf("/****Finish building nodes****/\n");
        printf("\n\n/****Begin to build channels****/\n");
        printf("    /****Begin to build p2p channels****/\n");
    }
    /************Building Channels*************/
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("2.5Mbps"));
    pointToPoint.SetChannelAttribute ("Delay"  , TimeValue(NanoSeconds(500)) );

    NetDeviceContainer p2pDevices[4];
    for(i=0; i<4; i++){
        p2pDevices[i] = pointToPoint.Install(c2aNodes[i]);
    }

    if(debug)   printf("    /****Begin to build csma channels****/\n");
    CsmaHelper csma[2];
    for(i=0; i<2; i++){
        if(i==0)    csma[i].SetChannelAttribute("DataRate", StringValue("2Mbps"));
        else    csma[i].SetChannelAttribute("DataRate", StringValue("1Mbps"));
        csma[i].SetChannelAttribute("Delay"   , TimeValue(NanoSeconds(500)) );
    }

    if(debug)   printf("    /****Begin to install nodes on csma channel 1****/\n");
    NetDeviceContainer a2tNodesDevices[2];
    for(i=0; i<2; i++){
        a2tNodesDevices[i] = csma[0].Install(a2tNodes[i]);
    }

    if(debug)   printf("    /****Begin to install nodes on csma channel 2****/\n");
    NetDeviceContainer t2nNodesDevices[4];
    for(i=0; i<4; i++){
        t2nNodesDevices[i] = csma[1].Install(t2nNodes[i]);
    }
    if(debug)   printf("/****Finish building channels****/\n");

    /************Installing Stack*************/
    InternetStackHelper stack;
    stack.Install (Nodes);

    /************Assigning Adresses*************/
    Ipv4AddressHelper address;
    Ipv4InterfaceContainer c2aInterfaces[4], a2tInterfaces[2], t2nInterfaces[4];

    if(debug)    printf("\n\n/****Begin to assign adresses****/\n");
    
    for(i=0; i<4; i++){
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
    //Pattern 1 :  inter-cluster traffic
    //Each server communicates using TCP with another server that comes from different cluster
    //For example, 1-5, 6-2, 3-7, 8-4
    const int sinkPort = 8080;
    const int interval = 60;
    
    ApplicationContainer sinkApp[4];
    ApplicationContainer clientApp[4];
    int begin[4] = {1, 6, 3, 8};
    int end[4] = {5, 2, 7, 4};

    if(debug)    printf("\n\n/****Begin to build listeners****/\n");
    // listen on        n5, n2, n7, n4
    // t2nInterfaces    2.1 0.2 3.1 1.2
    for (int i=0; i<4; i++) {
        PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(t2nInterfaces[(end[i]-1)/2].GetAddress(2-end[i]%2), sinkPort));
        sinkApp[i] = packetSinkHelper.Install (Nodes.Get(end[i]+7));
        sinkApp[i].Start(Seconds (0));
        sinkApp[i].Stop(Seconds (1.0 + interval));
    }
    if(debug){
        printf("/****Finish building listeners****/\n");
        printf("\n\n/****Begin to build senders****/\n");
    }
    // send from        n1, n6, n3, n8
    // t2nInterfaces    0.1 2.2 1.1 3.2
    for (int i=0; i<4; i++) {
        if(debug){
            printf("    Building no.%d client\n", i);
            printf("    IPv4 address: ");
            Ipv4Address tmp_address = t2nInterfaces[(end[i]-1)/2].GetAddress(2-end[i]%2);
            tmp_address.Print(cout);
            printf(", Port: %d\n", sinkPort);
        }
        OnOffHelper client("ns3::TcpSocketFactory",InetSocketAddress(t2nInterfaces[(end[i]-1)/2].GetAddress(2-end[i]%2),sinkPort));
        client.SetAttribute ("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=50]"));
        client.SetAttribute ("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        client.SetAttribute ("DataRate", DataRateValue (DataRate ("1.0Mbps")));
        client.SetAttribute ("PacketSize", UintegerValue (4096));
        clientApp[i] = client.Install (Nodes.Get(begin[i]+7));
        clientApp[i].Start(Seconds (1.0 ));
        clientApp[i].Stop (Seconds (interval));
    }
    if(debug)    printf("/****Finish building senders****/\n");


    if(debug)    printf("\n\n/****Begin to build route table****/\n");
    /***********Building route table**************/
    Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    if(debug)    printf("/****Finish building route table****/\n");

    /***********Enable pcap**************/
    pointToPoint.EnablePcapAll ("Pattern1_p2p_improve");
    // csma[0].EnablePcapAll ("Pattern1_csma1_improve");
    csma[1].EnablePcapAll ("Pattern1_csma2_improve");


    if(debug)    printf("\n\n/****Begin to run****/\n");
    // AnimationInterface anim("DC_Pattern1.xml");  
    Simulator::Run();
    Simulator::Destroy();
    if(debug)    printf("/****Finish running****/\n");
    return 0;
    
}




