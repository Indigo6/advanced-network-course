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
        adrress = "10.{}.{}.{}".format(i//4, (i%4)//2, (i%4)%2+2) # host9, 8//4=2, (i%4)//2=0, (i%4)%2)=0
        if debug:
            info("      Host{}: {}\n".format(i+1, adrress))
        hosts.append(net.addHost('h' + str(i+1), cls=Host, ip=adrress, defaultRoute=None))


    info( '*** Add links\n')
    for i in [0]:
        if debug:
            info("      Linking   {}-{}\n".format(core[i], aggregations[i//2]))
            info("      Linking   {}-{}\n".format(core[i], aggregations[2+i//2]))
            info("      Linking   {}-{}\n".format(core[i], aggregations[4+i//2]))
            info("      Linking   {}-{}\n".format(core[i], aggregations[6+i//2]))
        net.addLink(core[i], aggregations[i//2])
        net.addLink(core[i], aggregations[2+i//2])
        net.addLink(core[i], aggregations[4+i//2])
        net.addLink(core[i], aggregations[6+i//2])
    for i in range(0,8,2):
        if debug:
            info("      Linking   {}-{}\n".format(aggregations[i], edges[i]))
            info("      Linking   {}-{}\n".format(aggregations[i], edges[i+1]))
        net.addLink(aggregations[i], edges[i])
        net.addLink(aggregations[i], edges[i+1])
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
        adrress = "10.4.{}.{}".format(i//2+1, i%2+1) # core4, 3//2+1=2, 3%2+1=2
        core[i].cmd('ifconfig c{} {}'.format(i+1, adrress))
        if debug:
            info("      Core{}: {}\n".format(i+1, adrress))
    for i in range(8):
        adrress = "10.{}.{}.1".format(i//2, i%2+2) # aggeration5, 4//2=2, 4%2+2=2
        aggregations[i].cmd('ifconfig a{} {}'.format(i+1, adrress))
        if debug:
            info("      Aggeration{}: {}\n".format(i+1, adrress))
    for i in range(8):
        edges[i].cmd('ifconfig e{} {}'.format(i+1, adrress))
        adrress = "10.{}.{}.1".format(i//2, i%2) # edge5, 4//2=2, 4%2=0
        if debug:
            info("      Edge{}: {}\n".format(i+1, adrress))


    CLI(net)
    net.stop()

if __name__ == '__main__':
    setLogLevel( 'info' )
    myNetwork()
