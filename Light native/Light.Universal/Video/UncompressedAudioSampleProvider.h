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

#pragma once
#ifdef ENABLE_VIDEO
#include "MediaSampleProvider.h"

extern "C"
{
#include <libswresample/swresample.h>
}

namespace Light {
	namespace Video {
		ref class UncompressedAudioSampleProvider : MediaSampleProvider
		{
		public:
			virtual ~UncompressedAudioSampleProvider();

		internal:
			UncompressedAudioSampleProvider(
				FFmpegReader^ reader,
				AVFormatContext* avFormatCtx,
				AVCodecContext* avCodecCtx);
			virtual HRESULT WriteAVPacketToStream(Windows::Storage::Streams::DataWriter^ writer, AVPacket* avPacket) override;
			virtual HRESULT DecodeAVPacket(Windows::Storage::Streams::DataWriter^ dataWriter, AVPacket* avPacket) override;
			virtual HRESULT AllocateResources() override;

		private:
			AVFrame* m_pAvFrame;
			SwrContext* m_pSwrCtx;
		};
	}

}
#endif