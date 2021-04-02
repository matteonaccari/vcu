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
#include "packet.h"
#include <cstring>

//////////////////////////////////////////////////////////////////////////////////////////
//								Packet member functions
/////////////////////////////////////////////////////////////////////////////////////////
/*!
 *
 *	\brief
 *	It frees the memory space for a given NALU
 *
 *	\param
 *	n a pointer to the NALU being cancelled
 *
 *	\author
 *	Matteo Naccari
 *
*/
void Packet::FreeNALU(NALU_t* n)
{
  if (n)
  {
    if (n->buf)
    {
      free(n->buf);
      n->buf = NULL;
    }
    free(n);
  }
}

/*!
 *
 *	\brief
 *	It allocates the memory space for a given NALU
 *
 *	\param
 *	buffersize the maximum dimension for the NALU buffer
 *
 *	\author
 *	Matteo Naccari
 *
*/

void Packet::AllocNALU(int buffersize)
{

  if ((nalu = (NALU_t*)calloc(1, sizeof(NALU_t))) == NULL)
    cout << "no_mem_exit (AllocNALU: n)" << endl;

  nalu->max_size = buffersize;

  if ((nalu->buf = (byte*)calloc(buffersize, sizeof(byte))) == NULL) cout << "GetAnnexbNALU: Buf" << endl;

}

/*!
 *
 *	\brief
 *	It decodes the slice type for the current NALU (i.e. the current packet).
 *	Following the specification given in: "Information Technology - Coding of
 *	audio-visual objects - Part 10: advanced video coding", the slice header syntax
 *	contains firstly the number of the first macroblock belonging to the current slice
 *	then it follows the slice type.
 *	Both these two syntax elements are retrived by a simple exp-Golomb decoding with
 *	unsigned direct mapping of the informative codeword
 *
 *	\author
 *	Matteo Naccari
 *
*/

void Packet::decode_slice_type() {

  byte* buffer = &nalu->buf[1];
  int dummy;

  //First syntax element in the slice header: first_mb_in_slice
  dummy = exp_golomb_decoding(buffer);

  //Second syntax element in the slice header: slice_type
  dummy = exp_golomb_decoding(buffer);

  if (dummy > 4) dummy -= 5;

  slice_type = (SliceType)dummy;
}

/*!
 *
 *	\brief
 *	It performs the Exponential-Golomb decoding with unsigned direct mapping
 *
 *	\param
 *	buffer a pointer to the memory area which contains the coded data
 *
 *	\return the slice type casted to the SliceType enumeration
*/

int Packet::exp_golomb_decoding(byte* buffer) {

  register int info = 0;
  long byteoffset = frame_bitoffset >> 3;                 //!	Byte containing the current starting bit
  int bitoffset = (7 - (frame_bitoffset & 0x07));     //!	Starting bit inside the current byte	
  int len = 0;                                  //!	Control variable for exp-golomb decoding
  int M = 0;                                  //!	Number of zeros before the first 1
  byte* curr_byte = &(buffer[byteoffset]);              //!	Current byte's address
  int ctr_bit = ((*curr_byte >> bitoffset) & 0x01); //!	Control bit

  frame_bitoffset++;

  //First step of exp-golomb decoding: finding the leading 1	
  while (ctr_bit == 0) {
    len++;
    frame_bitoffset++;
    bitoffset--;
    bitoffset &= 0x07; //Modulo 8 conversion
    curr_byte += (bitoffset == 7);
    byteoffset += (bitoffset == 7);
    ctr_bit = ((*curr_byte >> bitoffset) & 0x01);
  }

  M = len;

  //Second step of exp-golomb decoding: read the info part
  while (len) {
    len--;
    frame_bitoffset++;
    bitoffset--;
    bitoffset &= 0x07;
    curr_byte += (bitoffset == 7);
    info <<= 1;
    info |= ((*curr_byte >> bitoffset) & 0x01);
  }

  //Third step of exp-golomb decoding: info = 2^M + info - 1
  info = (1 << M) + info - 1;

  return info;

}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//							RTP_packet member functions
/////////////////////////////////////////////////////////////////////////////////////////
/*!
 *
 * \brief
 *    DecomposeRTPpacket interprets the RTP packet and writes the various
 *    structure members of the RTPpacket_t structure
 *
 * \return
 *    0 in case of success
 *    negative error code in case of failure
 *
 *
 * \author
 *    Stephan Wenger   stewe@cs.tu-berlin.de
*/
int Rtp_packet::DecomposeRTPpacket() {

  // consistency check 
  assert(p->packlen < 65536 - 28);  // IP, UDP headers
  assert(p->packlen >= 12);         // at least a complete RTP header
  assert(p->payload != NULL);
  assert(p->packet != NULL);

  // Extract header information

  p->v = (p->packet[0] >> 6) & 0x03;
  p->p = (p->packet[0] >> 5) & 0x01;
  p->x = (p->packet[0] >> 4) & 0x01;
  p->cc = (p->packet[0] >> 0) & 0x0F;

  p->m = (p->packet[1] >> 7) & 0x01;
  p->pt = (p->packet[1] >> 0) & 0x7F;

  memcpy(&p->seq, &p->packet[2], 2);
  p->seq = ntohs((unsigned short)p->seq);

  memcpy(&p->timestamp, &p->packet[4], 4);// change to shifts for unified byte sex
  p->timestamp = ntohl(p->timestamp);
  memcpy(&p->ssrc, &p->packet[8], 4);// change to shifts for unified byte sex
  p->ssrc = ntohl(p->ssrc);

  // header consistency checks
  if ((p->v != 2)
    || (p->p != 0)
    || (p->x != 0)
    || (p->cc != 0))
  {
    cout << "DecomposeRTPpacket, RTP header consistency problem, header follows\n";
    DumpRTPHeader();
    return -1;
  }
  p->paylen = p->packlen - 12;
  memcpy(p->payload, &p->packet[12], p->paylen);
  return 0;
}

