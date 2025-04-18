/*
 * Umfeld
 *
 * This file is part of the *Umfeld* library (https://github.com/dennisppaul/umfeld).
 * Copyright (c) 2025 Dennis P Paul.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <atomic>
#include <thread>

#include "PImage.h"

#ifndef DISABLE_GRAPHICS
#ifndef DISABLE_VIDEO
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}
#endif // DISABLE_VIDEO
#endif // DISABLE_GRAPHICS

namespace umfeld {

    class Movie;

    class MovieListener {
    public:
        virtual ~MovieListener() = default;
        // virtual void movieEvent(Movie m)                                                     = 0;
        virtual void movieVideoEvent(Movie* m, float* audio_buffer, int length, int channels) = 0;
        virtual void movieAudioEvent(Movie* m, float* audio_buffer, int length, int channels) = 0;
    };

    extern PGraphics* g;

    class Movie final : public PImage {
    public:
        explicit Movie(const std::string& filename, int channels = -1);

        bool  available();
        float duration() const;
        float frameRate() const;
        void  jump(float seconds) const;
        void  loop();
        void  noLoop();
        void  pause();
        void  play();
        bool  read(PGraphics* graphics = g);
        void  speed(float factor);
        void  stop() { pause(); }
        float time() const;
        void  reload(PGraphics* graphics = g);
        void  set_listener(MovieListener* listener);

        ~Movie() override;

    private:
        std::atomic<bool> isLooping            = false;
        bool              mVideoFrameAvailable = false;
        std::thread       playbackThread;
        std::atomic<bool> keepRunning{};
        std::atomic<bool> isPlaying{};
        double            frameDuration{}; // Duration of each frame in seconds
#ifndef DISABLE_GRAPHICS
#ifndef DISABLE_VIDEO
        uint8_t*         buffer{};
        AVFrame*         frame{};
        AVFrame*         convertedFrame{};
        AVCodecContext*  videoCodecContext{};
        AVCodecContext*  audioCodecContext{};
        AVFormatContext* formatContext{};
        AVPacket*        packet{};
        SwsContext*      swsContext{};
        SwrContext*      swrCtx{};
        int              videoStreamIndex{};
        int              audioStreamIndex{};
        int              mFrameCounter = 0;
        MovieListener*   fListener{};
#endif // DISABLE_VIDEO
#endif // DISABLE_GRAPHICS

        int init_from_file(const std::string& filename, int _channels = -1);

        void playbackLoop();

        void calculateFrameDuration();

        bool processFrame();
    };

} // namespace umfeld
