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

#include "Movie.h"

#include <UmfeldFunctionsAdditional.h>

// TODO look into audio processing
// TODO look into camera access
// TODO implement `MovieListener` including callback

using namespace umfeld;

#if !defined(DISABLE_GRAPHICS) && !defined(DISABLE_VIDEO)

#include "Umfeld.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
}

Movie::Movie(const std::string& filename, const int channels) {
    if (init_from_file(filename, channels) >= 0) {
        calculateFrameDuration();
        keepRunning    = true;
        isPlaying      = false;
        playbackThread = std::thread(&Movie::playbackLoop, this);
    } else {
        std::cerr << "+++ Movie: ERROR: could not initialize from file" << std::endl;
    }
}

int Movie::init_from_file(const std::string& filename, int _channels) {
    // Open the input file
    const std::string absolute_path = resolveDataPath(filename);
    formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, absolute_path.c_str(), nullptr, nullptr) != 0) {
        error("+++ Movie: ERROR: Could not open file: ", filename);
        return -1;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        std::cerr << "+++ Movie: ERROR: Could not find stream information" << std::endl;
        return -1;
    }

    // Find the first video stream
    videoStreamIndex = -1;
    audioStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        //        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
        //            videoStreamIndex = i;
        //            break;
        //        }
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStreamIndex < 0) {
            videoStreamIndex = static_cast<int>(i);
        } else if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audioStreamIndex < 0) {
            audioStreamIndex = static_cast<int>(i);
        }
    }

    if (videoStreamIndex == -1) {
        std::cerr << "+++ Movie: ERROR: Could not find a video stream" << std::endl;
        return -1;
    }

    if (audioStreamIndex == -1) {
        std::cerr << "+++ Movie: ERROR: Could not find a video stream" << std::endl;
        return -1;
    }

    // Get a pointer to the codec context for the video stream
    const AVCodecParameters* codecParameters = formatContext->streams[videoStreamIndex]->codecpar;
    const AVCodec*           codec           = avcodec_find_decoder(codecParameters->codec_id);
    videoCodecContext                        = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(videoCodecContext, codecParameters);
    if (avcodec_open2(videoCodecContext, codec, nullptr) < 0) {
        std::cerr << "+++ Movie: ERROR: Could not open codec" << std::endl;
        return -1;
    }

    // Initialize audio codec
    const AVCodec* audioCodec = avcodec_find_decoder(formatContext->streams[audioStreamIndex]->codecpar->codec_id);
    audioCodecContext         = avcodec_alloc_context3(audioCodec);
    avcodec_parameters_to_context(audioCodecContext, formatContext->streams[audioStreamIndex]->codecpar);
    avcodec_open2(audioCodecContext, audioCodec, nullptr);

    // retrieve movie framerate
    const AVRational frame_rate     = formatContext->streams[videoStreamIndex]->avg_frame_rate;
    const double     frame_duration = 1.0 / (frame_rate.num / static_cast<double>(frame_rate.den));

    // Determine the pixel channels and number of channels based on input file
    const AVPixelFormat       src_pix_fmt = videoCodecContext->pix_fmt;
    AVPixelFormat             dst_pix_fmt;
    const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(src_pix_fmt);
    if (_channels < 0) {
        _channels   = 4;
        dst_pix_fmt = AV_PIX_FMT_RGBA;
        //        std::cout << "not looking for channels. defaulting to RGBA ( 4 channels )" << std::endl;
    } else if (desc && desc->nb_components == 4) {
        _channels   = 4;
        dst_pix_fmt = AV_PIX_FMT_RGBA;
        //        std::cout << "found RGBA video" << std::endl;
    } else {
        _channels   = 3;
        dst_pix_fmt = AV_PIX_FMT_RGB24;
        //        std::cout << "found RGB video" << std::endl;
    }

    // Create a sws context for the conversion
    swsContext = sws_getContext(
        videoCodecContext->width, videoCodecContext->height, src_pix_fmt,
        videoCodecContext->width, videoCodecContext->height, dst_pix_fmt,
        SWS_FAST_BILINEAR,
        //            SWS_BICUBIC,
        nullptr, nullptr, nullptr);

    if (!swsContext) {
        std::cerr << "+++ Movie: ERROR: Failed to create SwScale context" << std::endl;
        return -1;
    }

    // Allocate an AVFrame structure
    frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "+++ Movie: ERROR: Failed to allocate frame" << std::endl;
        return -1;
    }

    // Allocate an AVFrame structure for the converted frame
    convertedFrame = av_frame_alloc();
    if (!convertedFrame) {
        std::cerr << "+++ Movie: ERROR: Failed to allocate converted frame" << std::endl;
        return -1;
    }

    const int numBytes = av_image_get_buffer_size(dst_pix_fmt,
                                                  videoCodecContext->width,
                                                  videoCodecContext->height,
                                                  1);
    buffer             = static_cast<uint8_t*>(av_malloc(numBytes * sizeof(uint8_t)));
    av_image_fill_arrays(convertedFrame->data,
                         convertedFrame->linesize,
                         buffer,
                         dst_pix_fmt,
                         videoCodecContext->width,
                         videoCodecContext->height,
                         1);
    packet = av_packet_alloc();

    PImage::init(reinterpret_cast<uint32_t*>(convertedFrame->data[0]),
                 videoCodecContext->width,
                 videoCodecContext->height);

