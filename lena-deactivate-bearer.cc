/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
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
 *
 * Author: Gaurav Sathe <gaurav.sathe@tcs.com>
 */
// include wszystkich wymaganych modulow z plikow naglowkowych
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
//#include "ns3/gtk-config-store.h"

using namespace ns3;

/**
 * Sample simulation script for LTE+EPC. It instantiates one eNodeB,
 * attaches three UE to eNodeB starts a flow for each UE to  and from a remote host.
 * It also instantiates one dedicated bearer per UE
 */
 //
NS_LOG_COMPONENT_DEFINE ("BearerDeactivateExample");
int main (int argc, char *argv[])
{
  // init var as args for simulation
  uint16_t numberOfNodes = 1;
  uint16_t numberOfUeNodes = 3;
  double simTime = 1.1;
  double distance = 60.0;
  double interPacketInterval = 100;

  // Command line arguments for args parser
  CommandLine cmd (__FILE__);
  cmd.AddValue ("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfNodes);
  cmd.AddValue ("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue ("distance", "Distance between eNBs [m]", distance);
  cmd.AddValue ("interPacketInterval", "Inter packet interval [ms])", interPacketInterval);
  cmd.Parse (argc, argv); // wywolanie metody parsujacej argumenty z ktorymi uruchopmiony zosta skrypt

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> (); // stworzenie i pobranie wskanika na obiekt pomocnicznego ltehelper
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper); // przekazanie do obiektu ltehelper wskanika na epcHelper wywolujac metode SetEpcHelper

  ConfigStore inputConfig; // powolanie obiektu inputConfig na podstawie ConfigStore
  inputConfig.ConfigureDefaults ();  // wywolanie metody ConfigureDefaults ustawiajacej domyslne parametry konfiguracji

  // parse again so you can override default values from the command line
  // ponowne parsowanie arguemntow tak aby nadpisac parametry domysle
  cmd.Parse (argc, argv);

  // pobranie wskaznika na Node Pgw wywolujac metode GetPgwNode na obiekcie epcHelper
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // powolanie zmiennej loglevel, laczacej logowania funkcji, czasu na poziomie ALL
  LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);

  // wlaczenie roznych kompnentow logow z argumentem logLevel
  LogComponentEnable ("BearerDeactivateExample", LOG_LEVEL_ALL);
  LogComponentEnable ("LteHelper", logLevel);
  LogComponentEnable ("EpcHelper", logLevel);
  LogComponentEnable ("EpcEnbApplication", logLevel);
  LogComponentEnable ("EpcMmeApplication", logLevel);
  LogComponentEnable ("EpcPgwApplication", logLevel);
  LogComponentEnable ("EpcSgwApplication", logLevel);
  LogComponentEnable ("LteEnbRrc", logLevel);


  // stworzenie pojedynczego zdalnego hosta

  // powolanie kontenera na zdalne hosty wielkosci 1
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);

  // pobranie wskaznika na stworzony host ktory jest pierwszy w kontenerze z indeksem 0
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);

  // powolanie obiektu stosu sieciowego
  InternetStackHelper internet;
  // zainstalowanie kontenera zdalnego hosta na pomocniczym module stosu
  internet.Install (remoteHostContainer);

  // powolanie obiektu peer to peer na ktorym ystawiamy parametry polaczenia
  PointToPointHelper p2ph;
  // ustawienie maksymalnej predkosci lacza
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  // ustawienie wielkosci maksymalnego datagramu
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  // ustawianie czasu opoznienia na kanale
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  // zaisntalowanie urzadzen (pgw i zdalnego hosta) na mostu p2p
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  // powolanie obiektu pomocniczej warstwy ip w wersji 4
  Ipv4AddressHelper ipv4h;
  // nadanie adresu ip i maski
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  // przypisanie obiektu pomocniczego ip do urzadzenia  i pobranie zestawu interfejsow
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  // pobranie z zestawu interfejsow interfejsu o indeksie 1
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  // powolanie obiektu pomocnego odpowiadajacego za routing wartwy ip
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  // stworzenie statycznego routingu pomiedzy siecmi zdalnego hosta a siecia '7.0.0.0'
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // powolanie kontenerow przechowujacych obiekty typu enbnode i uenodes
  NodeContainer ueNodes;
  NodeContainer enbNodes;
  // stworzenie okreslonej ilosci enbnode i uenode
  enbNodes.Create (numberOfNodes);
  ueNodes.Create (numberOfUeNodes);

  // stworzenie modelu rozlokowania pozycji nodow

  // stworzenie listy pozycji
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      // dodawanie kolejnych wektorow pozycji w ktorych pierwsza wspolrzedna jest wielokrotnoscia  wartosci distance a pozostale dwie rowne zer
      positionAlloc->Add (Vector (distance * i, 0, 0));
    }
  // powolanie obiektu mobility
  MobilityHelper mobility;
  // przypisanie mu konkretnych wlasciwosci reprezenujachych model stalych pozycji
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // ustawienie pozycji kolejnych nodow
  mobility.SetPositionAllocator (positionAlloc);
  // przypisanie obiekotw enbNode i ueNodes
  mobility.Install (enbNodes);
  mobility.Install (ueNodes);

  // zainstalowanie urzadzen lte do enbNoda i ueNoda otrzymujac kontenery urzadzen sieciowych
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // zainstalowanie stosu ip na ue nodach
  internet.Install (ueNodes);

  // stworzenie kontenera interfejsow ip
  Ipv4InterfaceContainer ueIpIface;
  // przypisanie
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // przypisanie adresow ip do nodow ue i instalacja aplikacj
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      // pobranie kolejnych nodoow ue
      Ptr<Node> ueNode = ueNodes.Get (u);
      // przypisanie statycznego routingu dla ueNodu przez domyslna brame
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // na obiekcie ktorego wskaznikiem jest ltehelper przypisanie ue do enb
  lteHelper->Attach (ueLteDevs, enbLteDevs.Get (0));

  // Activate an EPS bearer on all UEs

  // dla kazdego ue Noda:
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<NetDevice> ueDevice = ueLteDevs.Get (u);
      // powolanie modulu Quality Of Service zarzadzajacego priorytetami ruchu w sieci i ustawienie jego parametrow
      GbrQosInformation qos;
      qos.gbrDl = 132;  // bit/s, considering IP, UDP, RLC, PDCP header size
      qos.gbrUl = 132;
      qos.mbrDl = qos.gbrDl;
      qos.mbrUl = qos.gbrUl;

      // stworzenie bariery eps ?????
      enum EpsBearer::Qci q = EpsBearer::GBR_CONV_VOICE;
      EpsBearer bearer (q, qos);
      bearer.arp.priorityLevel = 15 - (u + 1);
      bearer.arp.preemptionCapability = true;
      bearer.arp.preemptionVulnerability = true;
      // przypisanie bariery na wskazniku do obiektu Lte
      lteHelper->ActivateDedicatedEpsBearer (ueDevice, bearer, EpcTft::Default ());
    }


  // instalacja i start aplikacji na UE i zdalnych hostach

  // porty
  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  uint16_t otherPort = 3000;

  // kontenery aplikacji clienta i serwera
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  // dla kazdego ue's
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      // wybranie kolejnego portu
      ++ulPort;
      ++otherPort;

      // powoloanie zestawu sink'ow w modelu socketFactory opartych na modelu socketFactory
      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), otherPort));

      // przypisanie kazdego ueNoda do dlPacketSinkHelper a kolejno dodanie go go kontenera serwerow
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));
      // przypisanie kazdego zdalnego hosta do ulPacketSinkHelper a kolejno dodanie go go kontenera serwerow
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
      // przypisanie kazdego ueNoda do packetSinkHelper a kolejno dodanie go go kontenera serwerow
      serverApps.Add (packetSinkHelper.Install (ueNodes.Get (u)));

      // powolanie clienta na interfejsie ip ueNoda i porcie dlPort
      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      // przypisanie opoznien wewnetnych
      dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds (interPacketInterval)));
      // ustawienie parametry maxpackets na wartosc 1000000
      dlClient.SetAttribute ("MaxPackets", UintegerValue (1000000));

      // powolanie clienta na adresie hosta zdalnego i porcie ulPort
      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      // przypisanie opoznien wewnetnych
      ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds (interPacketInterval)));
      // ustawienie parametry maxpackets na wartosc 1000000
      ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));

      // stworzenie clienta operujacego na adresie interfejsu kolejnego ueNoda i na podanym porcie
      UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
      // przypisanie opoznien wewnetnych
      client.SetAttribute ("Interval", TimeValue (MilliSeconds (interPacketInterval)));
      // ustawienie parametry maxpackets na wartosc 1000000
      client.SetAttribute ("MaxPackets", UintegerValue (1000000));

      // instalacja
      clientApps.Add (dlClient.Install (remoteHost));
      clientApps.Add (ulClient.Install (ueNodes.Get (u)));

      // dodanie instalacji clientow na ueNodach tak aby pierwszy node byl nodem ostatnim
      if (u + 1 < ueNodes.GetN ())
        {
          clientApps.Add (client.Install (ueNodes.Get (u + 1)));
        }
      else
        {
          clientApps.Add (client.Install (ueNodes.Get (0)));
        }
    }

  // start aplikacji serwera w 30ms twania symulacji
  serverApps.Start (Seconds (0.030));
  // start aplikacji clientow w 30ms twania symulacji
  clientApps.Start (Seconds (0.030));

  // powolanie zmiennnych ktore beda parametrami pracy modulu statystyk dla uplinka i downlinka
  double statsStartTime = 0.04; // need to allow for RRC connection establishment + SRS
  double statsDuration = 1.0;

  // wlaczenie Tracowania

