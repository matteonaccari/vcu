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

#include "gtest/gtest.h"
#include "parameters.h"
#include "packet.h"
#include "simulator.h"
#include "md5.h"
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////
// Parameter module tests
//////////////////////////////////////////////////////////////////
TEST(TestParameter, TestParametersInitialisationFromCmdLine)
{
  const string input_file_name = "bistream.264";
  const string output_file_name = "bistream_err.264";
  const string pattern_file_name = "error.txt";
  const int offset = 1979;
  const int packet_type = 0;
  const int modality = 2;

  const string offset_str = to_string(offset);
  const string packet_type_str = to_string(packet_type);
  const string modality_str = to_string(modality);

  const char* cmdLine[] = { "transmitter-simulator-avc.exe", input_file_name.c_str(), output_file_name.c_str(), pattern_file_name.c_str(),
                             packet_type_str.c_str(), offset_str.c_str(), modality_str.c_str() };

  Parameters p(cmdLine);

  EXPECT_TRUE(input_file_name == p.get_bitstream_original_filename());
  EXPECT_TRUE(output_file_name == p.get_bitstream_transmitted_filename());
  EXPECT_TRUE(pattern_file_name == p.get_loss_pattern_filename());
  EXPECT_EQ(packet_type, p.get_packet_type());
  EXPECT_EQ(modality, p.get_modality());
  EXPECT_EQ(offset, p.get_offset());
}

TEST(TestParameter, TestParametersInitialisationFromFile)
{
  const string parameter_file_name = "../config_file.txt";

  Parameters p(parameter_file_name.c_str());

  EXPECT_TRUE("container.264" == p.get_bitstream_original_filename());
  EXPECT_TRUE("container_err.264" == p.get_bitstream_transmitted_filename());
  EXPECT_TRUE("error_plr_3" == p.get_loss_pattern_filename());
  EXPECT_EQ(1, p.get_packet_type());
  EXPECT_EQ(0, p.get_modality());
  EXPECT_EQ(0, p.get_offset());
}

//////////////////////////////////////////////////////////////////
// AnnexB packet module tests
//////////////////////////////////////////////////////////////////
TEST(TestPacketAnnexB, TestPacketIsSPS)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> sps_stream = { 0, 0, 0, 1, int(NaluType::NALU_TYPE_SPS), 0, 0, 0, 1 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&sps_stream[0]), sps_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<AnnexBPacket>();

  p->get_packet(ifs);

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_SPS, p->get_nalu_type());
  EXPECT_FALSE(p->is_nalu_vcl());
}

TEST(TestPacketAnnexB, TestPacketIsPPS)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> pps_stream = { 0, 0, 0, 1, int(NaluType::NALU_TYPE_PPS), 0, 0, 0, 1 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&pps_stream[0]), pps_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<AnnexBPacket>();

  p->get_packet(ifs);

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_PPS, p->get_nalu_type());
  EXPECT_FALSE(p->is_nalu_vcl());
}

TEST(TestPacketAnnexB, TestPacketIsIDR)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> idr_stream = { 0, 0, 0, 1, int(NaluType::NALU_TYPE_IDR), 0, 0, 0, 1 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&idr_stream[0]), idr_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<AnnexBPacket>();

  p->get_packet(ifs);

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_IDR, p->get_nalu_type());
  EXPECT_TRUE(p->is_nalu_vcl());
}

TEST(TestPacketAnnexB, TestPacketIsSlice)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> slice_stream = { 0, 0, 0, 1, int(NaluType::NALU_TYPE_SLICE), 0, 0, 0, 1 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&slice_stream[0]), slice_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<AnnexBPacket>();

  p->get_packet(ifs);

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_SLICE, p->get_nalu_type());
  EXPECT_TRUE(p->is_nalu_vcl());
}

