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

#ifndef H_READER_
#define H_READER_

#include <cstdint>
#include <string>

using namespace std;
/*!
 *
 * \brief
 * Class modelling a bit reader from a memory buffer.
 *
 * \author
 * Matteo Naccari (adapted from the HEVC reference test Model (HM))
*/
class Reader
{
  const uint8_t* m_buffer;

  uint32_t m_buffer_idx;

  uint32_t m_num_held_bits;
  uint8_t m_held_bits;
  uint32_t  m_num_bits_read;

public:
  Reader(const uint8_t* b)
    : m_buffer(b)
    , m_buffer_idx(0)
    , m_num_held_bits(0)
    , m_held_bits(0)
    , m_num_bits_read(0)
  {}

  uint32_t read_bits(uint32_t bits_to_read)
  {
    if (bits_to_read > 32) {
      throw logic_error("Cannot read: " + to_string(bits_to_read) + " bits in one go");
    }

    m_num_bits_read += bits_to_read;

    uint32_t retval = 0;
    if (bits_to_read <= m_num_held_bits) {
      retval = m_held_bits >> (m_num_held_bits - bits_to_read);
      retval &= ~(0xff << bits_to_read);
      m_num_held_bits -= bits_to_read;
      return retval;
    }

    bits_to_read -= m_num_held_bits;
    retval = m_held_bits & ~(0xff << m_num_held_bits);
    retval <<= bits_to_read;

    uint32_t aligned_word = 0;
    uint32_t num_bytes_to_load = (bits_to_read - 1) >> 3;

    switch (num_bytes_to_load) {
      case 3: aligned_word = m_buffer[m_buffer_idx++] << 24;
      case 2: aligned_word |= m_buffer[m_buffer_idx++] << 16;
      case 1: aligned_word |= m_buffer[m_buffer_idx++] << 8;
      case 0: aligned_word |= m_buffer[m_buffer_idx++];
    }

    uint32_t next_num_held_bits = (32 - bits_to_read) % 8;

    retval |= aligned_word >> next_num_held_bits;

    m_num_held_bits = next_num_held_bits;
    m_held_bits = aligned_word;

    return retval;
  }
};
#endif // !H_READER_
