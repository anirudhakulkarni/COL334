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
#ifndef TCPNEWRENOCSE_H
#define TCPNEWRENOCSE_H
#include "tcp-congestion-ops.h"

#include "ns3/tcp-socket-state.h"

namespace ns3
{

  class TcpNewRenoCSE : public TcpNewReno
  {
  public:
    static TypeId GetTypeId(void);

    TcpNewRenoCSE();
    TcpNewRenoCSE(const TcpNewRenoCSE &sock);

    ~TcpNewRenoCSE();

    std::string GetName() const;

    protected:
    virtual uint32_t SlowStart(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
    virtual void CongestionAvoidance(Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
  };

} // namespace ns3

#endif // TCPNEWRENOCSE_H
