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

/*!
 *
 *	\brief
 *	First constructor whereby the parameters are passed via command line
 *
 *	\param
 *	argv, string of input parameters
 *
 *	\author
 *	Matteo Naccari
 *
*/

Parameters::Parameters(char** argv) {

  bitstream_original = argv[1];

  bitstream_transmitted = argv[2];

  loss_pattern_file = argv[3];

  packet_type = atoi(argv[4]);

  offset = atoi(argv[5]);

  modality = atoi(argv[6]);

  check_parameters();
}

/*!
 *
 *	\brief
 *	Second constructor whereby the parameters are passed via configuration file
 *
 *	\param
 *	argv, name of the configuration file
 *
 *	\author
 *	Matteo Naccari
 *
*/

Parameters::Parameters(char* argv) {
  string line;
  int i = 0;
  char temp[100];
  ifstream fin;

  fin.open(argv, ifstream::in);

  if (!fin) {
    cout << "Cannot open config file " << argv << " abort" << endl;
    exit(-1);
  }

  while (getline(fin, line, '\n')) {
    if (valid_line(line)) {
      switch (i)
      {
      case 0:
        sscanf(line.c_str(), "%s", temp);
        bitstream_original = temp;
        break;
      case 1:
        sscanf(line.c_str(), "%s", temp);
        bitstream_transmitted = temp;
        break;
      case 2:
        sscanf(line.c_str(), "%s", temp);
        loss_pattern_file = temp;
        break;
      case 3:
        sscanf(line.c_str(), "%d", &packet_type);
        break;
      case 4:
        sscanf(line.c_str(), "%d", &offset);
        break;
      case 5:
        sscanf(line.c_str(), "%d", &modality);
        break;
      default:
        cout << "Something wrong: (?)" << line << endl;
      }
      i++;
    }
  }

  fin.close();
  check_parameters();
}

/*!
 *
 *	\brief
 *	It reads a valid line from the configuration file. A valid line is a text line
 *	that does not start with the following characters: #, carriage return, space or
 *	new line
 *
 *	\param
 *	line a char array containing the current line read from the configuration file
 *
 *	\return
 *	True if the current line is valid line
 *	False otherwise
 *
 *	\author
 *	Matteo Naccari
*/

bool Parameters::valid_line(string line) {
  if (line.length() == 0)
    return 0;
  if (line.at(0) == '\r' || line.at(0) == '#' || line.at(0) == ' ' || line.at(0) == '\n')
    return 0;
  return 1;
}

Parameters::~Parameters() {
}

void Parameters::check_parameters() {

  if (offset < 0) {
    cout << "Warning! Offset = " << offset << " is not allowed, set it to zero\n";
    offset = 0;
  }
  if (!(0 <= modality && modality <= 2)) {
    cout << "Warning! Modality = " << modality << " is not allowed, set it to zero\n";
    modality = 0;
  }

}