TEST(TestPacketAnnexB, TestPacketIsSliceI)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> slice_i_stream = { 0, 0, 0, 1, int(NaluType::NALU_TYPE_SLICE), 136, 0, 0, 0, 1 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&slice_i_stream[0]), slice_i_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<AnnexBPacket>();

  p->get_packet(ifs);
  p->decode_slice_type();

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_SLICE, p->get_nalu_type());
  EXPECT_TRUE(p->is_nalu_vcl());
  EXPECT_EQ(SliceType::I_SLICE, p->get_slice_type());
}

TEST(TestPacketAnnexB, TestPacketIsSliceP)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> slice_p_stream = { 0, 0, 0, 1, int(NaluType::NALU_TYPE_SLICE), 152, 0, 0, 0, 1 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&slice_p_stream[0]), slice_p_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<AnnexBPacket>();

  p->get_packet(ifs);
  p->decode_slice_type();

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_SLICE, p->get_nalu_type());
  EXPECT_TRUE(p->is_nalu_vcl());
  EXPECT_EQ(SliceType::P_SLICE, p->get_slice_type());
}

TEST(TestPacketAnnexB, TestPacketIsSliceB)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> slice_b_stream = { 0, 0, 0, 1, int(NaluType::NALU_TYPE_SLICE), 156, 0, 0, 0, 1 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&slice_b_stream[0]), slice_b_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<AnnexBPacket>();

  p->get_packet(ifs);
  p->decode_slice_type();

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_SLICE, p->get_nalu_type());
  EXPECT_TRUE(p->is_nalu_vcl());
  EXPECT_EQ(SliceType::B_SLICE, p->get_slice_type());
}

TEST(TestPacketAnnexB, TestPacketParserFailsOnWrongStartCode1)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> bad_nalu_stream = { 0, 1, int(NaluType::NALU_TYPE_SLICE), 0, 0, 0, 1 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&bad_nalu_stream[0]), bad_nalu_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<AnnexBPacket>();

  EXPECT_THROW(p->get_packet(ifs), logic_error);

  ifs.close();
  remove(nalu_file_name.c_str());
}

TEST(TestPacketAnnexB, TestPacketParserFailsOnWrongStartCode2)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> bad_nalu_stream = { 0, 0, 0, 2, int(NaluType::NALU_TYPE_SLICE), 0, 0, 0, 1 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&bad_nalu_stream[0]), bad_nalu_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<AnnexBPacket>();

  EXPECT_THROW(p->get_packet(ifs), logic_error);

  ifs.close();
  remove(nalu_file_name.c_str());
}

//////////////////////////////////////////////////////////////////
// RTP packet module tests
//////////////////////////////////////////////////////////////////
TEST(TestPacketRTP, TestPacketIsSPS)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> sps_stream = { 13, 0, 0, 0, 255, 255, 255, 255, 128, 233, 0, 0, 0, 0, 0, 0, 18, 52, 86, 120, int(NaluType::NALU_TYPE_SPS), 77 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&sps_stream[0]), sps_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<RtpPacket>();

  p->get_packet(ifs);

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_SPS, p->get_nalu_type());
  EXPECT_FALSE(p->is_nalu_vcl());
}

TEST(TestPacketRTP, TestPacketIsPPS)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> pps_stream = { 13, 0, 0, 0, 255, 255, 255, 255, 128, 233, 0, 0, 0, 0, 0, 0, 18, 52, 86, 120, int(NaluType::NALU_TYPE_PPS), 77 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&pps_stream[0]), pps_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<RtpPacket>();

  p->get_packet(ifs);

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_PPS, p->get_nalu_type());
  EXPECT_FALSE(p->is_nalu_vcl());
}

TEST(TestPacketRTP, TestPacketIsIDR)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> idr_stream = { 13, 0, 0, 0, 255, 255, 255, 255, 128, 233, 0, 0, 0, 0, 0, 0, 18, 52, 86, 120, int(NaluType::NALU_TYPE_IDR), 77 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&idr_stream[0]), idr_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<RtpPacket>();

  p->get_packet(ifs);

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_IDR, p->get_nalu_type());
  EXPECT_TRUE(p->is_nalu_vcl());
}

