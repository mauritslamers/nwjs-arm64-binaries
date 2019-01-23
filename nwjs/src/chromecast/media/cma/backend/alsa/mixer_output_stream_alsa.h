// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_MEDIA_CMA_BACKEND_ALSA_MIXER_OUTPUT_STREAM_ALSA_H_
#define CHROMECAST_MEDIA_CMA_BACKEND_ALSA_MIXER_OUTPUT_STREAM_ALSA_H_

#include <cstdint>
#include <vector>

#include "base/macros.h"
#include "chromecast/media/cma/backend/mixer_output_stream.h"

#include <alsa/asoundlib.h>

namespace chromecast {
namespace media {

class AlsaWrapper;

// MixerOutputStream implementation for ALSA
class MixerOutputStreamAlsa : public MixerOutputStream {
 public:
  MixerOutputStreamAlsa();
  ~MixerOutputStreamAlsa() override;

  void SetAlsaWrapperForTest(std::unique_ptr<AlsaWrapper> alsa);

  // MixerOutputStream interface.
  bool IsFixedSampleRate() override;
  bool Start(int requested_sample_rate, int channels) override;
  bool GetTimeUntilUnderrun(base::TimeDelta* result) override;
  int GetSampleRate() override;
  MediaPipelineBackend::AudioDecoder::RenderingDelay GetRenderingDelay()
      override;
  bool Write(const float* data,
             int data_size,
             bool* out_playback_interrupted) override;
  void Stop() override;

 private:
  // Reads the buffer size, period size, start threshold, and avail min value
  // from the provided command line flags or uses default values if no flags are
  // provided.
  void DefineAlsaParameters();

  // Takes the provided ALSA config and sets all ALSA output hardware/software
  // playback parameters.  It will try to select sane fallback parameters based
  // on what the output hardware supports and will log warnings if it does so.
  // If any ALSA function returns an unexpected error code, the error code will
  // be returned by this function. Otherwise, it will return 0.
  int SetAlsaPlaybackParams(int requested_rate);

  // Determines output sample rate based on the requested rate and the sample
  // rate the device supports.
  int DetermineOutputRate(int requested_rate);

  void UpdateRenderingDelay(int newly_pushed_frames);

  // Value of --alsa-fixed-output-sample-rate flag if any.
  int fixed_sample_rate_ = kInvalidSampleRate;

  std::unique_ptr<AlsaWrapper> alsa_;

  snd_pcm_t* pcm_ = nullptr;
  snd_pcm_hw_params_t* pcm_hw_params_ = nullptr;
  snd_pcm_status_t* pcm_status_ = nullptr;
  snd_pcm_format_t pcm_format_ = SND_PCM_FORMAT_UNKNOWN;

  int num_output_channels_ = 0;
  int sample_rate_ = kInvalidSampleRate;

  // User-configurable ALSA parameters. This caches the results, so the code
  // only has to interact with the command line parameters once.
  snd_pcm_uframes_t alsa_buffer_size_ = 0;
  snd_pcm_uframes_t alsa_period_size_ = 0;
  snd_pcm_uframes_t alsa_start_threshold_ = 0;
  snd_pcm_uframes_t alsa_avail_min_ = 0;

  MediaPipelineBackend::AudioDecoder::RenderingDelay rendering_delay_;

  std::vector<uint8_t> output_buffer_;

  DISALLOW_COPY_AND_ASSIGN(MixerOutputStreamAlsa);
};

}  // namespace media
}  // namespace chromecast

#endif  // CHROMECAST_MEDIA_CMA_BACKEND_ALSA_MIXER_OUTPUT_STREAM_ALSA_H_