/*!
 *
 * \brief
 *    RTPReadPacket reads one packet from file
 *
 * \return
 *    0: EOF
 *    negative: error
 *    positive: size of RTP packet in bytes
 *
 *
 *
 * \author
 *    Stephan Wenger, stewe@cs.tu-berlin.de
*/

int Rtp_packet::RTPReadPacket(FILE* bits) {

  int Filepos, intime;

  assert(p != NULL);
  assert(p->packet != NULL);
  assert(p->payload != NULL);

  Filepos = ftell(bits);

  if (4 != fread(&p->packlen, 1, 4, bits))
  {
    return 0;
  }


  if (4 != fread(&intime, 1, 4, bits))
  {
    fseek(bits, Filepos, SEEK_SET);
    cout << "RTPReadPacket: File corruption, could not read Timestamp, exit\n";
    exit(-1);
  }

  assert(p->packlen < MAXRTPPACKETSIZE);

  if (p->packlen != fread(p->packet, 1, p->packlen, bits))
  {
    cout << "RTPReadPacket: File corruption, could not read " << p->packlen << " bytes\n";
    exit(-1);    // EOF indication
  }
  if (DecomposeRTPpacket() < 0)
  {
    // this should never happen, hence exit() is ok.  We probably do not want to attempt
    // to decode a packet that obviously wasn't generated by RTP
    cout << "Errors reported by DecomposePacket(), exit\n";
    exit(-700);
  }
  assert(p->pt == H26LPAYLOADTYPE);
  assert(p->ssrc == 0x12345678);
  return p->packlen;
}
/*!
 *****************************************************************************
 *
 * \brief
 *    DumpRTPHeader is a debug tool that dumps a human-readable interpretation
 *    of the RTP header
 *
 * \author
 *    Stephan Wenger   stewe@cs.tu-berlin.de
*/
void Rtp_packet::DumpRTPHeader() {
  int i;
  for (i = 0; i < 30; i++)
    printf("%02x ", p->packet[i]);
  printf("Version (V): %d\n", p->v);
  printf("Padding (P): %d\n", p->p);
  printf("Extension (X): %d\n", p->x);
  printf("CSRC count (CC): %d\n", p->cc);
  printf("Marker bit (M): %d\n", p->m);
  printf("Payload Type (PT): %d\n", p->pt);
  printf("Sequence Number: %d\n", p->seq);
  printf("Timestamp: %d\n", p->timestamp);
  printf("SSRC: %d\n", p->ssrc);
}

