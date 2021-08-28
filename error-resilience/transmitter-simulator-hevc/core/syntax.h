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

#ifndef H_SYNTAX_
#define H_SYNTAX_

#include <cstdint>
#include <vector>
#include <string>
#include <map>
#include <array>
#include "reader.h"

using namespace std;

enum class SliceType
{
  B_SLICE,
  P_SLICE,
  I_SLICE,
  INVALID_SLICE
};

enum class NaluType
{
  NAL_UNIT_CODED_SLICE_TRAIL_N = 0, // 0
  NAL_UNIT_CODED_SLICE_TRAIL_R,     // 1

  NAL_UNIT_CODED_SLICE_TSA_N,       // 2
  NAL_UNIT_CODED_SLICE_TSA_R,       // 3

  NAL_UNIT_CODED_SLICE_STSA_N,      // 4
  NAL_UNIT_CODED_SLICE_STSA_R,      // 5

  NAL_UNIT_CODED_SLICE_RADL_N,      // 6
  NAL_UNIT_CODED_SLICE_RADL_R,      // 7

  NAL_UNIT_CODED_SLICE_RASL_N,      // 8
  NAL_UNIT_CODED_SLICE_RASL_R,      // 9

  NAL_UNIT_RESERVED_VCL_N10,
  NAL_UNIT_RESERVED_VCL_R11,
  NAL_UNIT_RESERVED_VCL_N12,
  NAL_UNIT_RESERVED_VCL_R13,
  NAL_UNIT_RESERVED_VCL_N14,
  NAL_UNIT_RESERVED_VCL_R15,

  NAL_UNIT_CODED_SLICE_BLA_W_LP,    // 16
  NAL_UNIT_CODED_SLICE_BLA_W_RADL,  // 17
  NAL_UNIT_CODED_SLICE_BLA_N_LP,    // 18
  NAL_UNIT_CODED_SLICE_IDR_W_RADL,  // 19
  NAL_UNIT_CODED_SLICE_IDR_N_LP,    // 20
  NAL_UNIT_CODED_SLICE_CRA,         // 21
  NAL_UNIT_RESERVED_IRAP_VCL22,
  NAL_UNIT_RESERVED_IRAP_VCL23,

  NAL_UNIT_RESERVED_VCL24,
  NAL_UNIT_RESERVED_VCL25,
  NAL_UNIT_RESERVED_VCL26,
  NAL_UNIT_RESERVED_VCL27,
  NAL_UNIT_RESERVED_VCL28,
  NAL_UNIT_RESERVED_VCL29,
  NAL_UNIT_RESERVED_VCL30,
  NAL_UNIT_RESERVED_VCL31,

  NAL_UNIT_VPS,                     // 32
  NAL_UNIT_SPS,                     // 33
  NAL_UNIT_PPS,                     // 34
  NAL_UNIT_ACCESS_UNIT_DELIMITER,   // 35
  NAL_UNIT_EOS,                     // 36
  NAL_UNIT_EOB,                     // 37
  NAL_UNIT_FILLER_DATA,             // 38
  NAL_UNIT_PREFIX_SEI,              // 39
  NAL_UNIT_SUFFIX_SEI,              // 40

  NAL_UNIT_RESERVED_NVCL41,
  NAL_UNIT_RESERVED_NVCL42,
  NAL_UNIT_RESERVED_NVCL43,
  NAL_UNIT_RESERVED_NVCL44,
  NAL_UNIT_RESERVED_NVCL45,
  NAL_UNIT_RESERVED_NVCL46,
  NAL_UNIT_RESERVED_NVCL47,
  NAL_UNIT_UNSPECIFIED_48,
  NAL_UNIT_UNSPECIFIED_49,
  NAL_UNIT_UNSPECIFIED_50,
  NAL_UNIT_UNSPECIFIED_51,
  NAL_UNIT_UNSPECIFIED_52,
  NAL_UNIT_UNSPECIFIED_53,
  NAL_UNIT_UNSPECIFIED_54,
  NAL_UNIT_UNSPECIFIED_55,
  NAL_UNIT_UNSPECIFIED_56,
  NAL_UNIT_UNSPECIFIED_57,
  NAL_UNIT_UNSPECIFIED_58,
  NAL_UNIT_UNSPECIFIED_59,
  NAL_UNIT_UNSPECIFIED_60,
  NAL_UNIT_UNSPECIFIED_61,
  NAL_UNIT_UNSPECIFIED_62,
  NAL_UNIT_UNSPECIFIED_63,
  NAL_UNIT_INVALID
};

