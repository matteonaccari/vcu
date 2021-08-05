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

#include "parameters.h"
#include "simulator.h"
#include <iostream>
#include <fstream>
#include <exception>

#define VERSION 0.2

/*!
 *  \brief
 * It prints on the screen a little help on the program usage
 *
 *  \author
 *  Matteo Naccari
 *
*/
void inline_help()
{
  cout << endl << endl << "\tTransmitter Simulator for the H.264/AVC standard. Version " << VERSION << "\n\n";
  cout << "\tCopyright Matteo Naccari" << endl << endl;
  cout << "\tUsage (1): transmitter-simulator-avc <in_bitstream> <out_bitstream> <loss_pattern_file> <packet_type> <offset> <modality>\n\n";
  cout << "\tUsage (2): transmitter-simulator-avc <configuration_file>\n\n";
  cout << "See configuration file for further information on parameters.\n\n";
}

/*!
 *  \brief
 * The main function, i.e. the entry point of the program
 *  Note:
 *      The error patter file must contain only '0' and '1' ASCII
 *      characters (8 bits). The character '0' means that no channel error
 *      occurred whilst the character '1' means that a channel error
 *      occurred. A burst of channel errors is defined as a contiguous sequence of 2
 *      or more characters '1'.
 *
 *  \author
 *  Matteo Naccari
 *
*/

int main(int argc, char** argv) {
  unique_ptr<Parameters> p;
  unique_ptr<Simulator> sim;

  try {
    if (argc == 2) {
      p = make_unique<Parameters>(argv[1]);
    } else if (argc == 7) {
      p = make_unique<Parameters>(argv);
    } else {
      inline_help();
      return EXIT_SUCCESS;
    }

    sim = make_unique<Simulator>(*p);
    sim->run_simulator();
  } catch (exception e) {
    cerr << "Something went wrong: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
