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
#include "packet.h"
#include <string>
#include <iomanip>

#ifdef _WIN32
#include <Winsock2.h>
#else
#include <netinet/in.h>
#endif

//////////////////////////////////////////////////////////////////////////////////////////
//        Packet member functions
/////////////////////////////////////////////////////////////////////////////////////////

/*!
 *
 * \brief
 * Allocates the memory space for a given NALU
 *
 * \param
 * buffersize the maximum dimension for the NALU buffer
 *
 * \author
 * Matteo Naccari
 *
*/

void Packet::alloc_nalu(int buffersize)
{
  m_nalu.buf.resize(buffersize, 0);
  m_nalu.max_size = buffersize;
}

/*!
 *
 * \brief
 * Decodes the slice type for the current NALU (i.e. the current packet).
 * Following the specification given in: "Information Technology - Coding of
 * audio-visual objects - Part 10: advanced video coding", the slice header syntax
 * contains firstly the number of the first macroblock belonging to the current slice
 * then it follows the slice type.
 * Both these two syntax elements are retrived by a simple exp-Golomb decoding with
 * unsigned direct mapping of the informative codeword
 *
 * \author
 * Matteo Naccari
 *
*/

void Packet::decode_slice_type()
{
  uint8_t* buffer = &m_nalu.buf[1];
  int dummy;

  //First syntax element in the slice header: first_mb_in_slice
  dummy = exp_golomb_decoding(buffer);

  //Second syntax element in the slice header: slice_type
  dummy = exp_golomb_decoding(buffer);

  if (dummy > 4) {
    dummy -= 5;
  }

  m_slice_type = SliceType(dummy);
}

/*!
 *
 * \brief
 * It performs the Exponential-Golomb decoding with unsigned direct mapping
 *
 * \param
 * buffer a pointer to the memory area which contains the coded data
 *
 * \return the slice type casted to the SliceType enumeration
*/