enum class ChromaFormat
{
  CHROMA_400 = 0,
  CHROMA_420 = 1,
  CHROMA_422 = 2,
  CHROMA_444 = 3,
  INVALID_CHROMA_FORMAT = 4
};

enum class Profile
{
  NONE = 0,
  MAIN = 1,
  MAIN10 = 2,
  MAINSTILLPICTURE = 3,
  MAINREXT = 4,
  HIGHTHROUGHPUTREXT = 5,
  MAINSCC = 9,
  HIGHTHROUGHPUTSCC = 11
};


/*!
 *
 * \brief
 * Structure corresponding to a Network Abstraction Layer Unit (NALU) as specified in Annex B of the standard
 *
 * \author
 * Matteo Naccari
*/
struct NALU
{
  int startcodeprefix_len = 0; //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
  unsigned len = 0;            //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
  unsigned max_size = 0;       //! Nal Unit Buffer size
  int forbidden_bit = 0;       //! Should be always FALSE
  vector<uint8_t> buf;         //! Contains the first byte followed by the EBSP
  vector<uint8_t> buf_rbsp;    //! Payload with emulation prevention codes stripped out
  NaluType nal_unit_type = NaluType::NAL_UNIT_INVALID;  //! NALU_TYPE

  bool is_slice()
  {
    return nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_TRAIL_R
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_TRAIL_N
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_TSA_R
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_TSA_N
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_STSA_R
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_STSA_N
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_BLA_W_LP
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_BLA_W_RADL
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_BLA_N_LP
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_IDR_W_RADL
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_IDR_N_LP
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_CRA
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_RADL_N
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_RADL_R
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_RASL_N
      || nal_unit_type == NaluType::NAL_UNIT_CODED_SLICE_RASL_R;
  }

  bool is_vcl()
  {
    return int(nal_unit_type) < 32;
  }

  bool is_pps()
  {
    return nal_unit_type == NaluType::NAL_UNIT_PPS;
  }

  bool is_sps()
  {
    return nal_unit_type == NaluType::NAL_UNIT_SPS;
  }

  NaluType get_nalu_type()
  {
    return nal_unit_type;
  }
};

/*!
 *
 * \brief
 * Structure corresponding to the Picture Parameter Set (PPS) syntax element (only the relevant information is retained)
 *
 * \author
 * Matteo Naccari
*/
struct ReducedPPS
{
  uint32_t m_id = 0;
  uint32_t m_sps_id = 0;
  bool m_dependent_slice_segments_enabled_flag = false;
  uint8_t m_num_extra_slice_header_bits = 0;
};

/*!
 *
 * \brief
 * Structure corresponding to the Sequence Parameter Set (SPS) syntax element (only the relevant information is retained)
 *
 * \author
 * Matteo Naccari
*/
struct ReducedSPS
{
  uint32_t m_id = 0;
  uint32_t m_pic_width_in_luma_samples = 0;
  uint32_t m_pic_height_in_luma_samples = 0;
  uint32_t m_log2_min_luma_coding_block_size_minus3 = 0;
  uint32_t m_log2_diff_max_min_luma_coding_block_size = 0;
  uint32_t m_cu_height = 0;
  uint32_t m_cu_width = 0;
};

/*!
 *
 * \brief
 *  Read an unsigned integer written with n bits
 *
 *  \param reader
 *  Bit reader
 *
 *  \param bits
 *  Number of bits to read
 *
 *  \param description
 *  Description of the syntax element associated with the reading (for code readability purposes)
 *
 *  \return
 *  The unsigned integer read from the nalu RBSP
 *
 *  \author
 *  Matteo Naccari
*/
inline uint32_t u(Reader& reader, int bits, const string& description = "")
{
  return reader.read_bits(bits);
}

