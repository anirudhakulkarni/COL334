/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Natale Patriciello <natale.patriciello@gmail.com>
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
 */
#include "tcp-congestion-ops.h"
#include "TcpNewRenoCSE.h"
#include "tcp-socket-base.h"
#include "ns3/log.h"
#include <math.h>
namespace ns3
{

  NS_LOG_COMPONENT_DEFINE("TcpNewRenoCSE");

  // RENO

  NS_OBJECT_ENSURE_REGISTERED(TcpNewRenoCSE);

  TypeId
  TcpNewRenoCSE::GetTypeId(void)
  {
    static TypeId tid = TypeId("ns3::TcpNewRenoCSE")
                            .SetParent<TcpNewReno>()
                            .SetGroupName("Internet")
                            .AddConstructor<TcpNewRenoCSE>();
    return tid;
  }

  TcpNewRenoCSE::TcpNewRenoCSE(void) : TcpNewReno()
  {
    NS_LOG_FUNCTION(this);
  }

  TcpNewRenoCSE::TcpNewRenoCSE(const TcpNewRenoCSE &sock)
      : TcpNewReno(sock)
  {
    NS_LOG_FUNCTION(this);
  }

  TcpNewRenoCSE::~TcpNewRenoCSE(void)
  {
  }

  uint32_t
  TcpNewRenoCSE::SlowStart(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
  {
    NS_LOG_FUNCTION(this << tcb << segmentsAcked);

    if (segmentsAcked >= 1)
    {
      tcb->m_cWnd += int(pow(tcb->m_segmentSize, 1.9) / tcb->m_cWnd);
      NS_LOG_INFO("In SlowStart, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
      return segmentsAcked - 1;
    }

    return 0;
  }

  void TcpNewRenoCSE::CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
  {
    NS_LOG_FUNCTION(this << tcb << segmentsAcked);

    if (segmentsAcked > 0)
    {
      tcb->m_cWnd += int(0.5 * tcb->m_segmentSize);
      NS_LOG_INFO("In CongAvoid, updated to cwnd " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
    }
  }

  std::string
  TcpNewRenoCSE::GetName() const
  {
    return "TcpNewRenoCSE";
  }

} // namespace ns3