//  lteHelper->EnableTraces  ();
//  lteHelper->EnableLogComponents  ();
  lteHelper->EnablePhyTraces ();
  lteHelper->EnableMacTraces ();
  lteHelper->EnableRlcTraces ();
  lteHelper->EnablePdcpTraces ();

  //  p2ph.EnableAsciiAll ("log");

  AsciiTraceHelper ascii;
  p2ph.EnableAsciiAll (ascii.CreateFileStream ("log_ascii.tr"));
  p2ph.EnablePcapAll ("log");

  // pobranie wskanika na obiekt rlcStats przez wywolanie metody GetRlcStats
  Ptr<RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();

  // ustawienie parametru StartTime na statsStartTime
  rlcStats->SetAttribute ("StartTime", TimeValue (Seconds (statsStartTime)));

  // ustawienie parametru EpochDuration na statsDuration
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (statsDuration)));

  // pobranie wskaznikow na urzadenia ue i enb
  Ptr<NetDevice> ueDevice = ueLteDevs.Get (0);
  Ptr<NetDevice> enbDevice = enbLteDevs.Get (0);

  /*
   *   Instantiate De-activation using Simulator::Schedule() method which will initiate bearer de-activation after deActivateTime
   *   Instantiate De-activation in sequence (Time const &time, MEM mem_ptr, OBJ obj, T1 a1, T2 a2, T3 a3)
   */

  // powolanie obiektu deActivateTime
  Time deActivateTime (Seconds (1.5));

  // zapisanie do schedulera zdarzenia DeActivateDedicatedEpsBearer po czasie deActivateTime wynoszacym 1.5s
  Simulator::Schedule (deActivateTime, &LteHelper::DeActivateDedicatedEpsBearer, lteHelper, ueDevice, enbDevice, 2);

  //zatrzymanie symulacji po 3 sekundach pracy
  Simulator::Stop (Seconds (3.0));

  // uruchomienie petli glownej symulacj
  Simulator::Run ();
  /*GtkConfigStore config;
  config.ConfigureAttributes();*/

  // wywolanie destruktora na obiekcie symulacji
  Simulator::Destroy ();
  return 0;

}