/*!
 *
 *	\brief
 *	It reads an RTP packet for the bitstream being transmitted in order to simulate
 *	error prone transmission
 *
 *	\author
 *	Matteo Naccari
 *
*/
int Rtp_packet::getpacket() {

  int ret;

  ret = RTPReadPacket(bits);
  nalu->forbidden_bit = 1;
  nalu->len = 0;

  frame_bitoffset = 0;

  if (ret < 0)
    return -1;
  if (ret == 0)
    return 0;

  assert(p->paylen < nalu->max_size);

  nalu->len = p->paylen;
  memcpy(nalu->buf, p->payload, p->paylen);
  nalu->forbidden_bit = (nalu->buf[0] >> 7) & 1;
  nalu->nal_reference_idc = (nalu->buf[0] >> 5) & 3;
  nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;

  return ret;

}

/*!
 *
 *	\brief
 *	It allocates the memory space for an RTP packet
 *
 *	\author
 *	Matteo Naccari
 *
*/
void Rtp_packet::allocate_rtp_packet() {

  if ((p = (RTPpacket_t*)malloc(sizeof(RTPpacket_t))) == NULL) {
    cout << "There is not enought memory for RTP Packet, abort\n" << endl;
    exit(-1);
  }
  if ((p->packet = (byte*)malloc(MAXRTPPACKETSIZE)) == NULL) {
    cout << "There is not enought memory for RTP Packet, abort\n" << endl;
    exit(-1);
  }

  if ((p->payload = (byte*)malloc(MAXRTPPACKETSIZE)) == NULL) {
    cout << "There is not enought memory for RTP payload, abort\n" << endl;
    exit(-1);
  }

}

/*!
 *
 *	\brief
 *	It writes on the received bitstream (i.e. the output bitstream) an RTP packet
 *
 *	\param
 *	f a pointer to the file associated to the transmitted bitstream
 *
 *	\return
 *	Number of bytes written to output file
 *
 *	\author
 *	Matteo Naccari
 *	Stephan Wenger   stewe@cs.tu-berlin.de
 *
*/
int Rtp_packet::writepacket(FILE* f) {

  assert(f != NULL);
  assert(nalu != NULL);
  assert(nalu->len < 65000);

  nalu->buf[0] = (byte)(nalu->forbidden_bit << 7 | nalu->nal_reference_idc << 5 | nalu->nal_unit_type);

  p->v = 2;
  p->p = 0;
  p->x = 0;
  p->cc = 0;
  p->m = (nalu->startcodeprefix_len == 4) & 1;     // a long startcode of Annex B sets marker bit of RTP
                      // Not exactly according to the RTP paylaod spec, but
                      // good enough for now (hopefully).
                      //! For error resilience work, we need the correct
                      //! marker bit.  Introduce a nalu->marker and set it in
                      //! terminate_slice()?
  p->pt = H264PAYLOADTYPE;
  p->seq = CurrentRTPSequenceNumber++;
  p->timestamp = CurrentRTPTimestamp;
  p->ssrc = H264SSRC;
  p->paylen = nalu->len;
  memcpy(p->payload, nalu->buf, nalu->len);

  if (WriteRTPPacket(f) < 0)
  {
    printf("Cannot write %d bytes of RTP packet to outfile, exit\n", p->packlen);
    exit(-1);
  }

  return (nalu->len * 8);
}

/*!
 *****************************************************************************
 *
 * \brief
 *    WriteRTPPacket writes the supplied RTP packet to the output file
 *
 * \return
 *    0 in case of access
 *    <0 in case of write failure (typically fatal)
 *
 * \param f
 *    output file
 *
 * \author
 *    Stephan Wenger   stewe@cs.tu-berlin.de
 *****************************************************************************/

int Rtp_packet::WriteRTPPacket(FILE* f)

{
  int intime = -1;

  assert(f != NULL);
  assert(p != NULL);


  if (1 != fwrite(&p->packlen, 4, 1, f))
    return -1;
  if (1 != fwrite(&intime, 4, 1, f))
    return -1;
  if (1 != fwrite(p->packet, p->packlen, 1, f))
    return -1;
  return 0;
}

Rtp_packet::~Rtp_packet() {

  free(p->payload);
  free(p->packet);
  free(p);

}

/*!
 *****************************************************************************
 *
 * \brief
 *    ComposeRTPpacket composes the complete RTP packet using the various
 *    structure members of the RTPpacket_t structure
 *
 * \return
 *    0 in case of success
 *    negative error code in case of failure
 *
 * \note
 *    Function contains assert() tests for debug purposes (consistency checks
 *    for RTP header fields
 *
 * \author
 *    Stephan Wenger   stewe@cs.tu-berlin.de
 *****************************************************************************/

int Rtp_packet::ComposeRTPPacket()