/*!
 *
 *  \brief
 *  Performs the Exponential-Golomb decoding with unsigned direct mapping
 *
 *  \param reader
 *  Bit reader
 *
 *  \param description
 *  Description of the syntax element associated with the reading (for code readability purposes)
 *
 *  \return
 *  The unsigned integer decoded according to the Exponential Golomb decoding
 *
 *  \return the slice type casted to the SliceType enumeration
*/
inline uint32_t ue(Reader& reader, const string& description = "")
{
  uint32_t value = 0;
  auto code = reader.read_bits(1);

  if (code == 0) {
    uint32_t prefix_length = 1;
    while (reader.read_bits(1) == 0) {
      prefix_length++;
    }

    value = reader.read_bits(prefix_length);
    value += (1 << prefix_length) - 1;
  }

  return value;
}

/*!
 *
 * \brief
 * Parse partly the general slice segment header as specified in Clause 7.3.6.1 of the H.265/HEVC standard.
 * The parsing halts when the slice type is decoded
 *
 * \param
 * Reference to a reader object to read from the raw payload
 *
 * \param
 * Integer associated with the nalu type to drive the parsing process accordingly
 *
 * \param
 * Constant reference to a pps object to drive the parsing process accordingly
 *
 * \param
 * Constant reference to an sps object to drive the parsing process accordingly
 *
 * \return
 * The slice type (B, P or I)
 *
 * \author
 * Matteo Naccari
 *
*/
inline SliceType parse_slice_header(Reader& r, const int int_nalu_type, const map<uint32_t, ReducedPPS>& pps_memory, const map<uint32_t, ReducedSPS>& sps_memory)
{
  uint32_t value;
  bool dependent_slice_segment_flag = false;
  SliceType slice_type = SliceType::INVALID_SLICE;

  auto first_slice_segment_in_pic_flag = u(r, 1, "first_slice_segment_in_pic_flag");

  if (int(NaluType::NAL_UNIT_CODED_SLICE_BLA_W_LP) <= int_nalu_type && int_nalu_type <= int(NaluType::NAL_UNIT_RESERVED_IRAP_VCL23)) {
    value = u(r, 1, "no_output_of_prior_pics_flag");
  }
  value = ue(r, "slice_pic_parameter_set_id");
  const auto& pps = pps_memory.find(value)->second;

  if (!first_slice_segment_in_pic_flag) {
    if (pps.m_dependent_slice_segments_enabled_flag) {
      dependent_slice_segment_flag = u(r, 1, "dependent_slice_segment_flag");
    }

    const auto& sps = sps_memory.find(pps.m_sps_id)->second;
    int32_t total_ctus = ((sps.m_pic_width_in_luma_samples + sps.m_cu_width - 1) / sps.m_cu_width) * ((sps.m_pic_height_in_luma_samples + sps.m_cu_height - 1) / sps.m_cu_height);
    int32_t bits_seg_address = 0;
    while (total_ctus > (1 << bits_seg_address)) {
      bits_seg_address++;
    }

    value = u(r, bits_seg_address, "slice_segment_address");
  }

  if (!dependent_slice_segment_flag) {
    for (uint32_t i = 0; i < pps.m_num_extra_slice_header_bits; i++) {
      value = u(r, 1, "slice_reserved_flag[ i ]");
    }
    slice_type = SliceType(ue(r, "slice_type"));
  }

  return slice_type;
}

/*!
 *
 * \brief
 * Parse the general picture parameter set RBSP as specified in Clause 7.3.2.3.1 of the H.265/HEVC standard.
 * Only the relevant information is parsed and set accordingly.
 *
 * \param
 * Reference to a reader object to read from the raw payload
 *
 * \return
 * A pps object which can be stored in the whole memory associated with this syntax element
 *
 * \author
 * Matteo Naccari
 *
*/
inline ReducedPPS parse_reduced_pps(Reader& r)
{
  ReducedPPS pps;
  pps.m_id = ue(r, "pps_pic_parameter_set_id");
  pps.m_sps_id = ue(r, "pps_seq_parameter_set_id");
  pps.m_dependent_slice_segments_enabled_flag = u(r, 1, "dependent_slice_segments_enabled_flag");
  auto value = u(r, 1, "output_flag_present_flag");
  pps.m_num_extra_slice_header_bits = u(r, 3, "num_extra_slice_header_bits");

  return pps;
}

