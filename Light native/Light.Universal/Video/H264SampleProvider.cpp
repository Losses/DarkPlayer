//*****************************************************************************
//
//	Copyright 2015 Microsoft Corporation
//
//	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
//
//	http ://www.apache.org/licenses/LICENSE-2.0
//
//	Unless required by applicable law or agreed to in writing, software
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//	See the License for the specific language governing permissions and
//	limitations under the License.
//
//*****************************************************************************

#include "pch.h"
#ifdef ENABLE_VIDEO
#include "H264SampleProvider.h"
#include "InternalByteBuffer.h"

using namespace Light::Video;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;

H264SampleProvider::H264SampleProvider(
	FFmpegReader^ reader,
	AVFormatContext* avFormatCtx,
	AVCodecContext* avCodecCtx)
	: MediaSampleProvider(reader, avFormatCtx, avCodecCtx)
{
}


H264SampleProvider::~H264SampleProvider()
{
}

HRESULT H264SampleProvider::WriteAVPacketToStream(DataWriter^ dataWriter, AVPacket* avPacket)
{
	HRESULT hr = S_OK;
	// On a KeyFrame, write the SPS and PPS
	if (avPacket->flags & AV_PKT_FLAG_KEY)
	{
		hr = GetSPSAndPPSBuffer(dataWriter);
	}

	if (SUCCEEDED(hr))
	{
		// Convert the packet to NAL format
		hr = WriteNALPacket(dataWriter, avPacket);
	}

	// We have a complete frame
	return hr;
}

HRESULT H264SampleProvider::GetSPSAndPPSBuffer(DataWriter^ dataWriter)
{
	HRESULT hr = S_OK;
	int spsLength = 0;
	int ppsLength = 0;

	// Get the position of the SPS
	if (m_pAvCodecCtx->extradata == nullptr && m_pAvCodecCtx->extradata_size < 8)
	{
		// The data isn't present
		hr = E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		byte* spsPos = m_pAvCodecCtx->extradata + 8;
		spsLength = spsPos[-1];

		if (m_pAvCodecCtx->extradata_size < (8 + spsLength))
		{
			// We don't have a complete SPS
			hr = E_FAIL;
		}
		else
		{
			auto vSPS = CreateInternalBuffer(spsPos, spsLength);//ref new Platform::Array<uint8_t>(spsPos, spsLength);

			// Write the NAL unit for the SPS
			dataWriter->WriteByte(0);
			dataWriter->WriteByte(0);
			dataWriter->WriteByte(0);
			dataWriter->WriteByte(1);

			// Write the SPS
			dataWriter->WriteBuffer(vSPS);
		}
	}

	if (SUCCEEDED(hr))
	{
		if (m_pAvCodecCtx->extradata_size < (8 + spsLength + 3))
		{
			hr = E_FAIL;
		}

		if (SUCCEEDED(hr))
		{
			byte* ppsPos = m_pAvCodecCtx->extradata + 8 + spsLength + 3;
			ppsLength = ppsPos[-1];

			if (m_pAvCodecCtx->extradata_size < (8 + spsLength + 3 + ppsLength))
			{
				hr = E_FAIL;
			}
			else
			{
				auto vPPS = CreateInternalBuffer(ppsPos, ppsLength);//ref new Platform::Array<uint8_t>(ppsPos, ppsLength);

				// Write the NAL unit for the PPS
				dataWriter->WriteByte(0);
				dataWriter->WriteByte(0);
				dataWriter->WriteByte(0);
				dataWriter->WriteByte(1);

				// Write the PPS
				dataWriter->WriteBuffer(vPPS);
			}
		}
	}

	return hr;
}

// Write out an H.264 packet converting stream offsets to start-codes
HRESULT H264SampleProvider::WriteNALPacket(DataWriter^ dataWriter, AVPacket* avPacket)
{
	HRESULT hr = S_OK;
	int32 index = 0;
	int32 size = 0;

	do
	{
		// Make sure we have enough data
		if (avPacket->size < (index + 4))
		{
			hr = E_FAIL;
			break;
		}

		// Grab the size of the blob
		size = (avPacket->data[index] << 24) + (avPacket->data[index + 1] << 16) + (avPacket->data[index + 2] << 8) + avPacket->data[index + 3];

		// Write the NAL unit to the stream
		dataWriter->WriteByte(0);
		dataWriter->WriteByte(0);
		dataWriter->WriteByte(0);
		dataWriter->WriteByte(1);
		index += 4;

		if (avPacket->size < (index + size))
		{
			hr = E_FAIL;
			break;
		}
		// Write the rest of the packet to the stream
		auto vBuffer = CreateInternalBuffer(&(avPacket->data[index]), size);//ref new Platform::Array<uint8_t>(&(avPacket->data[index]), size);
		dataWriter->WriteBuffer(vBuffer);
		index += size;
	} while (index < avPacket->size);

	return hr;
}


#endif