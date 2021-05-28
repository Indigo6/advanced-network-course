#!/usr/bin/python

from mininet.net import Mininet
from mininet.node import Controller, RemoteController, OVSController
from mininet.node import CPULimitedHost, Host, Node
from mininet.node import OVSKernelSwitch, UserSwitch
from mininet.cli import CLI
from mininet.log import setLogLevel, info
from mininet.link import TCLink, Intf
from subprocess import call

def myNetwork(debug=True):

    net = Mininet( topo=None,
                   build=False,
                   ipBase='10.0.0.0/8')


    info( '*** Adding controller\n' )
    c0=net.addController(name='c0', 
                         controller=RemoteController,
                         protocol='tcp',
                         port=6633)


    info( '*** Add switches\n')
    core = []
    for i in range(4):
        core.append(net.addSwitch('s' + str(i+17), cls=OVSKernelSwitch))
    aggregations = []
    for i in range(8):
        aggregations.append(net.addSwitch('s' + str(i+9), cls=OVSKernelSwitch))
    edges = []
    for i in range(8):
        edges.append(net.addSwitch('s' + str(i+1), cls=OVSKernelSwitch))
    
    info( '*** Add hosts\n')
    hosts = []
    for i in range(16):
        address = "10.{}.{}.{}".format(i//4, (i%4)//2, (i%4)%2+2) # host9, 8//4=2, (i%4)//2=0, (i%4)%2)=0
        if debug:
            info("      Host{}: {}\n".format(i+1, address))
        hosts.append(net.addHost('h' + str(i+1), cls=Host, ip=address, defaultRoute=None))


    info( '*** Add links\n')
    for i in range(4):
        if debug:
            info("      Linking   {}-{}\n".format(core[i], aggregations[i//2]))
            info("      Linking   {}-{}\n".format(core[i], aggregations[2+i//2]))
            info("      Linking   {}-{}\n".format(core[i], aggregations[4+i//2]))
            info("      Linking   {}-{}\n".format(core[i], aggregations[6+i//2]))
        net.addLink(core[i], aggregations[i//2])
        net.addLink(core[i], aggregations[2+i//2])
        net.addLink(core[i], aggregations[4+i//2])
        net.addLink(core[i], aggregations[6+i//2])
    for i in range(8):
        net.addLink(aggregations[i], edges[i-i%2])
        net.addLink(aggregations[i], edges[i+1-i%2])
        if debug:
            info("      Linking   {}-{}\n".format(aggregations[i], edges[i-i%2]))
            info("      Linking   {}-{}\n".format(aggregations[i], edges[i+1-i%2]))
    for i in range(8):
        if debug:
            info("      Linking   {}-{}\n".format(edges[i], hosts[2*i]))
            info("      Linking   {}-{}\n".format(edges[i], hosts[2*i+1]))
        net.addLink(edges[i], hosts[2*i])
        net.addLink(edges[i], hosts[2*i+1])


    info( '*** Starting network\n')
    net.build()
    info( '*** Starting controllers\n')
    for controller in net.controllers:
        controller.start()


    info( '*** Starting switches\n')
    for i in range(1,21):
        net.get('s'+str(i)).start([c0])


    info( '*** Post configure switches and hosts\n')
    
    for i in range(4):
        address = "10.4.{}.{}".format(i//2+1, i%2+1) # core4, 3//2+1=2, 3%2+1=2
        core[i].cmd('ifconfig c{} {}'.format(i+1, address))
        if debug:
            info("      Core{}: {}\n".format(i+1, address))
    for i in range(8):
        address = "10.{}.{}.1".format(i//2, i%2+2) # aggeration5, 4//2=2, 4%2+2=2
        aggregations[i].cmd('ifconfig a{} {}'.format(i+1, address))
        if debug:
            info("      Aggeration{}: {}\n".format(i+1, address))
    for i in range(8):
        edges[i].cmd('ifconfig e{} {}'.format(i+1, address))
        address = "10.{}.{}.1".format(i//2, i%2) # edge5, 4//2=2, 4%2=0
        if debug:
            info("      Edge{}: {}\n".format(i+1, address))



    #building flow entry arp
    if debug:
        info("\n*** Building edge flow entry arp\n")
    for i in range(1,21):
        net.get("s"+str(i)).cmd("sudo ovs-ofctl add-flow s{} arp,in_port=1,actions=output:{}".format(i, "2,3,4" if i>=17 else "3,4"))
        net.get("s"+str(i)).cmd("sudo ovs-ofctl add-flow s{} arp,in_port=2,actions=output:{}".format(i, "1,3,4" if i>=17 else "3,4"))
        net.get("s"+str(i)).cmd("sudo ovs-ofctl add-flow s{} arp,in_port=3,actions=output:1,2,4".format(i))
        net.get("s"+str(i)).cmd("sudo ovs-ofctl add-flow s{} arp,in_port=4,actions=output:1,2,3".format(i))
        if debug:
            info("      sudo ovs-ofctl add-flow s{} arp,in_port=1,actions=output:{}\n".format(i, "2,3,4" if i>=17 else "3,4"))
            info("      sudo ovs-ofctl add-flow s{} arp,in_port=2,actions=output:{}\n".format(i, "1,3,4" if i>=17 else "3,4"))
            info("      sudo ovs-ofctl add-flow s{} arp,in_port=3,actions=output:1,2,4\n".format(i))
            info("      sudo ovs-ofctl add-flow s{} arp,in_port=4,actions=output:1,2,3\n\n".format(i))

            

    # edge flow entry ip
    if debug:
        info("*** edge flow entry ip\n")
    for i in range(8):
        for j in range(16):
            address = "10.{}.{}.{}".format(j//4, (j%4)//2, (j%4)%2+2) # host9, 8//4=2, (i%4)//2=0, (i%4)%2)=0
            if j//2 == i:   # host dst in edge 
                edges[i].cmd("sudo ovs-ofctl add-flow s{} ip,nw_dst={},actions=output:{}".format(i+1, address, 3+(j%4)%2))
                if debug:
                    info("      sudo ovs-ofctl add-flow s{} ip,nw_dst={},actions=output:{}\n".format(i+1, address, 3+(j%4)%2))
            else:           # host dst out of edge 
                edges[i].cmd("sudo ovs-ofctl add-flow s{} ip,nw_dst={},actions=output:1,output:2".format(i+1, address))
                if debug:
                    info("      sudo ovs-ofctl add-flow s{} ip,nw_dst={},actions=output:1,output:2\n".format(i+1, address))
    
    # aggregation flow entry ip
    if debug:
        info("*** aggregation flow entry ip\n")
    for i in range(8):
        for j in range(16):
            address = "10.{}.{}.{}".format(j//4, (j%4)//2, (j%4)%2+2) # host9, 8//4=2, (i%4)//2=0, (i%4)%2)=0
            if i == j//4*2 or i == j//4*2 + 1:   # host dst in aggregation
                aggregations[i].cmd("sudo ovs-ofctl add-flow s{} ip,nw_dst={},actions=output:{}".format(i+9, address, 3+(j%4)//2))
                if debug:
                    info("      sudo ovs-ofctl add-flow s{} ip,nw_dst={},actions=output:{}\n".format(i+9, address, 3+(j%4)//2))
            else:           # host dst in aggregation
                aggregations[i].cmd("sudo ovs-ofctl add-flow s{} ip,nw_dst={},actions=output:1,output:2".format(i+9, address))
                if debug:
                    info("      sudo ovs-ofctl add-flow s{} ip,nw_dst={},actions=output:1,output:2\n".format(i+9, address))
    
    # core flow entry ip
    if debug:
        info("*** core flow entry ip\n")
    for i in range(4):
        for j in range(16):
            address = "10.{}.{}.{}".format(j//4, (j%4)//2, (j%4)%2+2) # host9, 8//4=2, (i%4)//2=0, (i%4)%2)=0
            core[i].cmd("sudo ovs-ofctl add-flow s{} ip,nw_dst={},actions=output:{}".format(i+17, address, 1+j//4))
            if debug:
                info("      sudo ovs-ofctl add-flow s{} ip,nw_dst={},actions=output:{}\n".format(i+17, address, 1+j//4))

    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel( 'info' )
    myNetwork()
