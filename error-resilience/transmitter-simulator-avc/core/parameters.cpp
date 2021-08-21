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
#include <regex>
#include <iostream>
#include <fstream>

/*!
 *
 * \brief
 * First constructor whereby the parameters are passed via command line
 *
 * \param
 * argv, 2D array of char
 *
 * \author
 * Matteo Naccari
 *
*/

Parameters::Parameters(const char** argv)
  : m_bitstream_original(argv[1])
  , m_bitstream_transmitted(argv[2])
  , m_loss_pattern_file(argv[3])
{
  m_packet_type = stoi(argv[4]);

  m_offset = stoi(argv[5]);

  m_modality = stoi(argv[6]);

  check_parameters();
}

/*!
 *
 * \brief
 * Second constructor whereby the parameters are passed via configuration file
 *
 * \param
 * argv, name of the configuration file
 *
 * \author
 * Matteo Naccari
 *
*/

Parameters::Parameters(const char* argv)
{
  string line;
  int i = 0;
  ifstream fin;

  regex pattern_str("[^ ]+");
  regex pattern_nr("[+-]?[0-9]+");
  smatch match;

  fin.open(argv, ifstream::in);

  if (!fin) {
    throw runtime_error("Cannot open config file " + string(argv) + " abort");
  }

  while (getline(fin, line, '\n')) {
    if (valid_line(line)) {
      switch (i) {
      case 0:
        regex_search(line, match, pattern_str);
        m_bitstream_original = match[0];
        break;
      case 1:
        regex_search(line, match, pattern_str);
        m_bitstream_transmitted = match[0];
        break;
      case 2:
        regex_search(line, match, pattern_str);
        m_loss_pattern_file = match[0];
        break;
      case 3:
        regex_search(line, match, pattern_nr);
        m_packet_type = stoi(match[0]);
        break;
      case 4:
        regex_search(line, match, pattern_nr);
        m_offset = stoi(match[0]);
        break;
      case 5:
        regex_search(line, match, pattern_nr);
        m_modality = stoi(match[0]);
        break;
      default:
        cout << "Something wrong: (?)" << line << endl;
      }
      i++;
    }
  }
  check_parameters();
}

/*!
 *
 * \brief
 * Reads a valid line from the configuration file. A valid line is a text line
 * that does not start with the following characters: #, carriage return, space or
 * new line
 *
 * \param
 * line a char array containing the current line read from the configuration file
 *
 * \return
 * True if the current line is a valid line
 * False otherwise
 *
 * \author
 * Matteo Naccari
*/

bool Parameters::valid_line(const string& line)
{
  if (line.length() == 0 || line.at(0) == '\r' || line.at(0) == '#' || line.at(0) == ' ' || line.at(0) == '\n') {
    return 0;
  }
  return 1;
}

/*!
 *
 * \brief
 * Checks the compliance of the input parameters. A fault tolerant policy is adopted, i.e. only warnings are issued and the default values
 * are set accordingly
 *
 * \author
 * Matteo Naccari
*/
void Parameters::check_parameters()
{
  if (m_offset < 0) {
    cout << "Warning! Offset = " << m_offset << " is not allowed, set it to zero\n";
    m_offset = 0;
  }
  if (!(0 <= m_modality && m_modality <= 2)) {
    cout << "Warning! Modality = " << m_modality << " is not allowed, set it to zero\n";
    m_modality = 0;
  }
}