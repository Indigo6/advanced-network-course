# Datacenter Network Simulation using Mininet

## 实验要求

使用 mininet 模拟 FatTree datacenter network，实验两种实现方式

![](pictures\FatTree.png)

### With Controller

+ 画出生成树拓扑图
+ 使用 `pingall` 测试网络连通性，并记录连接失败事件
+ 记录 flows table

### Without Controller

+ 在脚本中手动建立转发规则
+ `pingall` 测试人工规则
+ 证明流量均衡分布在 FatTree 上

## 压缩包内容

+ code
  + dump_flows.sh：批量获取交换机 flows table 的脚本
  + FatTree.py: Without Controller的代码
  + get_bash.py：生成 `dump_flows.sh` 的脚本
  + simplify_flowTable.py：简化原始 flows table 数据的脚本
  + SpanningTree.py: With Controller的代码
+ flowsTable
  + WithController
    + origin：在虚拟机上运行 `sudo ovs-ofctl dump-flows` 获得的原始数据
    + simple：经过 `simplify_flowTable.py` 处理简化后的 flows table 数据
  + WithoutController：
+ pictures
  + FatTree.png：FatTree 拓扑图
  + SpanningTree.jpg：SpanningTree 拓扑图
+ topos
  + FatTree.mn：FatTree 拓扑图
  + SpanningTree.mn：SpanningTree 拓扑图

## With Controller

**注：core1-4 命名为 core1-4 的时候，pyretic_hub controller 无法与这些交换机建立连接，所以把所有交换机都命名为 s1-20**

### Spanning Tree 拓扑图

![](pictures\Spanning Tree.jpg)

### IP 地址

>     Host1: 10.0.0.2
>     Host2: 10.0.0.3
>     Host3: 10.0.1.2
>     Host4: 10.0.1.3
>     Host5: 10.1.0.2
>     Host6: 10.1.0.3
>     Host7: 10.1.1.2
>     Host8: 10.1.1.3
>     Host9: 10.2.0.2
>     Host10: 10.2.0.3
>     Host11: 10.2.1.2
>     Host12: 10.2.1.3
>     Host13: 10.3.0.2
>     Host14: 10.3.0.3
>     Host15: 10.3.1.2
>     Host16: 10.3.1.3
>     
>     Core1: 10.4.1.1
>     Core2: 10.4.1.2
>     Core3: 10.4.2.1
>     Core4: 10.4.2.2
>     Aggeration1: 10.0.2.1
>     Aggeration2: 10.0.3.1
>     Aggeration3: 10.1.2.1
>     Aggeration4: 10.1.3.1
>     Aggeration5: 10.2.2.1
>     Aggeration6: 10.2.3.1
>     Aggeration7: 10.3.2.1
>     Aggeration8: 10.3.3.1
>     Edge1: 10.0.0.1
>     Edge2: 10.0.1.1
>     Edge3: 10.1.0.1
>     Edge4: 10.1.1.1
>     Edge5: 10.2.0.1
>     Edge6: 10.2.1.1
>     Edge7: 10.3.0.1
>     Edge8: 10.3.1.1



### `pingall`结果

使用 `xshell` 连接虚拟机，开两个窗口，一个窗口运行 `sudo python FatTree.py`，网络建立之后，在另一个窗口运行 `./pyretic.py -v high pyretic.examples.pyretic_hub` 开启 `controller`.

#### 第一次 `pingall`

![](pictures\With Controller\pingall_1.jpg)

可以看出，此时转发规则还不完善，终端之间的连接有部分还没有建立

#### 第二次 `pingall`

![](pictures\With Controller\pingall_2.jpg)



### Flows Table

各交换机的 flows entry 见 `flowsTable/WithController/simple` 

`s{10/2/14/16/18/19/20}_flows_table_simple.log` 为空，合理

#### Core 层

以 `s17` 的 flows table 为例

+ 第1条

  ```
  in_port=2,dl_src=2e:e8:cc:44:85:70,dl_dst=62:17:b2:5a:1c:29,nw_src=10.1.1.3,nw_dst=10.3.0.3,actions=output:1,output:4,output:3
  ```

  + 来自 10.1.1.3 代表 Host8，发往 10.3.0.3 代表 Host14，in_port=2，应当 output:4 发送给 s15，而actions=output:1,output:4,output:3 合理


