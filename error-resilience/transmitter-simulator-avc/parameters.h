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
#ifndef H_PARAMETERS_
#define H_PARAMETERS_

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

/*!
 *
 *	\brief
 *	It models the parameters related to the transmission conditions
 *
 *	\author
 *	Matteo Naccari
 *
*/

class Parameters {
private:
  string bitstream_original, bitstream_transmitted, loss_pattern_file;
  int modality, offset, packet_type;
  bool valid_line(string line);
public:
  //! First constructor: the parameters are passed through command line
  Parameters(char** argv);

  //! Second constructor: the parameters are passed through configuration file
  Parameters(char* argv);

  ~Parameters();

  string get_bitstream_original_filename() { return bitstream_original; }
  string get_bitstream_transmitted_filename() { return bitstream_transmitted; }
  string get_loss_pattern_filename() { return loss_pattern_file; }
  int get_modality() { return modality; }
  int get_offset() { return offset; }
  int get_packet_type() { return packet_type; }
  void check_parameters();
};

#endif