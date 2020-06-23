
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"

import ns.applications
import ns.core
import ns.internet
import ns.network
import ns.point_to_point
from ns.lte import LteHelper, PointToPointEpcHelper
from ns.internet import Ipv4StaticRoutingHelper

from ns.internet.Ipv4StaticRouting import Ipv4StaticRouting

lte_helper = LteHelper()
epc_helper = PointToPointEpcHelper()
lte_helper.SetEpcHelper(epc_helper)

pgw = epc_helper.GetPgwNode()

nodes = ns.network.NodeContainer()
nodes.Create(1)
remote_host = nodes.Get(0)
stack = ns.internet.InternetStackHelper()
stack.Install(nodes)


pointToPoint = ns.point_to_point.PointToPointHelper()
pointToPoint.SetDeviceAttribute("DataRate", ns.core.StringValue("100Gb/s"))
pointToPoint.SetChannelAttribute("Delay", ns.core.StringValue("10ms"))

devices = pointToPoint.Install(pgw, remote_host)

address = ns.internet.Ipv4AddressHelper()
address.SetBase(ns.network.Ipv4Address("10.0.0.0"), ns.network.Ipv4Mask("255.0.0.0"))
interface = address.Assign(devices)
remode_addr = interface.GetAddress(1)


routing_helper = Ipv4StaticRoutingHelper()
print(remote_host)
routing_helper.GetStaticRouting(remote_host.GetObject(Ipv4StaticRouting))

#
#
# ns.core.LogComponentEnable("UdpEchoClientApplication", ns.core.LOG_LEVEL_INFO)
# ns.core.LogComponentEnable("UdpEchoServerApplication", ns.core.LOG_LEVEL_INFO)
#
# nodes = ns.network.NodeContainer()
# nodes.Create(2)
#
# pointToPoint = ns.point_to_point.PointToPointHelper()
# pointToPoint.SetDeviceAttribute("DataRate", ns.core.StringValue("5Mbps"))
# pointToPoint.SetChannelAttribute("Delay", ns.core.StringValue("2ms"))
#
# devices = pointToPoint.Install(nodes)
#
# stack = ns.internet.InternetStackHelper()
# stack.Install(nodes)
#
# address = ns.internet.Ipv4AddressHelper()
# address.SetBase(ns.network.Ipv4Address("10.1.1.0"), ns.network.Ipv4Mask("255.255.255.0"))
#
# interfaces = address.Assign (devices);
#
# echoServer = ns.applications.UdpEchoServerHelper(9)
#
# serverApps = echoServer.Install(nodes.Get(1))
# serverApps.Start(ns.core.Seconds(1.0))
# serverApps.Stop(ns.core.Seconds(10.0))
#
# echoClient = ns.applications.UdpEchoClientHelper(interfaces.GetAddress(1), 9)
# echoClient.SetAttribute("MaxPackets", ns.core.UintegerValue(1))
# echoClient.SetAttribute("Interval", ns.core.TimeValue(ns.core.Seconds (1.0)))
# echoClient.SetAttribute("PacketSize", ns.core.UintegerValue(1024))
#
# clientApps = echoClient.Install(nodes.Get(0))
# clientApps.Start(ns.core.Seconds(2.0))
# clientApps.Stop(ns.core.Seconds(10.0))
#
# ns.core.Simulator.Run()
# ns.core.Simulator.Destroy()