TEST(TestPacketRTP, TestPacketIsSlice)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> slice_stream = { 13, 0, 0, 0, 255, 255, 255, 255, 128, 233, 0, 0, 0, 0, 0, 0, 18, 52, 86, 120, int(NaluType::NALU_TYPE_SLICE), 77 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&slice_stream[0]), slice_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<RtpPacket>();

  p->get_packet(ifs);

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_SLICE, p->get_nalu_type());
  EXPECT_TRUE(p->is_nalu_vcl());
}

TEST(TestPacketRTP, TestPacketIsSliceI)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> slice_i_stream = { 14, 0, 0, 0, 255, 255, 255, 255, 128, 233, 0, 0, 0, 0, 0, 0, 18, 52, 86, 120, int(NaluType::NALU_TYPE_SLICE), 136, 77 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&slice_i_stream[0]), slice_i_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<RtpPacket>();

  p->get_packet(ifs);
  p->decode_slice_type();

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_SLICE, p->get_nalu_type());
  EXPECT_TRUE(p->is_nalu_vcl());
  EXPECT_EQ(SliceType::I_SLICE, p->get_slice_type());
}

TEST(TestPacketRTP, TestPacketIsSliceP)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> slice_p_stream = { 14, 0, 0, 0, 255, 255, 255, 255, 128, 233, 0, 0, 0, 0, 0, 0, 18, 52, 86, 120, int(NaluType::NALU_TYPE_SLICE), 152, 77 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&slice_p_stream[0]), slice_p_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<RtpPacket>();

  p->get_packet(ifs);
  p->decode_slice_type();

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_SLICE, p->get_nalu_type());
  EXPECT_TRUE(p->is_nalu_vcl());
  EXPECT_EQ(SliceType::P_SLICE, p->get_slice_type());
}

TEST(TestPacketRTP, TestPacketIsSliceB)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> slice_b_stream = { 14, 0, 0, 0, 255, 255, 255, 255, 128, 233, 0, 0, 0, 0, 0, 0, 18, 52, 86, 120, int(NaluType::NALU_TYPE_SLICE), 156, 77 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&slice_b_stream[0]), slice_b_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<RtpPacket>();

  p->get_packet(ifs);
  p->decode_slice_type();

  ifs.close();
  remove(nalu_file_name.c_str());

  EXPECT_EQ(NaluType::NALU_TYPE_SLICE, p->get_nalu_type());
  EXPECT_TRUE(p->is_nalu_vcl());
  EXPECT_EQ(SliceType::B_SLICE, p->get_slice_type());
}

TEST(TestPacketRTP, TestPacketParserFailsOnIllegalRTP)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> bad_rtp_stream = { 229, 255, 0, 0, 255, 255, 255, 255, 128, 233, 0, 0, 0, 0, 0, 0, 18, 52, 86, 120, int(NaluType::NALU_TYPE_SLICE), 77 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&bad_rtp_stream[0]), bad_rtp_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<RtpPacket>();

  EXPECT_THROW(p->get_packet(ifs), logic_error);

  ifs.close();
  remove(nalu_file_name.c_str());
}

TEST(TestPacketRTP, TestPacketParserFailsOnTooShortLength)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> too_short_rtp_stream = { 11, 0, 0, 0, 255, 255, 255, 255, 128, 233, 0, 0, 0, 0, 0, 0, 18, 52, 86, 120, int(NaluType::NALU_TYPE_SLICE), 77 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&too_short_rtp_stream[0]), too_short_rtp_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<RtpPacket>();

  EXPECT_THROW(p->get_packet(ifs), logic_error);

  ifs.close();
  remove(nalu_file_name.c_str());
}

