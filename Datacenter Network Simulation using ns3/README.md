# Datacenter Network Simulation using ns3

## 实验要求

使用 ns-3 模拟 Datacenter network ，实验两种网络流量模式，从中发现网络的瓶颈，并改善

### 网络拓扑结构

![image-20210522162543900](https://github.com/Indigo6/advanced-network-course/blob/main/Datacenter%20Network%20Simulation%20using%20ns3/pictures\origin.png)

+ All the end-end delays on the networks are 500ns.

## 压缩包内容

+ pcap：抓包结果存放的文件夹

  > 名称为 example-<Node编号>-<NetDevice编号>.pcap 命名格式

+ pictures：

  + new1.jpg：Pattern1 改进后的网络拓扑图
  + new2.jpg：Pattern2 改进后的网络拓扑图
  + Pattern*(_improve)：根据抓包分析所得的通道吞吐量图存放的文件夹

+ code
  
  + dc_inter-cluster.cc：Pattern1 原始网络的代码
  + dc_many-to-one.cc：Pattern2 原始网络的代码
  + dc_inter-cluster_improve.cc：Pattern1 改进后网络的代码
  + dc_many-to-one_improve.cc：Pattern2 改进后网络的代码

## 原始网络
### 网络构建

一次性创建所有的 15 个节点，然后将他们加入到不同组中，并在这些组上进行后续的网卡与地址安装

```c++
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
```

### 实验结果（瓶颈分析）
通过分析不同通道的吞吐量，来找出瓶颈

#### Pattern1

n1->n5, n6->n2, n3->n7, n8->n4 分别发起一条到TCP流。每条流均从0秒开始，60秒结束，测试时间持续60秒。

1. Core 到 Aggeration 层吞吐量，以 a1-c1 p2p 通道为例

   ![Pattern1-p2p-1-1](.\pictures\Pattern1\Pattern1-p2p-0-0.jpg)

2. Aggeration 到 ToR 层吞吐量，以 a1-t1-t2 ethernet 为例

   ![Pattern1-csma2-1-2](pictures\Pattern1\Pattern1-csma2-1-1.jpg)

3. ToR 与 Server 层吞吐量，以 t4-n7-n8 ethernet 通道为例

   ![Pattern1-csma2-14-0](pictures\Pattern1\Pattern1-csma2-7-0.jpg)

可见 ToR 与 Server 层 ethernet 通道的带宽完全没有利用起来，而p2p 和 Aggeration 到 ToR 层 ethernet 通道的带宽几乎发挥到极限，所以瓶颈在于Aggeration 到 ToR 层 ethernet 和 Core 到 Aggeration 层的 p2p 通道。

#### Pattern2

n2, n3, n4, n5, n6, n7, n8 分别发起一条到 n1 的TCP流。每条流均从0秒开始，60秒结束，测试时间持续60秒。

1. Core 到 Aggeration 层吞吐量，以 a1-c1 p2p 通道为例

   ![Pattern1-p2p-1-1](pictures\Pattern2\Pattern2-p2p-0-0.jpg)

2. Aggeration 到 ToR 层吞吐量，以 a1-t1-t2 ethernet 为例

   ![Pattern-csma2-1-2](pictures\Pattern2\Pattern-csma2-1-1.jpg)

3. t1-n1-n2 ethernet 通道吞吐量

   ![Pattern1-p2p-1-1](pictures\Pattern2\Pattern2-csma2-7-0.jpg)

4. t4-n7-n8 ethernet 通道吞吐量

   ![Pattern1-csma2-14-0](D:\Learn me\USTC\研究生\2021 春季\2021春季 高级计算机网络\实验\实验1\实验报告\pictures\Pattern2\Pattern2-csma2-14-0.jpg)

可见p2p和 a1-t1-t2 ethernet 通道带宽占用低，t4-n7-n8 ethernet 通道的带宽完全没有利用起来，而 t1-n1-n2 ethernet 通道的带宽几乎发挥到极限，所以瓶颈在于 t1-n1-n2 ethernet 通道。

## 优化数据中心拓扑结构

### Pattern1

#### 改进方法

1. 增加 Core 并增加 p2p 通道的带宽

   ```c++
   Nodes.Create(16);   
   
       // c1,a1 | c1,a2 | c2,a1 | c2,a2
       NodeContainer c2aNodes[4];
       for(i=0; i<4; i++){
           c2aNodes[i].Add(Nodes.Get(i/2));//c1, c2
           c2aNodes[i].Add(Nodes.Get(i%2+2));//a1, a2
       }
   
       PointToPointHelper pointToPoint;
       pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("2.5Mbps"));
       pointToPoint.SetChannelAttribute ("Delay"  , TimeValue(NanoSeconds(500)) );
   ```

2. 增加 Core和Aggregation层之间的带宽

   ```c++
   for(i=0; i<2; i++){
           if(i==0)    csma[i].SetChannelAttribute("DataRate", StringValue("2Mbps"));	// csma[0]: Aggeration to ToR
           else    csma[i].SetChannelAttribute("DataRate", StringValue("1Mbps"));	// csma[1]: ToR to Servers
           csma[i].SetChannelAttribute("Delay"   , TimeValue(NanoSeconds(500)) );
       }
   ```

3. 开启 ECMP 路由

   ```c++
   Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
   ```

#### 改进后拓扑图

![new](pictures\new1.jpg)

#### 改进后TCP性能

1. Core 到 Aggeration 层吞吐量，以 a1-c1 p2p 通道为例

   ![Pattern1-improve-p2p-0-0](D:\Learn me\USTC\研究生\2021 春季\2021春季 高级计算机网络\实验\实验1\实验报告\pictures\Pattern1_improve\Pattern1-improve-p2p-0-0.jpg)

2. Aggeration 到 ToR 层吞吐量，以 a1-t1-t2 ethernet 为例

   ![Pattern1-improve-csma2-2-2](pictures\Pattern1_improve\Pattern1-improve-csma2-2-2.jpg)

3. ToR 与 Server 层吞吐量，以 t4-n7-n8 ethernet 通道为例

   ![Pattern1-improve-csma2-15-0](pictures\Pattern1_improve\Pattern1-improve-csma2-15-0.jpg)

可见 t4-n7-n8 ethernet 通道的吞吐量提高了一倍，而 p2p 和 Aggeration 到 ToR 层 ethernet 通道的带宽占用降低了许多，不再是瓶颈。

### Pattern2

#### 改进方法

增加 t1-n1-n2 ethernet 的带宽，同时增加 Aggeration to Tor ethernet 的带宽

```c++
    CsmaHelper csma[3]; 
    for(i=0; i<2; i++){
        if(i==0)    csma[i].SetChannelAttribute("DataRate", StringValue("2Mbps"));  // csma[0]: Aggeration to ToR
        else if(i==1)    csma[i].SetChannelAttribute("DataRate", StringValue("1Mbps"));	 // csma[1]: ToR to Servers
        else    csma[i].SetChannelAttribute("DataRate", StringValue("4Mbps"));  // csma[2]: ethernet for t1,n1,n2
        csma[i].SetChannelAttribute("Delay"   , TimeValue(NanoSeconds(500)) );
    }
```
开启 ECMP 路由的代码同 Pattern1

#### 改进后拓扑图

![new](pictures\new2.jpg)

#### 改进后TCP性能

1. Core 到 Aggeration 层吞吐量，以 a1-c1 p2p 通道为例

   ![Pattern2-improve-p2p-1-1](pictures\Pattern2_improve\Pattern2-improve-p2p-0-0.jpg)

2. Aggeration 到 ToR 层吞吐量，以 a1-t1-t2 ethernet 为例

   ![Pattern2-improve-csma2-1-2](pictures\Pattern2_improve\Pattern2-improve-csma2-1-1.jpg)

3. t1-n1-n2 ethernet 通道吞吐量

   ![Pattern2-improve-csma2-7-0](pictures\Pattern2_improve\Pattern2-improve-csma2-7-0.jpg)

4. t4-n7-n8 ethernet 通道吞吐量

   ![Pattern2-improve-csma2-14-0](pictures\Pattern2_improve\Pattern2-improve-csma2-14-0.jpg)

可见 t4-n7-n8 和 a1-t1-t2 ethernet 通道的带宽仍然没有利用起来，n1 ethernet 通道的带宽仍然占用率很高，所以 t1-n1-n2 ethernet 通道可能仍为瓶颈。而 p2p 通道的占用变得很高，所以此时 p2p 通道也成为了瓶颈 。

