# Create a fully functional layer-2 Firewall application using Pyretic

## 实验要求

![](https://github.com/Indigo6/advanced-network-course/blob/main/Create%20a%20fully%20functional%20layer-2%20Firewall%20application%20using%20Pyretic/pictures/topo.png)

利用 pyretic 在如图的拓扑网络上建立防火墙

+ 补全 `pyretic_firewall.py`，完成防火墙建立的代码
+ 自定义防火墙规则并进行 `pingall` 测试
+ 进行数次 `ping` 之后（例如，h1 ping h3，h4 ping h6），打印 `pyretic_switch` 的 `self.policy`，并解释这些策略。



## 压缩包内容

+ SA20011020-方林-计网实验3报告.pdf
+ code
  + firewall-policies.csv ：防火墙规则 csv
  + pyretic_firewall.py ：读取防火墙规则 csv 并建立防火墙的代码
+ pictures
  + ping.png：数次 `ping` 查看 `self.policy`
  + pingall.png ：`pingall` 测试防火墙规则
  + policy.png ：`self.policy` 输出
  + topo.png：拓扑图



## 实验结果

### pyretic_firewall.py 解释

```python
# open firewall rules csv
csvFile = open("pyretic/examples/firewall-policies.csv", 'r')
# open csv cursor
cursor = csv.DictReader(csvFile)

# init firewall rules container
not_allowed = none
for d in cursor:
    not_allowed = not_allowed + (match(srcmac=MAC(d['mac_0'])) & match(dstmac=MAC(d['mac_1']))) + (
            match(srcmac=MAC(d['mac_1'])) & match(dstmac=MAC(d['mac_0'])))
# negate
allowed = ~not_allowed
return allowed >> act_like_switch()
```

其中关键代码是

```python
not_allowed = not_allowed + (match(srcmac=MAC(d['mac_0'])) & match(dstmac=MAC(d['mac_1']))) + (
            match(srcmac=MAC(d['mac_1'])) & match(dstmac=MAC(d['mac_0'])))
```

即根据每一行的两个 MAC 地址 `mac_0` 和 `mac_1`，根据消息的 `srcmac` 和 `dstmac` 进行 MAC 地址匹配，对于 `mac_0` 和 `mac_1` 之间发送的消息，不予转发。

### firewall rules

| id   | mac_0             | mac_1             |
| ---- | ----------------- | ----------------- |
| 1    | 00:00:00:00:00:03 | 00:00:00:00:00:02 |
| 2    | 00:00:00:00:00:03 | 00:00:00:00:00:04 |
| 3    | 00:00:00:00:00:03 | 00:00:00:00:00:06 |

+ 即在 h3 与 h2/h4/h6 之间建立防火墙



### 运行结果

首先运行 `python pyretic.py pyretic.examples.pyretic_firewall`，不要 `sudo `，会使得 `PYTHONPATH` 为空；然后 `sudo mn --topo single,6 --controller remote --mac` 建立网络

#### pingall 截图

![](https://github.com/Indigo6/advanced-network-course/blob/main/Create%20a%20fully%20functional%20layer-2%20Firewall%20application%20using%20Pyretic/pictures/pingall.png)



#### self.policy 截图

在 `pyretic/pyretic/core/runtime.py`  中加入打印日志的语句，以更好地理解

+ 第 267-270 句

  ```python
  def handle_packet_in(self, concrete_pkt):
      print "******new packet in******"
      print self.policy
      print ""
  ```


##### 进行数次 `ping`

![](https://github.com/Indigo6/advanced-network-course/blob/main/Create%20a%20fully%20functional%20layer-2%20Firewall%20application%20using%20Pyretic/pictures/ping.png)



##### 输出结果

```python
******new packet in******
sequential:
    negate:
        parallel:
            none
            sequential:
                match:
                    ('srcmac', 00:00:00:00:00:03)
                match:
                    ('dstmac', 00:00:00:00:00:02)
            sequential:
                match:
                    ('srcmac', 00:00:00:00:00:02)
                match:
                    ('dstmac', 00:00:00:00:00:03)
            sequential:
                match:
                    ('srcmac', 00:00:00:00:00:03)
                match:
                    ('dstmac', 00:00:00:00:00:04)
            sequential:
                match:
                    ('srcmac', 00:00:00:00:00:04)
                match:
                    ('dstmac', 00:00:00:00:00:03)
            sequential:
                match:
                    ('srcmac', 00:00:00:00:00:03)
                match:
                    ('dstmac', 00:00:00:00:00:06)
            sequential:
                match:
                    ('srcmac', 00:00:00:00:00:06)
                match:
                    ('dstmac', 00:00:00:00:00:03)
    [dynamic(act_like_switch)]
    parallel:
        if
            match:
                ('switch', 1)
                ('dstmac', 00:00:00:00:00:04)
        then
            fwd 4
        else
            parallel:
                if
                    match:
                        ('switch', 1)
                        ('dstmac', 00:00:00:00:00:03)
                then
                    fwd 3
                else
                    parallel:
                        if
                            match:
                                ('switch', 1)
                                ('dstmac', 00:00:00:00:00:01)
                        then
                            fwd 1
                        else
                            parallel:
                                flood on:
                                -------------------------------------------------------------------------------------
                                switch  |  switch edges    |                 egress ports                           |
                                -------------------------------------------------------------------------------------
                                1       |                  |  1[2]---, 1[3]---, 1[4]---, 1[5]---, 1[6]---, 1[1]---  |
                                -------------------------------------------------------------------------------------
                                packets
                                sequential:
                                    negate:
                                        match:
                                            ('switch', 1)
                                            ('srcmac', 00:00:00:00:00:04)
                                    negate:
                                        match:
                                            ('switch', 1)
                                            ('srcmac', 00:00:00:00:00:03)
                                    negate:
                                        match:
                                            ('switch', 1)
                                            ('srcmac', 00:00:00:00:00:01)
                                    identity
                        packets
                        sequential:
                            negate:
                                match:
                                    ('switch', 1)
                                    ('srcmac', 00:00:00:00:00:04)
                            negate:
                                match:
                                    ('switch', 1)
                                    ('srcmac', 00:00:00:00:00:03)
                            negate:
                                match:
                                    ('switch', 1)
                                    ('srcmac', 00:00:00:00:00:01)
                            identity
                packets
                sequential:
                    negate:
                        match:
                            ('switch', 1)
                            ('srcmac', 00:00:00:00:00:04)
                    negate:
                        match:
                            ('switch', 1)
                            ('srcmac', 00:00:00:00:00:03)
                    negate:
                        match:
                            ('switch', 1)
                            ('srcmac', 00:00:00:00:00:01)
                    identity
        packets
        sequential:
            negate:
                match:
                    ('switch', 1)
                    ('srcmac', 00:00:00:00:00:04)
            negate:
                match:
                    ('switch', 1)
                    ('srcmac', 00:00:00:00:00:03)
            negate:
                match:
                    ('switch', 1)
                    ('srcmac', 00:00:00:00:00:01)
            identity
```

##### 结果解释

输出结果表示，取手工建立的规则与 `pyretic_switch` 学到的 `[dynamic(act_like_switch)] policy` 的并集作为转发规则。

+ 手工建立的规则：

  ```python
  negate:
          parallel:
              none
              sequential:
                  match:
                      ('srcmac', 00:00:00:00:00:03)
                  match:
                      ('dstmac', 00:00:00:00:00:02)
              ···
              sequential
              	···
  ```

  对于每个 `sequential policy`，匹配源与目的 MAC 地址，若匹配，则 采用`none policy` 即 `drop` 

+ 如与手工规则都不匹配，则根据目的 MAC 地址转发到相应的端口



#### Flow Table

```python
mininet@mininet:~$ sudo ovs-ofctl dump-flows s1
NXST_FLOW reply (xid=0x4):
 cookie=0x0, duration=450.027s, table=0, n_packets=2, n_bytes=84, actions=CONTROLLER:65535
 cookie=0x0, duration=449.986s, table=0, n_packets=2, n_bytes=84, priority=65535,arp,in_port=3,vlan_tci=0x0000,dl_src=00:00:00:00:00:03,dl_dst=00:00:00:00:00:04,nw_src=10.0.0.3,nw_dst=10.0.0.4,arp_op=2 actions=drop
 cookie=0x0, duration=449.007s, table=0, n_packets=1, n_bytes=42, priority=65535,arp,in_port=4,vlan_tci=0x0000,dl_src=00:00:00:00:00:04,dl_dst=ff:ff:ff:ff:ff:ff,nw_src=10.0.0.4,nw_dst=10.0.0.3,arp_op=1 actions=output:3,output:1,output:2,output:5,output:6
```



