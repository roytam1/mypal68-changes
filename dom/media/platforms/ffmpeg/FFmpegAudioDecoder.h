/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __FFmpegAACDecoder_h__
#define __FFmpegAACDecoder_h__

#include "FFmpegDataDecoder.h"
#include "FFmpegLibWrapper.h"

namespace mozilla {

template <int V>
class FFmpegAudioDecoder {};

template <>
class FFmpegAudioDecoder<LIBAV_VER>;
DDLoggedTypeNameAndBase(FFmpegAudioDecoder<LIBAV_VER>,
                        FFmpegDataDecoder<LIBAV_VER>);

template <>
class FFmpegAudioDecoder<LIBAV_VER>
    : public FFmpegDataDecoder<LIBAV_VER>,
      public DecoderDoctorLifeLogger<FFmpegAudioDecoder<LIBAV_VER>> {
 public:
  FFmpegAudioDecoder(FFmpegLibWrapper* aLib, TaskQueue* aTaskQueue,
                     const AudioInfo& aConfig);
  virtual ~FFmpegAudioDecoder();

  RefPtr<InitPromise> Init() override;
  void InitCodecContext() override;
  static AVCodecID GetCodecId(const nsACString& aMimeType);
  nsCString GetDescriptionName() const override {
    return NS_LITERAL_CSTRING("ffmpeg audio decoder");
  }

 private:
  MediaResult DoDecode(MediaRawData* aSample, uint8_t* aData, int aSize,
                       bool* aGotFrame, DecodedData& aResults) override;
};

}  // namespace mozilla

#endif  // __FFmpegAACDecoder_h__