#### Aggregation 层

以 `s9` 的 flows table 为例

+ 第3条

  ```
  in_port=3,dl_src=8e:22:55:c0:b0:ec,dl_dst=e6:80:3d:f7:04:74,nw_src=10.0.1.2,nw_dst=10.3.0.2,actions=output:1,output:2
  ```

  + 来自 10.0.1.2 代表 Host3，发往 10.3.0.2 代表 Host13，in_port=4，应当 output:1 发送给 17，而actions=output:1,output:2 合理

+ 第10条

  ```
  in_port=2,dl_src=ba:41:3b:44:47:04,dl_dst=8e:6a:35:b6:38:4a,nw_src=10.0.0.2,nw_dst=10.0.1.3,actions=output:3,output:1
  ```

  + 来自 10.0.0.2 代表 Host1，发往 10.0.1.3 代表 Host4，in_port=2，应当 output:3 发送给 s2，而actions=output:3,output:1 合理

#### Edge 层

以 `s1` 的 flows table 为例

+ 第10条

  ```
  in_port=3,dl_src=a6:68:fd:df:e0:64,dl_dst=ba:41:3b:44:47:04,nw_src=10.0.0.3,nw_dst=10.0.0.2,actions=output:2,output:1
  ```

  + 来自 10.0.0.3 代表 Host1，发往 10.0.0.2 代表 Host2，in_port=3，应当 output:2 发送给 Host2，而actions=output:2,output:1 合理

+ 第9条

  ```
  in_port=2,dl_src=ba:41:3b:44:47:04,dl_dst=8e:6a:35:b6:38:4a,nw_src=10.0.0.2,nw_dst=10.0.1.3,actions=output:3,output:1
  ```

  + 来自 10.0.0.2 代表 Host2，发往 10.0.1.3 代表 Host4，in_port=2，应当 output:1 发送给 s9，而actions=output:3,output:1 合理



## Without Controller

### 转发规则

**注：一个节点上，先建立的连接对应序号靠前的端口，例如 core1 与 aggregation1 先建立连接，所以 aggregation1 的端口1 对应这条连接**

#### 首先建立 arp 转发规则

```python
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
```

#### 然后建立 ip 转发规则

```python
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
```



### 实验结果

#### 测试连通性 

使用 `pingall`  进行测试

![](pictures\Without Controller\pingall.jpg)

#### 测试流量均衡

以 `h1` 与 `h9` 之间的通讯为例，路径如下

![](pictures\With Controller\example.jpg)

各交换机的 flows entry 见 `flowsTable/WithoutController` 文件夹

+ s9, s10: n_packets 都是 20
  + s9: `cookie=0x0, duration=91.828s, table=0, n_packets=20, n_bytes=1960, ip,nw_dst=10.2.0.2 actions=output:1,output:2`
  + s10:  `cookie=0x0, duration=91.552s, table=0, n_packets=20, n_bytes=1960, ip,nw_dst=10.2.0.2 actions=output:1,output:2`
+ s17, s18, s19, s20: n_packets 都是 60
  + s17: `cookie=0x0, duration=89.518s, table=0, n_packets=60, n_bytes=5880, ip,nw_dst=10.2.0.2 actions=output:3`
  + s18: `cookie=0x0, duration=89.197s, table=0, n_packets=60, n_bytes=5880, ip,nw_dst=10.2.0.2 actions=output:3`
  + s19: `cookie=0x0, duration=88.877s, table=0, n_packets=60, n_bytes=5880, ip,nw_dst=10.2.0.2 actions=output:3`
  + s20: `cookie=0x0, duration=88.568s, table=0, n_packets=60, n_bytes=5880, ip,nw_dst=10.2.0.2 actions=output:3`
+ s13, s14: n_packets 都是 126
  + s13: `cookie=0x0, duration=90.696s, table=0, n_packets=126, n_bytes=12348, ip,nw_dst=10.2.0.2 actions=output:3`
  + s14: `cookie=0x0, duration=90.427s, table=0, n_packets=126, n_bytes=12348, ip,nw_dst=10.2.0.2 actions=output:3`

综上可得，手工构建的转发规则获得了均衡的网络流量分布
