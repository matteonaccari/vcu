/*  transmitter-simulator-avc, version 0.2
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

#include <iostream>
#include <fstream>
#include <vector>

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <netinet/in.h>
#endif

using namespace std;

constexpr uint32_t nalu_max_size = 8000000;

enum class SliceType
{
  P_SLICE = 0,
  B_SLICE,
  I_SLICE,
  SP_SLICE,
  SI_SLICE
};

enum class NaluType
{
  NALU_TYPE_SLICE = 1,
  NALU_TYPE_DPA = 2,
  NALU_TYPE_DPB = 3,
  NALU_TYPE_DPC = 4,
  NALU_TYPE_IDR = 5,
  NALU_TYPE_SEI = 6,
  NALU_TYPE_SPS = 7,
  NALU_TYPE_PPS = 8,
  NALU_TYPE_AUD = 9,
  NALU_TYPE_EOSEQ = 10,
  NALU_TYPE_EOSTREAM = 11,
  NALU_TYPE_FILL = 12,
  NALU_TYPE_PREFIX = 14,
  NALU_TYPE_SUB_SPS = 15,
  NALU_TYPE_SLC_EXT = 20,
  NALU_TYPE_VDRD = 24  // View and Dependency Representation Delimiter NAL Unit
};

/*!
 *
 * \brief
 * Structure corresponding to a Network Abstraction Layer Unit (NALU) as specified in Annex B of the standard
 *
 * \author
 * Matteo Naccari
*/
struct NALU
{
  int startcodeprefix_len; //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
  unsigned len;            //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
  unsigned max_size;       //! Nal Unit Buffer size
  NaluType nal_unit_type;  //! NALU_TYPE
  int nal_reference_idc;   //! NALU_PRIORITY
  int forbidden_bit;       //! Should be always FALSE
  vector<uint8_t> buf;     //! Contains the first byte followed by the EBSP

  bool is_nalu_vcl()
  {
    return int(nal_unit_type) <= int(NaluType::NALU_TYPE_IDR);
  }
};

/*!
 *
 * \brief
 * The Packet which models a coded packet corresponding to the bitstream being transmitted
 * This class will be specialized into the Rtp_packet and AnnexB_packet classes in order
 * to tackle different bitstream packetizations
 *
 * \author
 * Matteo Naccari
*/
class Packet
{

public:
  NALU m_nalu;
  //! Offset in bit units inside the current packet. It will serve for exp-Golomb decoding
  int m_frame_bitoffset;

  //! Type of the slice contained in the packet being transmitted
  SliceType m_slice_type;

  //! It allocates the memory space for a NALU
  void alloc_nalu(int buffersize);

  void decode_slice_type();

  //! Performs exponential-Golomb decoding with unsigned direct mapping of the VLC codeword
  int exp_golomb_decoding(uint8_t* buffer);

  //! Packet constructor, it just allocates memory for a coded packet (NALU).
  //! Further operation will follow in the class' specializations
  Packet() { alloc_nalu(8000000); }

  //! Packet destructor
  ~Packet() {}

  bool is_nalu_vcl() { return m_nalu.is_nalu_vcl(); }
  SliceType get_slice_type() { return m_slice_type; }

  //! The following functions will be implemented in the class' specialisations
  virtual int get_packet(ifstream& ifs) = 0;
  virtual int write_packet(ofstream& ofs) = 0;
};

/*!
 *
 * \brief
 * The Real-time Transfer Protocol (RTP) specialization of the Packet class
 *
 * \author
 * Matteo Naccari
*/

class RtpPacket : public Packet
{

private:
#define MAXRTPPACKETSIZE  (65536 - 28)
#define H26LPAYLOADTYPE 105
#define H264SSRC 0x12345678               //!< SSRC, chosen to simplify debugging
#define H264PAYLOADTYPE 105               //!< RTP paylaod type fixed here for simplicity
  typedef struct
  {
    unsigned int v;          //!< Version, 2 bits, MUST be 0x2
    unsigned int p;          //!< Padding bit, Padding MUST NOT be used
    unsigned int x;          //!< Extension, MUST be zero
    unsigned int cc;         //!< CSRC count, normally 0 in the absence of RTP mixers
    unsigned int m;          //!< Marker bit
    unsigned int pt;         //!< 7 bits, Payload Type, dynamically established
    unsigned int seq;        //!< RTP sequence number, incremented by one for each sent packet
    unsigned int old_seq;    //!< to detect wether packets were lost
    unsigned int timestamp;  //!< timestamp, 27 MHz for H.264
    unsigned int ssrc;       //!< Synchronization Source, chosen randomly
    vector<uint8_t> payload; //!< the payload including payload headers
    unsigned int paylen;     //!< length of payload in bytes
    vector<uint8_t> packet;  //!< complete packet including header and payload
    unsigned int packlen;    //!< length of packet, typically paylen+12
  } RtpData;

  RtpData m_rtp_data;

  int current_rtp_sequence_number, current_rtp_time_stamp;

  int decompose_rtp_packet();

  int rtp_read_packet(ifstream& ifs);

  void dump_rtp_header();

  void allocate_rtp_packet();

  int compose_rtp_packet();

  int write_rtp_packet(ofstream& ofs);

public:
  RtpPacket()
  {
    allocate_rtp_packet();
    current_rtp_sequence_number = 0;
    current_rtp_time_stamp = 0;
  }

  ~RtpPacket() {}

  int get_packet(ifstream& ifs);

  int write_packet(ofstream& ofs);
};

/*!
 *
 * \brief
 * The Annex B specialization of the Packet class
 *
 * \author
 * Matteo Naccari
*/

class AnnexBPacket : public Packet
{

private:
  int m_is_first_byte_stream_nalu;
  int find_start_code(unsigned char* buf, int zeros_in_startcode);

public:
  AnnexBPacket() { m_is_first_byte_stream_nalu = 1; }
  int get_packet(ifstream& ifs);
  int write_packet(ofstream& ofs);
};

#endif