{
  unsigned int temp32;
  unsigned short temp16;

  // Consistency checks through assert, only used for debug purposes
  assert(p->v == 2);
  assert(p->p == 0);
  assert(p->x == 0);
  assert(p->cc == 0);    // mixer designers need to change this one
  assert(p->m == 0 || p->m == 1);
  assert(p->pt < 128);
  assert(p->seq < 65536);
  assert(p->payload != NULL);
  assert(p->paylen < 65536 - 40);  // 2**16 -40 for IP/UDP/RTP header
  assert(p->packet != NULL);

  // Compose RTP header, little endian

  p->packet[0] = (byte)
    (((p->v & 0x03) << 6)
      | ((p->p & 0x01) << 5)
      | ((p->x & 0x01) << 4)
      | ((p->cc & 0x0F) << 0));

  p->packet[1] = (byte)
    (((p->m & 0x01) << 7)
      | ((p->pt & 0x7F) << 0));

  // sequence number, msb first
  temp16 = htons((unsigned short)p->seq);
  memcpy(&p->packet[2], &temp16, 2);  // change to shifts for unified byte sex

  //declare a temporary variable to perform network byte order converson
  temp32 = htonl(p->timestamp);
  memcpy(&p->packet[4], &temp32, 4);  // change to shifts for unified byte sex

  temp32 = htonl(p->ssrc);
  memcpy(&p->packet[8], &temp32, 4);// change to shifts for unified byte sex

  // Copy payload

  memcpy(&p->packet[12], p->payload, p->paylen);
  p->packlen = p->paylen + 12;
  return 0;
}
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//							AnnexB_packet member functions
/////////////////////////////////////////////////////////////////////////////////////////

/*!
 *
 * \brief
 *    Returns the size of the NALU (bits between start codes in case of
 *    Annex B.  nalu->buf and nalu->len are filled.  Other field in
 *    nalu-> remain uninitialized (will be taken care of by NALUtoRBSP.
 *
 * \return
 *     0 if there is nothing any more to read (EOF)
 *    -1 in case of any error
 *
 *  \note Side-effect: Returns length of start-code in bytes.
 *
 * \note
 *   GetAnnexbNALU expects start codes at byte aligned positions in the file
 *
 *	\author
 *	Matteo Naccari (adapted from the H.264/AVC decoder reference software)
 */