/*!
 *
 * \brief
 * Parse the profile and tier information as specified in Clause 7.3.3 of the H.265/HEVC standard.
 * Note that from the specification the same parsing applies for the main and sub layers. Accordingly, the
 * prefixes general_ and sub_ might be omitted.
 *
 * \param
 * Reference to a reader object to read from the raw payload
 *
 * \author
 * Matteo Naccari
 *
*/
inline void profile_tier(Reader& r)
{
  array<bool, 32> profile_compatibility_flag;
  auto value = u(r, 2, "profile_space");
  value = u(r, 1, "tier_flag");
  auto profile = Profile(u(r, 5, "profile_idc"));

  for (uint32_t j = 0; j < 32; j++) {
    profile_compatibility_flag[j] = bool(u(r, 1, "profile_compatibility_flag[j]"));
  }

  value = u(r, 1, "progressive_source_flag");
  value = u(r, 1, "interlaced_source_flag");
  value = u(r, 1, "non_packed_constraint_flag");
  value = u(r, 1, "frame_only_constraint_flag");

  if (profile == Profile::MAINREXT || profile_compatibility_flag[int(Profile::MAINREXT)] || profile == Profile::HIGHTHROUGHPUTREXT || profile_compatibility_flag[int(Profile::HIGHTHROUGHPUTREXT)]) {
    value = u(r, 1, "max_12bit_constraint_flag");
    value = u(r, 1, "max_10bit_constraint_flag");
    value = u(r, 1, "max_8bit_constraint_flag");

    value = u(r, 1, "max_422chroma_constraint_flag");
    value = u(r, 1, "max_420chroma_constraint_flag");
    value = u(r, 1, "max_monochrome_constraint_flag");
    
    value = u(r, 1, "intra_constraint_flag");
    value = u(r, 1, "one_picture_only_constraint_flag");
    value = u(r, 1, "lower_bit_rate_constraint_flag");

    value = u(r, 16, "reserved_zero_34bits[0..15]");
    value = u(r, 16, "reserved_zero_34bits[16..31]");
    value = u(r, 2, "reserved_zero_34bits[32..33]");
  }
  else {
    if (profile == Profile::MAIN10 || profile_compatibility_flag[int(Profile::MAIN10)]) {
      value = u(r, 7,"reserved_zero_7bits");
      value = u(r, 1, "one_picture_only_constraint_flag");
      value = u(r, 16, "reserved_zero_35bits[0..15]");
      value = u(r, 16, "reserved_zero_35bits[16..31]");
      value = u(r, 3, "reserved_zero_35bits[32..34]");
    }
    else {
      value = u(r, 16, "reserved_zero_43bits[0..15]");
      value = u(r, 16, "reserved_zero_43bits[16..31]");
      value = u(r, 11, "reserved_zero_43bits[32..42]");
    }
  }

  const bool compatibility_check = profile_compatibility_flag[int(Profile::MAIN)] ||
                                   profile_compatibility_flag[int(Profile::MAIN10)] ||
                                   profile_compatibility_flag[int(Profile::MAINSTILLPICTURE)] ||
                                   profile_compatibility_flag[int(Profile::MAINREXT)] ||
                                   profile_compatibility_flag[int(Profile::HIGHTHROUGHPUTREXT)];

  if ((int(profile) >= int(Profile::MAIN) && int(profile) <= int(Profile::HIGHTHROUGHPUTREXT)) || compatibility_check) {
    value = u(r, 1, "inbld_flag");
  }
  else {
    value = u(r, 1, "reserved_zero_bit");
  }

}