TEST(TestPacketRTP, TestPacketParserFailsOnNonAVCPayload)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> non_avc_rtp_stream = { 13, 0, 0, 0, 255, 255, 255, 255, 128, 232, 0, 0, 0, 0, 0, 0, 18, 52, 86, 120, int(NaluType::NALU_TYPE_SLICE), 77 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&non_avc_rtp_stream[0]), non_avc_rtp_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<RtpPacket>();

  EXPECT_THROW(p->get_packet(ifs), logic_error);

  ifs.close();
  remove(nalu_file_name.c_str());
}

TEST(TestPacketRTP, TestPacketParserFailsOnWrongSsrc)
{
  const string nalu_file_name = "nalu_stream.bin";
  vector<uint8_t> bad_ssrc_rtp_stream = { 13, 0, 0, 0, 255, 255, 255, 255, 128, 233, 0, 0, 0, 0, 0, 0, 17, 52, 86, 120, int(NaluType::NALU_TYPE_SLICE), 77 };

  ofstream ofs(nalu_file_name.c_str(), ios::binary);
  ofs.write(reinterpret_cast<char*>(&bad_ssrc_rtp_stream[0]), bad_ssrc_rtp_stream.size());
  ofs.close();

  ifstream ifs(nalu_file_name.c_str(), ios::binary);
  unique_ptr<Packet> p = make_unique<RtpPacket>();

  EXPECT_THROW(p->get_packet(ifs), logic_error);

  ifs.close();
  remove(nalu_file_name.c_str());
}

//////////////////////////////////////////////////////////////////
// Simulator module tests
//////////////////////////////////////////////////////////////////
TEST(TestSimulator, TestConstructorReactsOnWrongBitstreamName)
{
  const char* cmdLine[] = { "transmitter-simulator-avc.exe", "non_existing.bin", "", "whatever_plr_0", "0", "0", "0"};

  Parameters p(cmdLine);

  EXPECT_THROW(Simulator s(p), runtime_error);
}

TEST(TestSimulator, TestConstructorReactsOnWrongErrorPattern)
{
  const char* cmdLine[] = { "transmitter-simulator-avc.exe", "../unit-tests/bitstream_annexb.264", "bitstream_annexb_err.264", "whatever_plr_0", "0", "0", "0" };

  Parameters p(cmdLine);

  EXPECT_THROW(Simulator s(p), runtime_error);

  remove("bitstream_annexb_err.264");
}

TEST(TestSimulator, TestPlr0LeavesBitstreamIntact)
{
  const char* cmdLine[] = { "transmitter-simulator-avc.exe", "../unit-tests/bitstream_annexb.264", "bitstream_annexb_err.264", "../unit-tests/error_plr_0", "1", "0", "0" };
  ifstream ifs;

  Parameters p(cmdLine);

  Simulator s(p);

  s.run_simulator();

  // Compute MD5s for the original and corrupted
  ifs.open("../unit-tests/bitstream_annexb.264", ios::binary);
  string data_original = string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
  ifs.close();

  ifs.open("bitstream_annexb_err.264", ios::binary);
  string data_err = string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
  ifs.close();

  EXPECT_TRUE(md5(data_original) == md5(data_err));

  remove("bitstream_annexb_err.264");
}

TEST(TestSimulator, TestPlr3GivesTheExpectMD5)
{
  const char* cmdLine[] = { "transmitter-simulator-avc.exe", "../unit-tests/bitstream_annexb.264", "bitstream_annexb_err.264", "../error_plr_3", "1", "10", "0" };
  ifstream ifs;
  const string expected_md5 = "520e6ce1387750e8f5f218af5865c69b";

  Parameters p(cmdLine);

  Simulator s(p);

  s.run_simulator();

  ifs.open("bitstream_annexb_err.264", ios::binary);
  string data_err = string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
  ifs.close();

  EXPECT_TRUE(expected_md5 == md5(data_err));

  remove("bitstream_annexb_err.264");
}