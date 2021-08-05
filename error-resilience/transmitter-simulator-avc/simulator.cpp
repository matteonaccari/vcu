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
#include "simulator.h"
#include <iostream>

/*!
 *
 * \brief
 * The constructor for the simulator class. It sets up all the transmission enviroment:
 * bitstream being transmitted, received bitstream, error pattern file (the simulated error
 * prone channel) and the packetization used (AnnexB or RTP)
 *
 * \param
 * p a pointer to a parameters object which contains all the transmission parameters
 *
 * \author
 * Matteo Naccari
 *
*/

Simulator::Simulator(const Parameters& p)
  : m_param(p)
{
  string temp_loss_pattern;
  char* temp_str;
  int offset;

  m_fp_bitstream.open(m_param.get_bitstream_original_filename(), ios::binary);
  if (!m_fp_bitstream) {
    throw runtime_error("Cannot open " + m_param.get_bitstream_original_filename() + " input bitstream, abort");
  }

  m_fp_tr_bitstream.open(m_param.get_bitstream_transmitted_filename(), ios::binary);
  if (!m_fp_tr_bitstream) {
    throw runtime_error("Cannot open " + m_param.get_bitstream_transmitted_filename() + " transmitted bitstream, abort");
  }

  if (m_param.get_packet_type() == 0) { //RTP
    m_packet = make_unique<RtpPacket>();
  } else if (m_param.get_packet_type() == 1) { //Annex B
    m_packet = make_unique<AnnexBPacket>();
  } else {
    throw runtime_error("Bad packet type: " + to_string(m_param.get_packet_type()));
  }

  ifstream  fp_losspattern(m_param.get_loss_pattern_filename().c_str(), ifstream::in);

  if (!fp_losspattern) {
    runtime_error("Cannot open " + m_param.get_loss_pattern_filename() + " loss pattern file, abort");
  }

  fp_losspattern.seekg(0, ios_base::end);

  m_numchar = static_cast<int>(fp_losspattern.tellg());

  fp_losspattern.seekg(0, ios::beg);

  temp_str = new char[m_numchar];

  fp_losspattern.get(temp_str, m_numchar);

  temp_loss_pattern = temp_str;

  delete[] temp_str;

  // Builds the new error pattern string in order to simulate different channel realisations
  offset = m_param.get_offset() % temp_loss_pattern.length();

  m_loss_pattern = temp_loss_pattern.substr(offset, temp_loss_pattern.length() - offset);

  m_loss_pattern.append(temp_loss_pattern.substr(0, offset));
}

/*!
 *
 * \brief
 * Simulates the transmission of one coded bitstream through an error prone channel.
 * The method reads every nalu which corresponds to a coded slice. For each nalu the
 * method checks whether the current slice contains coded data rather than syntax
 * parameters as for example PPS, SPS, etc.
 * If the current slice contains coded data, then the Run_Simulator funtion decodes the
 * slice type in order to finalize the decision of transmitting or corrupting the data
 *
 * \author
 * Matteo Naccari
 *
*/

void Simulator::run_simulator()
{
  int i = 0, bytes, writeable;

  print_header();

  while (true) {
    if (m_fp_bitstream.eof()) {
      break;
    }
    writeable = 0;
    bytes = m_packet->get_packet(m_fp_bitstream);

    //Slice type decoding only for coded data slices [1:5]
    if (m_packet->is_nalu_vcl()) {
      m_packet->decode_slice_type();
    }

    if (bytes <= 0) {
      break;
    }

    switch (m_param.get_modality())
    {
    case 0:
      // Normal corruption: do nothing
      break;
    case 1:
      // Corrupt all slices but the intra ones: check whether the current slice is actually intra coded
      if (m_packet->get_slice_type() == SliceType::I_SLICE) {
        writeable = 1;
      }
      break;
    case 2:
      // Corrupts only intra coded slices: check whether the current slice is not intra coded
      if (m_packet->get_slice_type() != SliceType::I_SLICE) {
        writeable = 1;
      }
      break;
    }

    if (!m_packet->is_nalu_vcl()) {
      bytes = m_packet->write_packet(m_fp_tr_bitstream);
    } else if (m_loss_pattern[i] == '0') {
      bytes = m_packet->write_packet(m_fp_tr_bitstream);
      i++;
    } else if (m_loss_pattern[i] == '1') {
      if (writeable) {
        // Writes although the slice is ought to be discarded: this is because the modality chosen says to do so
        bytes = m_packet->write_packet(m_fp_tr_bitstream);
      } else {
        i++;
      }
    } else {
      cerr << "Wrong character used in the error pattern string: " << m_loss_pattern[i] << '\n';
    }

    if (i >= m_numchar - 1) {
      // Mimics a circular buffer
      i = 0;
    }
  }
}

/*!
 *
 * \brief
 * Prints the operating settings of the simulator so the user can be sure the software has been provided with the right inputs
 *
 * \author
 * Matteo Naccari
 *
*/
void Simulator::print_header()
{
  const string corruption_modality_text[] = { "all", "all but intra", "intra only" };
  const string packet_type_text[] = { "RTP", "AnnexB" };
  cout << "Input bitstream: " << m_param.get_bitstream_original_filename() << endl;
  cout << "Transmitted bitstream: " << m_param.get_bitstream_transmitted_filename() << endl;
  cout << "Error pattern file: " << m_param.get_loss_pattern_filename() << endl;
  cout << "Packet type: " << packet_type_text[m_param.get_packet_type()] << endl;
  cout << "Starting offset: " << m_param.get_offset() << endl;
  cout << "Corruption modality: " << corruption_modality_text[m_param.get_modality()] << endl << endl;
}