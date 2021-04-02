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

#include "packet.h"
#include <cassert>
#include <cstring>

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
  m_nalu.max_size = buffersize;
  m_nalu.buf.resize(buffersize, 0);
}

/*!
 *
 * \brief
 *  Returns if new start code is found at byte aligned position buf.
 *  new-startcode is of form N 0x00 bytes, followed by a 0x01 byte.
 *
 *  \return
 *  1 if start-code is found or
 *  0, indicating that there is no start code
 *
 *  \param Buf
 *  pointer to byte-stream
 *
 *  \param zeros_in_startcode
 *  indicates number of 0x00 bytes in start-code.
 *
 *	\author
 *	Matteo Naccari (adapted from the H.264/AVC decoder reference software)
*/
int Packet::find_start_code(unsigned char* buf, int zeros_in_startcode) {
  int info;
  int i;

  info = 1;
  for (i = 0; i < zeros_in_startcode; i++)
    if (buf[i] != 0)
      info = 0;

  if (buf[i] != 1)
    info = 0;
  return info;
}

/*!
 *
 * \brief
 * Converts the elementary byte stream payload to the raw byte stream payload formar where emulation prevention codes are not present
 *
 *	\author
 *	Matteo Naccari
*/
void Packet::convert_to_rbsp()
{
  m_nalu.buf_rbsp.clear();
  m_nalu.buf_rbsp.resize(m_nalu.len);
  copy(m_nalu.buf.begin(), m_nalu.buf.begin() + m_nalu.len, m_nalu.buf_rbsp.begin());

  vector<uint8_t>::iterator read, write;
  uint32_t zero_count = 0;

  for (read = write = m_nalu.buf_rbsp.begin(); read != m_nalu.buf_rbsp.end(); read++, write++) {
    if (zero_count == 2 && *read == 0x03) {
      read++;
      zero_count = 0;

      if (read == m_nalu.buf_rbsp.end()) {
        break;
      }
    }
    zero_count = *read == 0x00 ? zero_count + 1 : 0;
    *write = *read;
  }

  m_nalu.buf_rbsp.resize(write - m_nalu.buf_rbsp.begin());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Public members
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*!
 *
 * \brief
 * Parse partly the general slice segment header as specified in Clause 7.3.6.1 of the H.265/HEVC standard.
 * The parsing halts when the slice type is decoded
 *
 * \author
 * Matteo Naccari
 *
*/
void Packet::parse_slice_type()
{
  const int int_nalu_type = int(m_nalu.nal_unit_type);
  Reader r(&m_nalu.buf_rbsp[2]);

  m_slice_type = parse_slice_header(r, int_nalu_type, m_pps_memory, m_sps_memory);
}

/*!
 *
 * \brief
 * Parse the general picture parameter set RBSP as specified in Clause 7.3.2.3.1 of the H.265/HEVC standard.
 * The pps is then stored in the array of pps in case the stream is using multiple versions of it.
 *
 * \author
 * Matteo Naccari
 *
*/
void Packet::parse_pps()
{
  Reader r(&m_nalu.buf_rbsp[2]);

  auto pps = parse_reduced_pps(r);

  m_pps_memory[pps.m_id] = pps;
}

/*!
 *
 * \brief
 * Parse the general sequence parameter set RBSP as specified in Clause 7.3.2.2.1 of the H.265/HEVC standard.
 * The SPS is then stored in the array of sps in case the stream is using multiple versions of it.
 *
 * \author
 * Matteo Naccari
 *
*/
void Packet::parse_sps()
{
  Reader r(&m_nalu.buf_rbsp[2]);

  auto sps = parse_reduced_sps(r);

  m_sps_memory[sps.m_id] = sps;
}

/*!
 *
 * \brief
 *    Returns the size of the NALU (bits between start codes in case of
 *    Annex B.  nalu.buf and nalu.len are filled.  Other field in
 *    nalu. remain uninitialized (will be taken care of by NALUtoRBSP.
 *
 * \return
 *     0 if there is nothing any more to read (EOF)
 *    -1 in case of any error
 *
 *  \note Side-effect: Returns length of start-code in bytes.
 *
 * \note
 *   getpacket expects start codes at byte aligned positions in the file
 *
 *	\author
 *	Matteo Naccari (adapted from the H.264/AVC decoder reference software)
 */
int Packet::get_packet(ifstream& bits)
{
  int info2, info3;
  uint32_t pos = 0;
  int start_code_found, rewind;
  vector<unsigned char> buf(m_nalu.max_size, 0);
  int leading_zero_8bits_count = 0, trailing_zero_8bits = 0;

  for (pos = 0; pos < m_nalu.max_size; pos++) {
    bits.read(reinterpret_cast<char*>(&buf[pos]), 1);
    if (bits.eof() || buf[pos] != 0) {
      pos++;
      break;
    }
  }

  if (bits.eof()) {
    if (pos == 0) {
      return 0;
    }
    else {
      throw logic_error("getpacket: can't read start code");
    }
  }

  if (buf[pos - 1] != 1) {
    throw logic_error("getpacket: no Start Code at the begin of the NALU, return -1");
  }

  if (pos < 3) {
    throw logic_error("getpacket: no Start Code at the begin of the NALU, return -1");
  }
  else if (pos == 3) {
    m_nalu.startcodeprefix_len = 3;
    leading_zero_8bits_count = 0;
  }
  else {
    leading_zero_8bits_count = pos - 4;
    m_nalu.startcodeprefix_len = 4;
  }

  // the 1st byte stream NAL unit can has leading_zero_8bits, but subsequent ones are not
  // allowed to contain it since these zeros(if any) are considered trailing_zero_8bits
  // of the previous byte stream NAL unit.
  if (!m_is_first_byte_stream_nalu && leading_zero_8bits_count > 0) {
    throw logic_error("getpacket: The leading_zero_8bits syntax can only be present in the first byte stream NAL unit, return -1");
  }
  m_is_first_byte_stream_nalu = 0;

  start_code_found = 0;
  info2 = 0;
  info3 = 0;

  while (!start_code_found) {
    if (bits.eof()) {
      //Count the trailing_zero_8bits
      while (buf[pos - 2 - trailing_zero_8bits] == 0) {
        trailing_zero_8bits++;
      }
      m_nalu.len = (pos - 1) - m_nalu.startcodeprefix_len - leading_zero_8bits_count - trailing_zero_8bits;
      memcpy(&m_nalu.buf[0], &buf[leading_zero_8bits_count + m_nalu.startcodeprefix_len], m_nalu.len);
      m_nalu.forbidden_bit = (m_nalu.buf[0] >> 7) & 1;
      m_nalu.nal_unit_type = NaluType((m_nalu.buf[0]) >> 1);

      return pos - 1;
    }
    bits.read(reinterpret_cast<char*>(&buf[pos++]), 1);
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
  }
  else if (info2 == 1) {
    rewind = -3;
  }
  else {
    // Should this be a game stopper?
    cerr << " Panic: Error in next start code search \n";
  }

  bits.seekg(rewind, ios::cur);
  if (bits.fail() || bits.bad()) {
    logic_error("Something went wrong when moving the file pointer by " + to_string(rewind) + " bytes in the bitstream file");
  }

  m_nalu.len = (pos + rewind) - m_nalu.startcodeprefix_len - leading_zero_8bits_count - trailing_zero_8bits;
  memcpy(&m_nalu.buf[0], &buf[leading_zero_8bits_count + m_nalu.startcodeprefix_len], m_nalu.len);
  m_nalu.forbidden_bit = (m_nalu.buf[0] >> 7) & 1;
  m_nalu.nal_unit_type = NaluType((m_nalu.buf[0]) >> 1);

  convert_to_rbsp();

  return (pos + rewind);
}

/*!
 *
 *	\brief
 *    Writes a NALU to the Annex B Byte Stream
 *
 *	\return
 *    number of bits written
 *
 *	\author
 *	Matteo Naccari (adapted from the H.264/AVC decoder reference software)
*/
int Packet::write_packet(ofstream& ofs)
{
  int bits_written = 0;
  unsigned char zero = 0, one = 1;

  assert(m_nalu.forbidden_bit == 0);
  assert(m_nalu.startcodeprefix_len == 3 || m_nalu.startcodeprefix_len == 4);

  // printf ("WriteAnnexbNALU: writing %d bytes w/ startcode_len %d\n", n->len+1, n->startcodeprefix_len);
  if (m_nalu.startcodeprefix_len > 3) {
    ofs.write(reinterpret_cast<char*>(&zero), 1);
    bits_written += 8;
  }
  ofs.write(reinterpret_cast<char*>(&zero), 1);
  ofs.write(reinterpret_cast<char*>(&zero), 1);
  ofs.write(reinterpret_cast<char*>(&one), 1);
  bits_written += 24;

  m_nalu.buf[0] = (unsigned char)((m_nalu.forbidden_bit << 7) | (int(m_nalu.nal_unit_type)) << 1);

  ofs.write(reinterpret_cast<char*>(&m_nalu.buf[0]), m_nalu.len);
  bits_written += m_nalu.len * 8;

  ofs.flush();

  return bits_written;
}