#ifndef OMIT_PRINT_MOVIE_INFO
    std::cout << "+++ Movie: dimensions    : " << videoCodecContext->width << ", " << videoCodecContext->height << std::endl;
    std::cout << "+++ Movie: channels      : " << _channels << std::endl;
    std::cout << "+++ Movie: framerate     : " << frame_rate.num / frame_rate.den << std::endl;
    std::cout << "+++ Movie: frame duration: " << frame_duration << std::endl;
#endif

    return 1;
}

Movie::~Movie() {
    keepRunning = false;
    if (playbackThread.joinable()) {
        playbackThread.join();
    }
    av_freep(&buffer);
    av_frame_free(&frame);
    av_frame_free(&convertedFrame);
    avcodec_free_context(&audioCodecContext);
    avcodec_free_context(&videoCodecContext);
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    av_packet_free(&packet);
    sws_freeContext(swsContext);
}

void Movie::calculateFrameDuration() {
    const AVRational frame_rate = formatContext->streams[videoStreamIndex]->avg_frame_rate;
    frameDuration               = 1.0 / (frame_rate.num / static_cast<double>(frame_rate.den));
}

void Movie::playbackLoop() {
    while (keepRunning) {
        if (isPlaying) {
            auto frame_start = std::chrono::steady_clock::now();
            if (available()) {
                if (processFrame()) {
                    // TODO flag that a texture reload is required
                    // TODO callback with `MovieListener`
                    //                } else {
                    //                    // Check if the end of the video is reached
                    //                    bool isEndOfVideo = false;
                    //                    if (isEndOfVideo) {
                    //                        std::cout << "+++ end of video reached" << std::endl;
                    //                        if (isLooping) {
                    //                            // Seek back to the start of the video
                    //                            av_seek_frame(formatContext, videoStream, 0, AVSEEK_FLAG_BACKWARD);
                    //                            mFrameCounter = 0; // Reset frame counter
                    //                        } else {
                    //                            // Stop playback if not looping
                    //                            pause();
                    //                        }
                    //                    }
                }
            }

            auto                          frame_end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed   = frame_end - frame_start;

            if (elapsed.count() < frameDuration) {
                std::this_thread::sleep_for(std::chrono::duration<double>(frameDuration - elapsed.count()));
            }
        } else {
            // If not playing, sleep for a short duration to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void Movie::play() {
    isPlaying = true;
}

void Movie::pause() {
    isPlaying = false;
}

bool Movie::available() {
    const int ret = av_read_frame(formatContext, packet);
    if (ret >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            mVideoFrameAvailable = true;
            avcodec_send_packet(videoCodecContext, packet);
        } else if (packet->stream_index == audioStreamIndex) {
            // Decode audio frame
            avcodec_send_packet(audioCodecContext, packet);
            while (avcodec_receive_frame(audioCodecContext, frame) == 0) {
                if (!swrCtx) {
#if LIBAVUTIL_VERSION_MAJOR >= 57
                    AVChannelLayout in_ch_layout, out_ch_layout;

                    if (av_channel_layout_copy(&in_ch_layout, &audioCodecContext->ch_layout) < 0) {
                        fprintf(stderr, "Failed to copy input channel layout\n");
                        break;
                    }

                    av_channel_layout_default(&out_ch_layout, audioCodecContext->ch_layout.nb_channels);

                    if (swr_alloc_set_opts2(
                            &swrCtx,
                            &out_ch_layout, AV_SAMPLE_FMT_FLT, audioCodecContext->sample_rate,
                            &in_ch_layout, static_cast<AVSampleFormat>(frame->format), frame->sample_rate,
                            0, NULL) < 0) {
                        fprintf(stderr, "Failed to allocate and configure SwrContext\n");
                        av_channel_layout_uninit(&in_ch_layout);
                        av_channel_layout_uninit(&out_ch_layout);
                        break;
                    }

                    if (swr_init(swrCtx) < 0) {
                        fprintf(stderr, "Failed to initialize SwrContext\n");
                        av_channel_layout_uninit(&in_ch_layout);
                        av_channel_layout_uninit(&out_ch_layout);
                        break;
                    }

                    av_channel_layout_uninit(&in_ch_layout);
                    av_channel_layout_uninit(&out_ch_layout);
#else
                    uint64_t in_ch_layout  = audioCodecContext->channel_layout;
                    uint64_t out_ch_layout = av_get_default_channel_layout(audioCodecContext->channels);

                    swrCtx = swr_alloc_set_opts(
                        NULL,
                        out_ch_layout, AV_SAMPLE_FMT_FLT, audioCodecContext->sample_rate,
                        in_ch_layout, (AVSampleFormat) frame->channels, frame->sample_rate,
                        0, NULL);

                    if (!swrCtx || swr_init(swrCtx) < 0) {
                        fprintf(stderr, "Failed to initialize SwrContext\n");
                        break;
                    }
#endif
                }

#if LIBAVUTIL_VERSION_MAJOR >= 57
                int channels    = audioCodecContext->ch_layout.nb_channels;
#else
                int channels    = audioCodecContext->channels;
#endif
                int out_samples = av_rescale_rnd(
                    swr_get_delay(swrCtx, frame->sample_rate) + frame->nb_samples,
                    frame->sample_rate, frame->sample_rate, AV_ROUND_UP);

                auto     output_buffer = static_cast<float*>(av_malloc(out_samples * channels * sizeof(float)));
                uint8_t* out[]         = {reinterpret_cast<uint8_t*>(output_buffer)};

                int samples_converted = swr_convert(
                    swrCtx, out, out_samples,
                    (const uint8_t**) frame->extended_data, frame->nb_samples);

                if (samples_converted < 0) {
                    fprintf(stderr, "Error while converting audio\n");
                    av_free(output_buffer);
                    break;
                }

                // Process the float audio samples
                // process_audio_samples(output_buffer, samples_converted, frame->ch_layout.nb_channels);
                // std::cout << "+++ audio samples : " << samples_converted << std::endl;
                // std::cout << "+++       channels: " << frame->channels << std::endl;

                if (fListener) {
#if LIBAVUTIL_VERSION_MAJOR >= 57
                    fListener->movieAudioEvent(this, output_buffer, samples_converted, frame->ch_layout.nb_channels);
#else
                    fListener->movieAudioEvent(this, output_buffer, samples_converted, frame->channels);
#endif
                }

                // Free the output buffer
                av_free(output_buffer);

                av_frame_unref(frame);
            }
        }
        av_packet_unref(packet);
    } else if (ret == AVERROR_EOF) {
        //        std::cout << "AVERROR_EOF" << std::endl;
        if (isLooping) {
            // Seek back to the start of the video
            av_seek_frame(formatContext, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD);
            mFrameCounter = 0; // Reset frame counter
        } else {
            // Stop playback if not looping
            pause();
        }
    } else {
        char err_buf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, err_buf, AV_ERROR_MAX_STRING_SIZE);
        printf("Error occurred: %s\n", err_buf);
    }
    return mVideoFrameAvailable;
}

