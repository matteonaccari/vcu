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

#include <stdio.h>
#include <iostream>
#include <assert.h>
#ifdef _WIN32
#include <Winsock2.h>
#else
#include <netinet/in.h>
#endif

using namespace std;
typedef unsigned char byte;
typedef enum
{
  P_SLICE = 0,
  B_SLICE,
  I_SLICE,
  SP_SLICE,
  SI_SLICE
} SliceType;

/*!
 *
 *	\brief
 *	The Packet which models a coded packet corresponding to the bitstream being transmitted
 *	This class will be specialized into the Rtp_packet and AnnexB_packet classes in order
 *	to tackle different bitstream packetizations
 *
 *	\author
 *	Matteo Naccari
*/

class Packet {

public:
  FILE* bits;
  //!	Structure corresponding to a Network Abstraction Layer Unit (NALU)
  struct NALU_t {
    int startcodeprefix_len; //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    unsigned len;            //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    unsigned max_size;       //! Nal Unit Buffer size
    int nal_unit_type;       //! NALU_TYPE_xxxx
    int nal_reference_idc;   //! NALU_PRIORITY_xxxx
    int forbidden_bit;       //! Should be always FALSE
    byte* buf;               //! Contains the first byte followed by the EBSP
  };

  NALU_t* nalu;
  //! Offset in bit units inside the current packet. It will serve for exp-Golomb decoding
  int frame_bitoffset;

  //! Type of the slice contained in the packet being transmitted
  SliceType slice_type;

  //! It allocates the memory space for a NALU
  void AllocNALU(int buffersize);

  void decode_slice_type();

  //! It performs exponential-Golomb decoding with unsigned direct mapping of the VLC
  //!	codeword
  int exp_golomb_decoding(byte* buffer);

  //!	Packet constructor, it just allocates memory for a coded packet (NALU).
  //!	Further operation will follow in the class' specializations
  Packet() { AllocNALU(8000000); }

  //! Packet destructor
  ~Packet() { FreeNALU(nalu); }

  void FreeNALU(NALU_t* n);
  int get_nalu_type() { return nalu->nal_unit_type; }
  SliceType get_slice_type() { return slice_type; }

  //!	The following functions will be implemented in the class' specializations
  virtual int getpacket() = 0;
  virtual int writepacket(FILE* f) = 0;
  virtual void setBitsFile(FILE* file) = 0;
  virtual int get_timestamp() { return -1; }
};

/*!
 *
 *	\brief
 *	The Real-time Transfer Protocol (RTP) specialization of the Packet class
 *
 *	\author
 *	Matteo Naccari
*/

class Rtp_packet : public Packet {
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
    unsigned int cc;         /*!< CSRC count, normally 0 in the absence
                    of RTP mixers */
    unsigned int m;          //!< Marker bit
    unsigned int pt;         //!< 7 bits, Payload Type, dynamically established 
    unsigned int seq;        /*!< RTP sequence number, incremented by one for
                    each sent packet */
    unsigned int old_seq;    //!< to detect wether packets were lost
    unsigned int timestamp;  //!< timestamp, 27 MHz for H.264
    unsigned int ssrc;       //!< Synchronization Source, chosen randomly
    byte* payload;    //!< the payload including payload headers
    unsigned int paylen;     //!< length of payload in bytes
    byte* packet;     //!< complete packet including header and payload
    unsigned int packlen;    //!< length of packet, typically paylen+12			
  }RTPpacket_t;

  RTPpacket_t* p;

  int CurrentRTPSequenceNumber, CurrentRTPTimestamp;

public:

  //!	Constructor for the Rtp_packet class 
  Rtp_packet() {
    allocate_rtp_packet();
    CurrentRTPSequenceNumber = 0;
    CurrentRTPTimestamp = 0;
  }

  ~Rtp_packet();

  int DecomposeRTPpacket();

  int RTPReadPacket(FILE* bits);

  void DumpRTPHeader();

  int getpacket();

  void allocate_rtp_packet();

  void setBitsFile(FILE* file) { bits = file; }

  int writepacket(FILE* f);

  int ComposeRTPPacket();

  int WriteRTPPacket(FILE* f);

  int get_timestamp() { return p->timestamp; }

};

/*!
 *
 *	\brief
 *	The Annex B specialization of the Packet class
 *
 *	\author
 *	Matteo Naccari
*/

class AnnexB_packet : public Packet {
private:
  int IsFirstByteStreamNALU;
public:
  AnnexB_packet() { IsFirstByteStreamNALU = 1; }
  void setBitsFile(FILE* file) { bits = file; }
  int getpacket();
  int FindStartCode(unsigned char* Buf, int zeros_in_startcode);
  int writepacket(FILE* f);  
};

#endif