int Packet::exp_golomb_decoding(uint8_t* buffer)
{
  register int info = 0;
  long byteoffset = m_frame_bitoffset >> 3;         //! Byte containing the current starting bit
  int bitoffset = (7 - (m_frame_bitoffset & 0x07)); //! Starting bit inside the current byte
  int len = 0;                                      //! Control variable for exp-golomb decoding
  int M = 0;                                        //! Number of zeros before the first 1
  uint8_t* curr_byte = &(buffer[byteoffset]);       //! Current byte's address
  int ctr_bit = ((*curr_byte >> bitoffset) & 0x01); //! Control bit

  m_frame_bitoffset++;

  //First step of exp-golomb decoding: finding the leading 1
  while (ctr_bit == 0) {
    len++;
    m_frame_bitoffset++;
    bitoffset--;
    bitoffset &= 0x07; //Modulo 8 conversion
    curr_byte += (bitoffset == 7);
    byteoffset += (bitoffset == 7);
    ctr_bit = ((*curr_byte >> bitoffset) & 0x01);
  }

  M = len;

  //Second step of exp-golomb decoding: read the info part
  while (len) {
    len--;
    m_frame_bitoffset++;
    bitoffset--;
    bitoffset &= 0x07;
    curr_byte += (bitoffset == 7);
    info <<= 1;
    info |= ((*curr_byte >> bitoffset) & 0x01);
  }

  //Third step of exp-golomb decoding: info = 2^M + info - 1
  info = (1 << M) + info - 1;

  return info;
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//       RTP_packet member functions
/////////////////////////////////////////////////////////////////////////////////////////

/*!
 *
 * \brief
 * decompose_rtp_packet interprets the RTP packet and writes the various
 * structure members of the RTPpacket_t structure
 *
 * \return
 * 0 in case of success
 * negative error code in case of failure
 *
 *
 * \author
 * Adapted from the H.264/AVC reference implementation (aka JM)
 * Original author: Stephan Wenger   stewe@cs.tu-berlin.de
*/
int RtpPacket::decompose_rtp_packet()
{
  // consistency check
  if (!(m_rtp_data.packlen < MAXRTPPACKETSIZE)) {
    // IP, UDP headers
    throw logic_error("Condition: p.packlen < MAXRTPPACKETSIZE, violated");
  }

  if (!(m_rtp_data.packlen >= 12)) {
    // at least a complete RTP header
    throw logic_error("Condition: p.packlen >= 12, violated");
  }

  if (m_rtp_data.payload.size() == 0) {
    throw logic_error("p.payload not allocated");
  }

  if (m_rtp_data.packet.size() == 0) {
    throw logic_error("p.packet not allocated");
  }

  // Extract header information
  m_rtp_data.v = (m_rtp_data.packet[0] >> 6) & 0x03;
  m_rtp_data.p = (m_rtp_data.packet[0] >> 5) & 0x01;
  m_rtp_data.x = (m_rtp_data.packet[0] >> 4) & 0x01;
  m_rtp_data.cc = (m_rtp_data.packet[0] >> 0) & 0x0F;

  m_rtp_data.m = (m_rtp_data.packet[1] >> 7) & 0x01;
  m_rtp_data.pt = (m_rtp_data.packet[1] >> 0) & 0x7F;

  memcpy(&m_rtp_data.seq, &m_rtp_data.packet[2], 2);
  m_rtp_data.seq = ntohs((unsigned short)m_rtp_data.seq);

  memcpy(&m_rtp_data.timestamp, &m_rtp_data.packet[4], 4);// change to shifts for unified byte sex
  m_rtp_data.timestamp = ntohl(m_rtp_data.timestamp);
  memcpy(&m_rtp_data.ssrc, &m_rtp_data.packet[8], 4);// change to shifts for unified byte sex
  m_rtp_data.ssrc = ntohl(m_rtp_data.ssrc);

  // header consistency checks
  if ((m_rtp_data.v != 2) || (m_rtp_data.p != 0) || (m_rtp_data.x != 0) || (m_rtp_data.cc != 0)) {
    cerr << "DecomposeRTPpacket, RTP header consistency problem, header follows\n";
    dump_rtp_header();
    return -1;
  }
  m_rtp_data.paylen = m_rtp_data.packlen - 12;
  memcpy(&m_rtp_data.payload[0], &m_rtp_data.packet[12], m_rtp_data.paylen);
  return 0;
}

/*!
 *
 * \brief
 * RTPReadPacket reads one packet from file
 *
 * \return
 * 0: EOF
 * negative: error
 * positive: size of RTP packet in bytes
 *
 *
 *
 * \author
 * Adapted from the H.264/AVC reference implementation (aka JM)
 * Original author: Stephan Wenger   stewe@cs.tu-berlin.de
*/

int RtpPacket::rtp_read_packet(ifstream& ifs)
{
  uint32_t pos, intime;

  if (m_rtp_data.payload.size() == 0) {
    throw logic_error("p.payload has zero size");
  }

  if (m_rtp_data.packet.size() == 0) {
    throw logic_error("p.packed has zero size");
  }

  pos = static_cast<uint32_t>(ifs.tellg());

  ifs.read(reinterpret_cast<char*>(&m_rtp_data.packlen), 4);

  ifs.read(reinterpret_cast<char*>(&intime), 4);

  if (!(m_rtp_data.packlen < MAXRTPPACKETSIZE)) {
    throw logic_error("Condition: p.packlen < MAXRTPPACKETSIZE, violated");
  }

  ifs.read(reinterpret_cast<char*>(&m_rtp_data.packet[0]), m_rtp_data.packlen);

  if (decompose_rtp_packet() < 0) {
    // this should never happen. We probably do not want to attempt
    // to decode a packet that obviously wasn't generated by RTP
    throw logic_error("Errors reported by DecomposePacket()");
  }

  if (!(m_rtp_data.pt == H26LPAYLOADTYPE)) {
    throw logic_error("Condition: p.pt == H26LPAYLOADTYPE, violated");
  }
  
  if (!(m_rtp_data.ssrc == 0x12345678)) {
    throw logic_error("Condition: p.ssrc == 0x12345678, violated");
  }

  return m_rtp_data.packlen;
}
/*!
 *****************************************************************************
 *
 * \brief
 * DumpRTPHeader is a debug tool that dumps a human-readable interpretation
 * of the RTP header
 *
 * \author
 * Adapted from the H.264/AVC reference implementation (aka JM)
 * Original author: Stephan Wenger   stewe@cs.tu-berlin.de
*/
void RtpPacket::dump_rtp_header()
{
  int i;
  for (i = 0; i < 30; i++) {
    cout << setw(2) << hex << m_rtp_data.packet[i] << dec << endl;
  }

  cout << "Version (V): " << m_rtp_data.v << endl;
  cout << "Padding (P): " << m_rtp_data.p << endl;
  cout << "Extension (X): " <<  m_rtp_data.x << endl;
  cout << "CSRC count (CC): " <<  m_rtp_data.cc << endl;
  cout << "Marker bit (M): " <<  m_rtp_data.m << endl;
  cout << "Payload Type (PT): " <<  m_rtp_data.pt << endl;
  cout << "Sequence Number: " <<  m_rtp_data.seq << endl;
  cout << "Timestamp: " << m_rtp_data.timestamp << endl;
  cout << "SSRC: " << m_rtp_data.ssrc << endl;
}

/*!
 *
 * \brief
 * Reads an RTP packet for the bitstream being transmitted in order to simulate
 * error prone transmission
 *
 * \author
 * Matteo Naccari
 *
*/
int RtpPacket::get_packet(ifstream& ifs)
{
  int ret;

  ret = rtp_read_packet(ifs);
  m_nalu.forbidden_bit = 1;
  m_nalu.len = 0;

  m_frame_bitoffset = 0;

  if (ret < 0) {
    return -1;
  }

  if (ret == 0) {
    return 0;
  }

  if (!(m_rtp_data.paylen < m_nalu.max_size)) {
    throw logic_error("Condition: p.paylen < m_nalu.max_size, violated");
  }

  m_nalu.len = m_rtp_data.paylen;
  memcpy(&m_nalu.buf[0], &m_rtp_data.payload[0], m_rtp_data.paylen);
  m_nalu.forbidden_bit = (m_nalu.buf[0] >> 7) & 1;
  m_nalu.nal_reference_idc = (m_nalu.buf[0] >> 5) & 3;
  m_nalu.nal_unit_type = NaluType((m_nalu.buf[0]) & 0x1f);

  return ret;
}

/*!
 *
 * \brief
 * Allocates the memory space for an RTP packet
 *
 * \author
 * Matteo Naccari
 *
*/
void RtpPacket::allocate_rtp_packet()
{
  m_rtp_data.packet.resize(MAXRTPPACKETSIZE, 0);
  m_rtp_data.payload.resize(MAXRTPPACKETSIZE, 0);
}

/*!
 *
 * \brief
 * Writes on the received bitstream (i.e. the output bitstream) an RTP packet
 *
 * \param
 * f a pointer to the file associated to the transmitted bitstream
 *
 * \return
 * Number of bytes written to output file
 *
 * \author
 * Adapted from the H.264/AVC reference implementation (aka JM)
 * Original author: Stephan Wenger   stewe@cs.tu-berlin.de
 *
*/
int RtpPacket::write_packet(ofstream& ofs)
{
  if (!(m_nalu.len < 65000)) {
    throw logic_error("Condition m_nalu.len < 65000, violated");
  }

  m_nalu.buf[0] = (byte)(m_nalu.forbidden_bit << 7 | m_nalu.nal_reference_idc << 5 | int(m_nalu.nal_unit_type));

  m_rtp_data.v = 2;
  m_rtp_data.p = 0;
  m_rtp_data.x = 0;
  m_rtp_data.cc = 0;
  m_rtp_data.m = (m_nalu.startcodeprefix_len == 4) & 1;     // a long startcode of Annex B sets marker bit of RTP
                      // Not exactly according to the RTP paylaod spec, but
                      // good enough for now (hopefully).
                      //! For error resilience work, we need the correct
                      //! marker bit.  Introduce a nalu->marker and set it in
                      //! terminate_slice()?
  m_rtp_data.pt = H264PAYLOADTYPE;
  m_rtp_data.seq = current_rtp_sequence_number++;
  m_rtp_data.timestamp = current_rtp_time_stamp;
  m_rtp_data.ssrc = H264SSRC;
  m_rtp_data.paylen = m_nalu.len;
  memcpy(&m_rtp_data.payload[0], &m_nalu.buf[0], m_nalu.len);

  if (write_rtp_packet(ofs) < 0)
  {
    throw logic_error("RTP packet writing didn't complete successfully");
  }

  return (m_nalu.len * 8);
}

/*!
 *****************************************************************************
 *
 * \brief
 *    WriteRTPPacket writes the supplied RTP packet to the output file
 *
 * \return
 *    0 in case of access
 *    <0 in case of write failure (typically fatal)
 *
 * \param f
 *    output file
 *
 * \author
 * Adapted from the H.264/AVC reference implementation (aka JM)
 * Original author: Stephan Wenger   stewe@cs.tu-berlin.de
 *
 *****************************************************************************/

int RtpPacket::write_rtp_packet(ofstream& ofs)
{
  int intime = -1;

  ofs.write(reinterpret_cast<char*>(&m_rtp_data.packlen), 4);
  ofs.write(reinterpret_cast<char*>(&intime), 4);
  ofs.write(reinterpret_cast<char*>(&m_rtp_data.packet[0]), m_rtp_data.packlen);

  return 0;
}

/*!
 *****************************************************************************
 *
 * \brief
 *    ComposeRTPpacket composes the complete RTP packet using the various
 *    structure members of the RTPpacket_t structure
 *
 * \return
 *    0 in case of success
 *    negative error code in case of failure
 *
 *
 * \author
 * Adapted from the H.264/AVC reference implementation (aka JM)
 * Original author: Stephan Wenger   stewe@cs.tu-berlin.de
 *
 *****************************************************************************/

int RtpPacket::compose_rtp_packet()
{
  unsigned int temp32;
  unsigned short temp16;

  // Compose RTP header, little endian

  m_rtp_data.packet[0] = (uint8_t)(((m_rtp_data.v & 0x03) << 6) | ((m_rtp_data.p & 0x01) << 5) | ((m_rtp_data.x & 0x01) << 4) | ((m_rtp_data.cc & 0x0F) << 0));
  m_rtp_data.packet[1] = (uint8_t)(((m_rtp_data.m & 0x01) << 7) | ((m_rtp_data.pt & 0x7F) << 0));

  // sequence number, msb first
  temp16 = htons((unsigned short)m_rtp_data.seq);
  memcpy(&m_rtp_data.packet[2], &temp16, 2);  // change to shifts for unified byte sex

  //declare a temporary variable to perform network byte order converson
  temp32 = htonl(m_rtp_data.timestamp);
  memcpy(&m_rtp_data.packet[4], &temp32, 4);  // change to shifts for unified byte sex

  temp32 = htonl(m_rtp_data.ssrc);
  memcpy(&m_rtp_data.packet[8], &temp32, 4);// change to shifts for unified byte sex

  // Copy payload
  memcpy(&m_rtp_data.packet[12], &m_rtp_data.payload[0], m_rtp_data.paylen);
  m_rtp_data.packlen = m_rtp_data.paylen + 12;
  return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//       AnnexB_packet member functions
/////////////////////////////////////////////////////////////////////////////////////////

/*!
 *
 * \brief
 *    Returns the size of the NALU (bits between start codes in case of
 *    Annex B.  nalu->buf and nalu->len are filled.  Other field in
 *    nalu-> remain uninitialized (will be taken care of by NALUtoRBSP.
 *
 * \return
 *     0 if there is nothing any more to read (EOF)
 *    -1 in case of any error
 *
 *  \note Side-effect: Returns length of start-code in bytes.
 *
 * \note
 *   GetAnnexbNALU expects start codes at byte aligned positions in the file
 *
 * \author
 * Matteo Naccari (adapted from the H.264/AVC decoder reference software)
 */

int AnnexBPacket::get_packet(ifstream& ifs)
{
  int info2, info3;
  uint32_t pos = 0;
  int start_code_found, rewind;
  vector<unsigned char> buf(m_nalu.max_size, 0);
  int leading_zero_8bits_count = 0, trailing_zero_8bits = 0;
  m_frame_bitoffset = 0;

  for (pos = 0; pos < m_nalu.max_size; pos++) {
    ifs.read(reinterpret_cast<char*>(&buf[pos]), 1);
    if (ifs.eof() || buf[pos] != 0) {
      pos++;
      break;
    }
  }

  if (ifs.eof()) {
    if (pos == 0) {
      return 0;
    } else {
      throw logic_error("getpacket: can't read start code");
    }
  }

  if (buf[pos - 1] != 1) {
    throw logic_error("getpacket: no Start Code at the begin of the NALU");
  }

  if (pos < 3) {
    throw logic_error("getpacket: no Start Code at the begin of the NALU");
  } else if (pos == 3) {
    m_nalu.startcodeprefix_len = 3;
    leading_zero_8bits_count = 0;
  } else {
    leading_zero_8bits_count = pos - 4;
    m_nalu.startcodeprefix_len = 4;
  }

  // the 1st byte stream NAL unit can has leading_zero_8bits, but subsequent ones are not
  // allowed to contain it since these zeros(if any) are considered trailing_zero_8bits
  // of the previous byte stream NAL unit.
  if (!m_is_first_byte_stream_nalu && leading_zero_8bits_count > 0) {
    throw logic_error("getpacket: The leading_zero_8bits syntax can only be present in the first byte stream NAL unit");
  }
  m_is_first_byte_stream_nalu = 0;

  start_code_found = 0;
  info2 = 0;
  info3 = 0;

  while (!start_code_found) {
    if (ifs.eof()) {
      //Count the trailing_zero_8bits
      while (buf[pos - 2 - trailing_zero_8bits] == 0) {
        trailing_zero_8bits++;
      }
      m_nalu.len = (pos - 1) - m_nalu.startcodeprefix_len - leading_zero_8bits_count - trailing_zero_8bits;
      memcpy(&m_nalu.buf[0], &buf[leading_zero_8bits_count + m_nalu.startcodeprefix_len], m_nalu.len);
      m_nalu.forbidden_bit = (m_nalu.buf[0] >> 7) & 1;
      m_nalu.nal_reference_idc = (m_nalu.buf[0] >> 5) & 3;
      m_nalu.nal_unit_type = NaluType(m_nalu.buf[0] & 0x1f);

      return pos - 1;
    }
    ifs.read(reinterpret_cast<char*>(&buf[pos++]), 1);
    info3 = find_start_code(&buf[pos - 4], 3);
    if (info3 != 1) {
      info2 = find_start_code(&buf[pos - 3], 2);
    }
    start_code_found = (info2 == 1 || info3 == 1);
  }

  //Count the trailing_zero_8bits
  if (info3 == 1) {	//if the detected start code is 00 00 01, trailing_zero_8bits is sure not to be present
    while (buf[pos - 5 - trailing_zero_8bits] == 0) {
      trailing_zero_8bits++;
    }
  }
  // Here, we have found another start code (and read length of startcode bytes more than we should
  // have.  Hence, go back in the file
  rewind = 0;
  if (info3 == 1) {
    rewind = -4;
  } else if (info2 == 1) {
    rewind = -3;
  } else {
    // Should this be a game stopper?
    cerr << " Panic: Error in next start code search \n";
  }

  ifs.seekg(rewind, ios::cur);
  if (ifs.fail() || ifs.bad()) {
    throw logic_error("Something went wrong when moving the file pointer by " + to_string(rewind) + " bytes in the bitstream file");
  }

  m_nalu.len = (pos + rewind) - m_nalu.startcodeprefix_len - leading_zero_8bits_count - trailing_zero_8bits;
  memcpy(&m_nalu.buf[0], &buf[leading_zero_8bits_count + m_nalu.startcodeprefix_len], m_nalu.len);
  m_nalu.forbidden_bit = (m_nalu.buf[0] >> 7) & 1;
  m_nalu.nal_reference_idc = (m_nalu.buf[0] >> 5) & 3;
  m_nalu.nal_unit_type = NaluType(m_nalu.buf[0] & 0x1f);

  return (pos + rewind);
}

/*!
 *
 * \brief
 *    Returns if new start code is found at byte aligned position buf.
 *    new-startcode is of form N 0x00 bytes, followed by a 0x01 byte.
 *
 *  \return
 *     1 if start-code is found or                      \n
 *     0, indicating that there is no start code
 *
 *  \param Buf
 *     pointer to byte-stream
 *  \param zeros_in_startcode
 *     indicates number of 0x00 bytes in start-code.
 *
 * \author
 * Matteo Naccari (adapted from the H.264/AVC decoder reference software)
*/
int AnnexBPacket::find_start_code(unsigned char* Buf, int zeros_in_startcode)
{
  int info;
  int i;

  info = 1;
  for (i = 0; i < zeros_in_startcode; i++) {
    if (Buf[i] != 0)
      info = 0;
  }

  if (Buf[i] != 1) {
    info = 0;
  }

  return info;
}

/*!
 *
 * \brief
 *    Writes a NALU to the Annex B Byte Stream
 *
 * \return
 *    number of bits written
 *
 * \author
 * Matteo Naccari (adapted from the H.264/AVC decoder reference software)
*/
int AnnexBPacket::write_packet(ofstream& ofs)
{
  int bits_written = 0;
  unsigned char zero = 0, one = 1;

  if (m_nalu.forbidden_bit) {
    throw logic_error("Forbidden bit is not zero");
  }

  if (!(m_nalu.startcodeprefix_len == 3 || m_nalu.startcodeprefix_len == 4)) {
    throw logic_error("m_nalu.startcodeprefix_len == 3 || m_nalu.startcodeprefix_len == 4, violated");
  }

  if (m_nalu.startcodeprefix_len > 3) {
    ofs.write(reinterpret_cast<char*>(&zero), 1);
    bits_written += 8;
  }
  ofs.write(reinterpret_cast<char*>(&zero), 1);
  ofs.write(reinterpret_cast<char*>(&zero), 1);
  ofs.write(reinterpret_cast<char*>(&one), 1);
  bits_written += 24;

  m_nalu.buf[0] = (unsigned char)((m_nalu.forbidden_bit << 7) | (m_nalu.nal_reference_idc << 5) | int(m_nalu.nal_unit_type));

  ofs.write(reinterpret_cast<char*>(&m_nalu.buf[0]), m_nalu.len);
  bits_written += m_nalu.len * 8;

  ofs.flush();

  return bits_written;
}
/////////////////////////////////////////////////////////////////////////////////////////