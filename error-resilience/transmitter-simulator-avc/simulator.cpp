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

/*!
 *
 *	\brief
 *	The constructor for the simulator class. It sets up all the transmission enviroment:
 *	bitstream being transmitted, received bitstream, error pattern file (the simulated error
 *	prone channel) and the packetization used (AnnexB or RTP)
 *
 *	\param
 *	p a pointer to a parameters object which contains all the transmission parameters
 *
 *	\author
 *	Matteo Naccari
 *
*/

Simulator::Simulator(Parameters* p) {

  string temp_loss_pattern;
  char* temp_str;
  int offset;

  param = p;

  if ((fp_bitstream = fopen(param->get_bitstream_original_filename().c_str(), "rb")) == NULL) {
    cout << "Cannot open " << param->get_bitstream_original_filename() << "input bitstream, abort" << endl;
    exit(-1);
  }

  if ((fp_tr_bitstream = fopen(param->get_bitstream_transmitted_filename().c_str(), "wb+")) == NULL) {
    cout << "Cannot open " << param->get_bitstream_transmitted_filename() << "transmitted bitstream, abort" << endl;
    exit(-1);
  }

  if (param->get_packet_type() == 0) { //RTP
    packet = new Rtp_packet();
  }
  else if (param->get_packet_type() == 1) { //Annex B
    packet = new AnnexB_packet();
  }
  else {
    cout << "Bad packet type, abort" << endl;
    exit(-1);
  }

  fp_losspattern.open(param->get_loss_pattern_filename().c_str(), ifstream::in);

  if (!fp_losspattern) {
    cout << "Cannot open " << param->get_loss_pattern_filename() << "loss pattern file, abort" << endl;
    exit(-1);
  }

  fp_losspattern.seekg(0, ios_base::end);

  numchar = fp_losspattern.tellg();

  fp_losspattern.seekg(0, ios::beg);

  temp_str = new char[numchar];

  fp_losspattern.get(temp_str, numchar);

  fp_losspattern.close();

  temp_loss_pattern = temp_str;

  delete temp_str;

  //It builds the new error pattern string in order to simulate different channel
//realizations
  offset = param->get_offset() % temp_loss_pattern.length();

  loss_pattern = temp_loss_pattern.substr(offset, temp_loss_pattern.length() - offset);

  loss_pattern.append(temp_loss_pattern.substr(0, offset));
}

/*!
 *
 *	\brief
 *	It simulates the transmission of one coded bitstream through an error prone channel.
 *	The method reads every nalu which corresponds to a coded slice. For each nalu the
 *	Run_Simulator checks whether the current slice contains coded data rather than syntax
 *	parameters as for example PPS, SPS, etc.
 *	If the current slice contains coded data, then the Run_Simulator funtion decodes the
 *	slice type in order to finalize the decision of transmitting or corrupting the data
 *	Note:
 *		The Run_Simulator function does not corrupt the nalus corresponding to the first
 *		video sequence frame
 *
 *	\author
 *	Matteo Naccari
 *
*/

void Simulator::Run_Simulator() {

  int i = 0, read, writeable, timestamp;

  packet->setBitsFile(fp_bitstream);

  while (true) {
    if (feof(fp_bitstream))
      break;
    writeable = 0;
    read = packet->getpacket();

    //Slice type decoding only for coded data slices [1:5]
    if (packet->get_nalu_type() <= 5) {
      packet->decode_slice_type();
    }

    if (read <= 0) {
      break;
    }

    switch (param->get_modality())
    {
    case 0:  //	Normal corruption: do nothing
      break;
    case 1:  //	It corrupts all the slice but the intra ones:
         //	check whether the current slice is actually intra coded
      if (packet->get_slice_type() == I_SLICE)
        writeable = 1;
      break;
    case 2:  //	It corrupts only intra coded slices: check whether current
           //	slice is not intra coded
      if (packet->get_slice_type() != I_SLICE)
        writeable = 1;
      break;
    }

    timestamp = packet->get_timestamp();

    if (loss_pattern.at(i) == '0' || timestamp == 0 || packet->get_nalu_type() > 5) {
      read = packet->writepacket(fp_tr_bitstream);
      i++;
    }
    else if (loss_pattern.at(i) == '1') {
      if (writeable) { //It writes although the slice has to be discarded
        read = packet->writepacket(fp_tr_bitstream);
      }
      else {
        i++;
      }
    }
    else {
      cout << "problems!\n";
    }

    if (i >= numchar - 1) //circular buffer
      i = 0;
  }
}
/*!
 *
 *	\brief
 *	Destructor of the Simulator class
 *
 *	\author
 *	Matteo Naccari
 */
Simulator::~Simulator() {
  fclose(fp_bitstream);
  fclose(fp_tr_bitstream);
  delete packet;
}