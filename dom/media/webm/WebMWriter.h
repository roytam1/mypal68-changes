/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WebMWriter_h_
#define WebMWriter_h_

#include "ContainerWriter.h"
#include "nsAutoPtr.h"

namespace mozilla {

class EbmlComposer;

// Vorbis meta data structure
class VorbisMetadata : public TrackMetadataBase {
 public:
  nsTArray<uint8_t> mData;
  int32_t mChannels;
  float mSamplingFrequency;
  MetadataKind GetKind() const override { return METADATA_VORBIS; }
};

// VP8 meta data structure
class VP8Metadata : public TrackMetadataBase {
 public:
  int32_t mWidth;
  int32_t mHeight;
  int32_t mDisplayWidth;
  int32_t mDisplayHeight;
  MetadataKind GetKind() const override { return METADATA_VP8; }
};

/**
 * WebM writer helper
 * This class accepts encoder to set audio or video meta data or
 * encoded data to ebml Composer, and get muxing data through GetContainerData.
 * The ctor/dtor run in the MediaRecorder thread, others run in MediaEncoder
 * thread.
 */
class WebMWriter : public ContainerWriter {
 public:
  // aTrackTypes indicate this muxer should multiplex into Video only or A/V
  // foramt. Run in MediaRecorder thread
  explicit WebMWriter(uint32_t aTrackTypes);
  virtual ~WebMWriter();

  // WriteEncodedTrack inserts raw packets into WebM stream.
  nsresult WriteEncodedTrack(const EncodedFrameContainer& aData,
                             uint32_t aFlags = 0) override;

  // GetContainerData outputs multiplexing data.
  // aFlags indicates the muxer should enter into finished stage and flush out
  // queue data.
  nsresult GetContainerData(nsTArray<nsTArray<uint8_t> >* aOutputBufs,
                            uint32_t aFlags = 0) override;

  // Assign metadata into muxer
  nsresult SetMetadata(TrackMetadataBase* aMetadata) override;

 private:
  nsAutoPtr<EbmlComposer> mEbmlComposer;

  // Indicate what kind of meta data needed in the writer.
  // If this value become 0, it means writer can start to generate header.
  uint8_t mMetadataRequiredFlag;
};

}  // namespace mozilla

#endif