int AnnexB_packet::getpacket() {
  int info2, info3, pos = 0;
  int StartCodeFound, rewind;
  unsigned char* Buf;
  int LeadingZero8BitsCount = 0, TrailingZero8Bits = 0;

  if ((Buf = (unsigned char*)calloc(nalu->max_size, sizeof(char))) == NULL)
    cout << "GetAnnexbNALU: Buf" << endl;

  frame_bitoffset = 0;

  while (!feof(bits) && (Buf[pos++] = fgetc(bits)) == 0);

  if (feof(bits))
  {
    if (pos == 0)
      return 0;
    else
    {
      printf("GetAnnexbNALU can't read start code\n");
      free(Buf);
      return -1;
    }
  }

  if (Buf[pos - 1] != 1)
  {
    printf("GetAnnexbNALU: no Start Code at the begin of the NALU, return -1\n");
    free(Buf);
    return -1;
  }

  if (pos < 3)
  {
    printf("GetAnnexbNALU: no Start Code at the begin of the NALU, return -1\n");
    free(Buf);
    return -1;
  }
  else if (pos == 3)
  {
    nalu->startcodeprefix_len = 3;
    LeadingZero8BitsCount = 0;
  }
  else
  {
    LeadingZero8BitsCount = pos - 4;
    nalu->startcodeprefix_len = 4;
  }

  //the 1st byte stream NAL unit can has leading_zero_8bits, but subsequent ones are not
  //allowed to contain it since these zeros(if any) are considered trailing_zero_8bits
  //of the previous byte stream NAL unit.
  if (!IsFirstByteStreamNALU && LeadingZero8BitsCount > 0)
  {
    printf("GetAnnexbNALU: The leading_zero_8bits syntax can only be present in the first byte stream NAL unit, return -1\n");
    free(Buf);
    return -1;
  }
  IsFirstByteStreamNALU = 0;

  StartCodeFound = 0;
  info2 = 0;
  info3 = 0;

  while (!StartCodeFound)
  {
    if (feof(bits))
    {
      //Count the trailing_zero_8bits
      while (Buf[pos - 2 - TrailingZero8Bits] == 0)
        TrailingZero8Bits++;
      nalu->len = (pos - 1) - nalu->startcodeprefix_len - LeadingZero8BitsCount - TrailingZero8Bits;
      memcpy(nalu->buf, &Buf[LeadingZero8BitsCount + nalu->startcodeprefix_len], nalu->len);
      nalu->forbidden_bit = (nalu->buf[0] >> 7) & 1;
      nalu->nal_reference_idc = (nalu->buf[0] >> 5) & 3;
      nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;

      free(Buf);
      return pos - 1;
    }
    Buf[pos++] = fgetc(bits);
    info3 = FindStartCode(&Buf[pos - 4], 3);
    if (info3 != 1)
      info2 = FindStartCode(&Buf[pos - 3], 2);
    StartCodeFound = (info2 == 1 || info3 == 1);
  }

  //Count the trailing_zero_8bits
  if (info3 == 1)	//if the detected start code is 00 00 01, trailing_zero_8bits is sure not to be present
  {
    while (Buf[pos - 5 - TrailingZero8Bits] == 0)
      TrailingZero8Bits++;
  }
  // Here, we have found another start code (and read length of startcode bytes more than we should
  // have.  Hence, go back in the file
  rewind = 0;
  if (info3 == 1)
    rewind = -4;
  else if (info2 == 1)
    rewind = -3;
  else
    printf(" Panic: Error in next start code search \n");

  if (0 != fseek(bits, rewind, SEEK_CUR))
  {
    printf("GetAnnexbNALU: Cannot fseek %d in the bit stream file", rewind);
    free(Buf);
    return -1;
  }

  // Here the leading zeros(if any), Start code, the complete NALU, trailing zeros(if any)
  // and the next start code is in the Buf.
  // The size of Buf is pos, pos+rewind are the number of bytes excluding the next
  // start code, and (pos+rewind)-startcodeprefix_len-LeadingZero8BitsCount-TrailingZero8Bits
  // is the size of the NALU.

  nalu->len = (pos + rewind) - nalu->startcodeprefix_len - LeadingZero8BitsCount - TrailingZero8Bits;
  memcpy(nalu->buf, &Buf[LeadingZero8BitsCount + nalu->startcodeprefix_len], nalu->len);
  nalu->forbidden_bit = (nalu->buf[0] >> 7) & 1;
  nalu->nal_reference_idc = (nalu->buf[0] >> 5) & 3;
  nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;

  free(Buf);

  return (pos + rewind);

}
/*!
 *
 * \brief
 *    returns if new start code is found at byte aligned position buf.
 *    new-startcode is of form N 0x00 bytes, followed by a 0x01 byte.
 *
 *  \return
 *     1 if start-code is found or                      \n
 *     0, indicating that there is no start code
 *
 *  \param Buf
 *     pointer to byte-stream
 *  \param zeros_in_startcode
 *     indicates number of 0x00 bytes in start-code.
 *
 *	\author
 *	Matteo Naccari (adapted from the H.264/AVC decoder reference software)
*/
int AnnexB_packet::FindStartCode(unsigned char* Buf, int zeros_in_startcode) {
  int info;
  int i;

  info = 1;
  for (i = 0; i < zeros_in_startcode; i++)
    if (Buf[i] != 0)
      info = 0;

  if (Buf[i] != 1)
    info = 0;
  return info;
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
int AnnexB_packet::writepacket(FILE* f) {

  int BitsWritten = 0;

  assert(nalu != NULL);
  assert(nalu->forbidden_bit == 0);
  assert(f != NULL);
  assert(nalu->startcodeprefix_len == 3 || nalu->startcodeprefix_len == 4);

  // printf ("WriteAnnexbNALU: writing %d bytes w/ startcode_len %d\n", n->len+1, n->startcodeprefix_len);
  if (nalu->startcodeprefix_len > 3)
  {
    putc(0, f);
    BitsWritten = +8;
  }
  putc(0, f);
  putc(0, f);
  putc(1, f);
  BitsWritten += 24;

  nalu->buf[0] = (unsigned char)((nalu->forbidden_bit << 7) | (nalu->nal_reference_idc << 5) | nalu->nal_unit_type);

  if (nalu->len != fwrite(nalu->buf, 1, nalu->len, f))
  {
    printf("Fatal: cannot write %d bytes to bitstream file, exit (-1)\n", nalu->len);
    exit(-1);
  }
  BitsWritten += nalu->len * 8;

  fflush(f);

  return BitsWritten;

}