bool Movie::processFrame() {
    if (mVideoFrameAvailable) {
        const int ret = avcodec_receive_frame(videoCodecContext, frame);
        if (ret == 0) {
            // Successfully received a frame
            mFrameCounter++;

            // Convert data to RGBA or RGB
            sws_scale(swsContext,
                      frame->data,
                      frame->linesize,
                      0,
                      frame->height,
                      convertedFrame->data,
                      convertedFrame->linesize);

            av_frame_unref(frame);
            mVideoFrameAvailable = false;
            return true;
        } else {
            if (ret == AVERROR_EOF) {
                // Handle end of stream
                std::cout << "+++ AVERROR_EOF" << std::endl;
            } else if (ret == AVERROR(EAGAIN)) {
                // No frame available right now, try again later
                //                std::cout << "+++ AVERROR(EAGAIN)" << std::endl;
            } else {
                // std::cerr << "+++ Movie: ERROR: Error receiving frame: " << av_err2str(ret));
            }
            av_frame_unref(frame);
            return false;
        }
    }
    return false;
}

void Movie::reload(PGraphics* graphics) {
    if (graphics == nullptr) {
        return;
    }

    if (convertedFrame == nullptr) {
        return;
    }

    pixels = reinterpret_cast<uint32_t*>(convertedFrame->data[0]);
    update_full_internal(graphics);
}

