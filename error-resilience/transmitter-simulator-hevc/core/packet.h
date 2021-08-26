/*  transmitter-simulator-hevc, version 0.1
 *  Copyright(c) 2021 Matteo Naccari
 *  All Rights Reserved.
 *
 *  email: matteo.naccari@gmail.com | matteo.naccari@polimi.it | matteo.naccari@lx.it.pt
 *
 * The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the author may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#ifndef H_PACKET_
#define H_PACKET_

#include <fstream>
#include <vector>
#include <cstdint>
#include <map>
#include "reader.h"
#include "syntax.h"

using namespace std;

constexpr uint32_t nalu_max_size = 8000000;

/*!
 *
 * \brief
 * Class modelling a packet (i.e. an Annex B NALU) belonging to the bitstream being transmitted.
 * A packet can be further categorised as a slice, picture parameter set or sequence parameter set.
 * For each different category, the class takes proper action to provide the information required by the simulator engine.
 *
 * \author
 * Matteo Naccari
*/
class Packet {
  int m_is_first_byte_stream_nalu;

  NALU m_nalu;
  map<uint32_t, ReducedPPS> m_pps_memory;
  map<uint32_t, ReducedSPS> m_sps_memory;

  //! Type of the slice contained in the packet being transmitted
  SliceType m_slice_type = SliceType::INVALID_SLICE;

  //! Allocates the memory space for a NALU
  void alloc_nalu(int buffersize);

  //! Finds the start code of an Annex B NALU
  int find_start_code(unsigned char* Buf, int zeros_in_startcode);

  void convert_to_rbsp();

public:

  //!	Packet constructor, just allocates memory for a coded packet (NALU)
  Packet() { alloc_nalu(nalu_max_size); }

  //! Packet destructor
  ~Packet() {}

  bool is_nalu_slice() { return m_nalu.is_slice(); }
  bool is_nalu_vcl() { return m_nalu.is_vcl(); }
  bool is_nalu_pps() { return m_nalu.is_pps(); }
  bool is_nalu_sps() { return m_nalu.is_sps(); }
  SliceType get_slice_type() { return m_slice_type; }
  void parse_slice_type();
  void parse_pps();
  void parse_sps();

  int get_packet(ifstream& bits);
  int write_packet(ofstream& ofs);
};

#endif