/*!
 *
 * \brief
 * Parse the profile, tier and level information as specified in Clause 7.3.3 of the H.265/HEVC standard.
 * The whole information is parsed so that the subsequence parsing of the SPS happens correctly.
 *
 * \param
 * Reference to a reader object to read from the raw payload
 *
 * \param
 * Flag to indicate that the profile is present (always equal to one)
 *
 * \param
 * Maximum number of temporal layers minus one which are associated with the SPS
 *
 * \author
 * Matteo Naccari
 *
*/
inline void profile_tier_level(Reader& r, const bool profile_preset_flag, const uint32_t max_sub_layers_m1)
{
  array<bool, 6> sub_layer_profile_present, sub_layer_level_present;
  if (profile_preset_flag) {
    profile_tier(r);
  }
  auto value = u(r, 8, "general_level_idc");

  for (uint32_t i = 0; i < max_sub_layers_m1; i++) {
    sub_layer_profile_present[i] = bool(u(r, 1, "sub_layer_profile_present_flag[i]"));
    sub_layer_level_present[i] = bool(u(r, 1, "sub_layer_level_present_flag[i]"));
  }

  if (max_sub_layers_m1 > 0) {
    for (uint32_t i = max_sub_layers_m1; i < 8; i++) {
      value = u(r, 2, "reserved_zero_2bits");
    }
  }

  for (uint32_t i = 0; i < max_sub_layers_m1; i++) {
    if (sub_layer_profile_present[i]) {
      profile_tier(r);
    }
    if (sub_layer_level_present[i]) {
      value = u(r, 8, "sub_layer_level_idc[i]");
    }
  }
}

/*!
 *
 * \brief
 * Parse the general sequence parameter set RBSP as specified in Clause 7.3.2.2.1 of the H.265/HEVC standard.
 * Only the relevant information is parsed and set accordingly.
 *
 * \param
 * Reference to a reader object to read from the raw payload
 *
 * \return
 * An sps object which can be stored in the whole memory associated with this syntax element
 *
 * \author
 * Matteo Naccari
 *
*/
inline ReducedSPS parse_reduced_sps(Reader& r)
{
  ReducedSPS sps;

  auto  value = u(r, 4, "sps_video_parameter_set_id");
  auto max_sub_layers_m1 = u(r, 3, "sps_max_sub_layers_minus1");
  value = u(r, 1, "sps_temporal_id_nesting_flag");

  profile_tier_level(r, true, max_sub_layers_m1);
  sps.m_id = ue(r, "sps_seq_parameter_set_id");

  ChromaFormat chf = ChromaFormat(ue(r, "chroma_format_idc"));

  if (chf == ChromaFormat::CHROMA_444) {
    value = u(r, 1, "separate_colour_plane_flag");
  }

  sps.m_pic_width_in_luma_samples = ue(r, "pic_width_in_luma_samples");
  sps.m_pic_height_in_luma_samples = ue(r, "pic_height_in_luma_samples");
  value = u(r, 1, "conformance_window_flag");
  if (value != 0) {
    value = ue(r, "conf_win_left_offset");
    value = ue(r, "conf_win_right_offset");
    value = ue(r, "conf_win_top_offset");
    value = ue(r, "conf_win_bottom_offset");
  }

  value = ue(r, "bit_depth_luma_minus8");
  value = ue(r, "bit_depth_chroma_minus8");
  value = ue(r, "log2_max_pic_order_cnt_lsb_minus4");

  auto sps_sub_layer_ordering_info_present_flag = bool(u(r, 1, "sps_sub_layer_ordering_info_present_flag"));

  for (uint32_t i = sps_sub_layer_ordering_info_present_flag ? 0 : max_sub_layers_m1; i <= max_sub_layers_m1; i++) {
    value = ue(r, "sps_max_dec_pic_buffering_minus1[i]");
    value = ue(r, "sps_max_num_reorder_pics[i]");
    value = ue(r, "sps_max_latency_increase_plus1[i]");
  }

  sps.m_log2_min_luma_coding_block_size_minus3 = ue(r, "log2_min_luma_coding_block_size_minus3");
  sps.m_log2_diff_max_min_luma_coding_block_size = ue(r, "log2_diff_max_min_luma_coding_block_size");

  uint32_t log2MaxCuSize = sps.m_log2_min_luma_coding_block_size_minus3 + 3 + sps.m_log2_diff_max_min_luma_coding_block_size;
  sps.m_cu_width = 1 << log2MaxCuSize;
  sps.m_cu_height = 1 << log2MaxCuSize;

  return sps;
}

#endif // !H_SYNTAX_