void Movie::set_listener(MovieListener* listener) { fListener = listener; }

bool Movie::read(PGraphics* graphics) {
    if (graphics == nullptr) {
        return false;
    }

    if (convertedFrame == nullptr) {
        return false;
    }

    if (!processFrame()) {
        return false; // No frame available or error processing frame
    }

    pixels = reinterpret_cast<uint32_t*>(convertedFrame->data[0]);
    update_full_internal(graphics);
    return true;
}

// Example of frameRate() method
float Movie::frameRate() const {
    if (formatContext == nullptr) {
        return 0;
    }

    const AVRational frame_rate = formatContext->streams[videoStreamIndex]->avg_frame_rate;
    return static_cast<float>(frame_rate.num) / static_cast<float>(frame_rate.den);
}

// Example of setting playback speed
void Movie::speed(const float factor) {
    //    std::lock_guard<std::mutex> lock(mutex);
    frameDuration /= factor; // Adjust frame duration based on speed factor
}

float Movie::duration() const {
    if (formatContext == nullptr) {
        return 0;
    }

    return static_cast<float>(formatContext->duration) / AV_TIME_BASE;
}

void Movie::jump(const float seconds) const {
    if (formatContext == nullptr) {
        return;
    }

    const int64_t timestamp = static_cast<int64_t>(seconds) * AV_TIME_BASE;
    av_seek_frame(formatContext, videoStreamIndex, timestamp, AVSEEK_FLAG_ANY);
    // Reset any buffers or states as needed
}

float Movie::time() const {
    //    std::lock_guard<std::mutex> lock(mutex);
    return static_cast<float>(mFrameCounter) / frameRate();
}

void Movie::loop() {
    isLooping = true;
}

void Movie::noLoop() {
    isLooping = false;
}
#else
Movie::Movie(const std::string& filename, int _channels) : PImage() {
    error("Movie - ERROR: video is disabled");
}

Movie::~Movie() {}

bool Movie::available() { return false; }

bool read(PGraphics* graphics = g) { return false; }

int Movie::init_from_file(const std::string& filename, int _channels) { return -1; }

void Movie::playbackLoop() {}

void Movie::calculateFrameDuration() {}

void Movie::play() {}

void Movie::pause() {}

bool Movie::processFrame() { return false; }

#endif // DISABLE_GRAPHICS && DISABLE_VIDEO