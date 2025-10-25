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

#ifdef DISABLE_GRAPHICS
#error "DISABLE_GRAPHICS must be set to ON"
#endif

#include "Capture.h"

#if defined(ENABLE_CAPTURE) && !defined(DISABLE_GRAPHICS) && !defined(DISABLE_VIDEO)
#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>
#include <libavutil/log.h>
#ifdef __cplusplus
}
#endif
#endif // ENABLE_CAPTURE && !DISABLE_GRAPHICS && !DISABLE_VIDEO

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <thread>

namespace umfeld {
#if defined(ENABLE_CAPTURE) && !defined(DISABLE_GRAPHICS) && !defined(DISABLE_VIDEO)
    static const char* get_platform_inputformat() {
#ifdef _WIN32
        return "dshow";
#elif __linux__
        return "v4l2";
#elif __APPLE__
        return "avfoundation";
#endif
    }

    Capture::Capture() {
        keepRunning    = true;
        isPlaying      = false;
        playbackThread = std::thread(&Capture::playbackLoop, this);
    }

    bool Capture::init(const char* device_name,
                       const char* resolution,
                       const char* frame_rate,
                       const char* pixel_format) {
        const int result = connect(device_name,
                                   resolution,
                                   frame_rate,
                                   pixel_format);
        if (result != 0) {
            std::cerr << "Failed to connect to camera" << std::endl;
            return false;
        }
        fDeviceName = device_name;
        // TODO not sure how smart this is … better if the devices decides?
        frameDuration  = 1.0f / (frame_rate ? std::stof(frame_rate) : 30.0f);
        fIsInitialized = true;
        return true;
    }

    bool Capture::read(PGraphics* graphics) {
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

    void Capture::reload(PGraphics* graphics) {
        if (graphics == nullptr) {
            return;
        }

        if (convertedFrame == nullptr) {
            return;
        }

        pixels = reinterpret_cast<uint32_t*>(convertedFrame->data[0]);
        update_full_internal(graphics);
    }

    int Capture::connect(const char* device_name,
                         const char* resolution,
                         const char* frame_rate,
                         const char* pixel_format) {
        register_all_devices();

        // Set the input channels for your platform:
        const AVInputFormat* inputFormat = av_find_input_format(get_platform_inputformat());
#ifdef _WIN32
        auto default_device_name = "video=Integrated Camera";
#elif __linux__
        auto default_device_name = "/dev/video0";
#elif __APPLE__
        auto default_device_name = "0";
#endif

        std::string device_name_str;
        if (device_name == nullptr) {
            device_name_str = default_device_name;
        } else {
            device_name_str = device_name;
        }
        const auto deviceName = device_name_str.c_str();

        formatContext = nullptr;

        options = nullptr;
        if (resolution != nullptr) {
            av_dict_set(&options, "video_size", resolution, 0);
        }
        if (frame_rate != nullptr) {
            av_dict_set(&options, "framerate", frame_rate, 0);
        }
        if (pixel_format != nullptr) {
            av_dict_set(&options, "pixel_format", pixel_format, 0);
        }

        av_dict_set(&options, "probesize", "10000000", 0);      // 1MB probe size
        av_dict_set(&options, "analyzeduration", "3000000", 0); // 5 seconds analysis duration

        // Open the camera
        std::cout << "+++ Capture: opening camera: \"" << deviceName << "\"" << std::endl;

#if LIBAVFORMAT_VERSION_MAJOR >= 60
        if (avformat_open_input(&formatContext, deviceName, inputFormat, &options) != 0) {
            std::cerr << "Failed to open camera" << std::endl;
            av_dict_free(&options);
            return -1;
        }
#else
        if (avformat_open_input(&formatContext, deviceName, const_cast<AVInputFormat*>(inputFormat), &options) != 0) {
            std::cerr << "Failed to open camera" << std::endl;
            av_dict_free(&options);
            return -1;
        }
#endif

        // Retrieve stream information
        if (avformat_find_stream_info(formatContext, nullptr) < 0) {
            std::cerr << "Couldn't find stream information" << std::endl;
            return -1;
        }

        // Find the first video stream
        videoStreamIndex = -1;
        for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
            if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndex = i;
                break;
            }
        }

        if (videoStreamIndex == -1) {
            std::cerr << "Couldn't find a video stream" << std::endl;
            return -1;
        }

        // Get the codec context for the video stream
        const AVCodecParameters* codecParams = formatContext->streams[videoStreamIndex]->codecpar;
        const AVCodec*           codec       = avcodec_find_decoder(codecParams->codec_id);
        if (!codec) {
            std::cerr << "Unsupported codec" << std::endl;
            return -1;
        }

        codecContext = avcodec_alloc_context3(codec);
        if (!codecContext) {
            std::cerr << "Couldn't allocate codec context" << std::endl;
            return -1;
        }

        if (avcodec_parameters_to_context(codecContext, codecParams) < 0) {
            std::cerr << "Couldn't copy codec parameters to context" << std::endl;
            return -1;
        }

        // Open codec
        if (avcodec_open2(codecContext, codec, nullptr) < 0) {
            std::cerr << "Couldn't open codec" << std::endl;
            return -1;
        }

        // Allocate video frame
        frame = av_frame_alloc();

        // Determine the pixel channels and number of channels based on input file
        constexpr int           default_channels_RGBA = 4;
        constexpr AVPixelFormat dst_pix_fmt           = AV_PIX_FMT_RGBA;
        const AVPixelFormat     src_pix_fmt           = codecContext->pix_fmt;

        // AVPixelFormat             dst_pix_fmt;
        // const AVPixFmtDescriptor* desc      = av_pix_fmt_desc_get(src_pix_fmt);
        // if (_channels < 0) {
        //     _channels   = 4;
        //     dst_pix_fmt = AV_PIX_FMT_RGBA;
        //     std::cout << "not looking for channels. defaulting to RGBA ( 4 channels )" << std::endl;
        // } else if (desc && desc->nb_components == 4) {
        //     _channels   = 4;
        //     dst_pix_fmt = AV_PIX_FMT_RGBA;
        //     std::cout << "found RGBA video" << std::endl;
        // } else {
        //     _channels   = 3;
        //     dst_pix_fmt = AV_PIX_FMT_RGB24;
        //     std::cout << "found RGB video" << std::endl;
        // }

        swsContext = sws_getContext(
            codecContext->width, codecContext->height, src_pix_fmt,
            codecContext->width, codecContext->height, dst_pix_fmt,
            SWS_FAST_BILINEAR,
            nullptr, nullptr, nullptr);

        // swsContext = sws_getContext(
        //     codecContext->width, codecContext->height, codecContext->pix_fmt,
        //     codecContext->width, codecContext->height, AV_PIX_FMT_RGB24,
        //     SWS_BILINEAR, nullptr, nullptr, nullptr);

        convertedFrame = av_frame_alloc();
        if (!convertedFrame) {
            std::cerr << "+++ Movie: ERROR: Failed to allocate converted frame" << std::endl;
            return -1;
        }

        const int numBytes = av_image_get_buffer_size(dst_pix_fmt,
                                                      codecContext->width,
                                                      codecContext->height,
                                                      1);
        buffer             = static_cast<uint8_t*>(av_malloc(numBytes * sizeof(uint8_t)));
        av_image_fill_arrays(convertedFrame->data,
                             convertedFrame->linesize,
                             buffer,
                             dst_pix_fmt,
                             codecContext->width,
                             codecContext->height,
                             1);
        // Prepare for frame conversion (if you need to scale or convert pixel channels)
        // swsContext         = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
        //                                     codecContext->width, codecContext->height, AV_PIX_FMT_RGB24,
        //                                     SWS_BILINEAR, nullptr, nullptr, nullptr);
        // rgbFrame           = av_frame_alloc();
        // const int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);
        // buffer             = static_cast<uint8_t*>(av_malloc(numBytes * sizeof(uint8_t)));
        // av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_RGB24, codecContext->width, codecContext->height, 1);
        av_image_fill_arrays(convertedFrame->data,
                             convertedFrame->linesize,
                             buffer,
                             dst_pix_fmt,
                             codecContext->width,
                             codecContext->height,
                             1);
        packet = av_packet_alloc();

        PImage::init(reinterpret_cast<uint32_t*>(buffer),
                     codecContext->width,
                     codecContext->height);
        return 0;
    }

    Capture::~Capture() {
        keepRunning = false;
        if (playbackThread.joinable()) {
            playbackThread.join();
        }
        av_dict_free(&options);
        av_free(buffer);
        av_frame_free(&frame);
        av_frame_free(&convertedFrame);
        // av_frame_free(&rgbFrame);
        av_packet_free(&packet);
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
    }

    void Capture::playbackLoop() {
        while (keepRunning) {
            if (isPlaying) {
                auto frame_start = std::chrono::steady_clock::now();
                if (available()) {
                    if (processFrame()) {
                        // TODO flag that a texture reload is required
                        if (listener) {
                            listener->captureEvent(this);
                        }
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

    bool Capture::available() {
        if (convertedFrame == nullptr) {
            return false;
        }

        if (codecContext == nullptr) {
            return false;
        }

        if (formatContext == nullptr) {
            return false;
        }

        const int ret = av_read_frame(formatContext, packet);
        if (ret >= 0) {
            if (packet->stream_index == videoStreamIndex) {
                fVideoFrameAvailable = true;
                avcodec_send_packet(codecContext, packet);
            }
            av_packet_unref(packet);
        } else if (ret == AVERROR_EOF) {
            stop();
        } else {
            char err_buf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(ret, err_buf, AV_ERROR_MAX_STRING_SIZE);
#ifdef UMFELD_CAPTURE_PRINT_ERRORS
            printf("Error occurred: %s\n", err_buf);
#endif
        }
        return fVideoFrameAvailable;
    }

    bool Capture::processFrame() {
        if (codecContext == nullptr) {
            return false;
        }

        if (convertedFrame == nullptr) {
            return false;
        }

        if (fVideoFrameAvailable) {
            const int ret = avcodec_receive_frame(codecContext, frame);
            if (ret == 0) {
                // Successfully received a frame
                fFrameCounter++;

                // Convert data to RGBA or RGB
                sws_scale(swsContext,
                          frame->data,
                          frame->linesize,
                          0,
                          frame->height,
                          convertedFrame->data,
                          convertedFrame->linesize);

                av_frame_unref(frame);

                fVideoFrameAvailable = false;
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
        // // Capture and process frames
        // while (av_read_frame(formatContext, packet) >= 0) {
        //     // Ensure the packet belongs to the video stream
        //     if (packet->stream_index == videoStreamIndex) {
        //         int response = avcodec_send_packet(codecContext, packet);
        //         if (response < 0) {
        //             std::cerr << "Error while sending packet to decoder: " << av_err2str(response) << std::endl;
        //             break;
        //         }
        //
        //         while (response >= 0) {
        //             response = avcodec_receive_frame(codecContext, frame);
        //             if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
        //                 break;
        //             } else if (response < 0) {
        //                 std::cerr << "Error while receiving frame from decoder: " << av_err2str(response) << std::endl;
        //                 break;
        //             }
        //
        //             // Convert the frame to RGB
        //             sws_scale(swsContext, frame->data,
        //                       frame->linesize,
        //                       0,
        //                       codecContext->height,
        //                       rgbFrame->data,
        //                       rgbFrame->linesize);
        //
        //             // Process the RGB frame (you could display it or save it)
        //             std::cout << "Captured frame with width " << codecContext->width << " and height " << codecContext->height << std::endl;
        //         }
        //     }
        //     av_packet_unref(packet);
        // }
    }

    void Capture::start() {
        isPlaying = true;
    }

    void Capture::stop() {
        isPlaying = false;
    }

    void Capture::register_all_devices() {
        if (!fDevicesRegistered) {
            avdevice_register_all(); // Register input device (camera)
            fDevicesRegistered = true;
        }
    }

    std::stringstream Capture::logStream;
    bool              Capture::fDevicesRegistered = false;

    void Capture::custom_log_callback(void* ptr, int level, const char* fmt, va_list vargs) {
        // Append the log messages to the logStream
        char message[1024];
        vsnprintf(message, sizeof(message), fmt, vargs);
        logStream << message;
    }

    void Capture::list_capabilities(const std::string& device_name) {
        if (device_name.empty()) {
            std::cerr << "No device name provided" << std::endl;
            return;
        }

        const std::vector<DeviceCapability> capabilities = getDeviceCapabilities();
        bool                                found        = false;
        for (const auto& capability: capabilities) {
            if (capability.device_name == device_name) {
                if (!found) {
                    found = true;
                    std::cout
                        << "+++ device capabilities "
                        << "'" << capability.device_name << "'"
                        << std::endl;
                }
                std::cout
                    << "    " << capability.width << "x" << capability.height << " (px)";
                if (capability.minimum_frame_rate == capability.maximum_frame_rate) {
                    std::cout
                        << "\t" << capability.minimum_frame_rate << " (FPS)";
                } else {
                    std::cout
                        << "\t" << capability.minimum_frame_rate << "–" << capability.maximum_frame_rate << " (FPS)";
                }
                // std::cout
                //     << "\t" << capability.pixel_format;
                std::cout
                    << std::endl;
            }
        }
    }

    std::vector<std::string> Capture::list() {
        register_all_devices();

        std::vector<std::string> devices;

        // Set the custom log callback
        av_log_set_callback(custom_log_callback);

        // List DirectShow devices on Windows
        AVFormatContext* formatContext = avformat_alloc_context();
        AVDictionary*    options       = nullptr;
        av_dict_set(&options, "list_devices", "true", 0);
        av_dict_set(&options, "probesize", "10000000", 0);      // 1MB probe size
        av_dict_set(&options, "analyzeduration", "3000000", 0); // 5 seconds analysis duration

#if LIBAVFORMAT_VERSION_MAJOR >= 59
        const AVInputFormat* inputFormat = av_find_input_format(get_platform_inputformat());
#else
        AVInputFormat* inputFormat = av_find_input_format(get_platform_inputformat());
#endif

        if (!inputFormat) {
            std::cerr << "Input channels not found" << std::endl;
            return devices;
        }

        // Intercept available devices log
        // avformat_open_input(&formatContext, "video=dummy", inputFormat, &options);
#if LIBAVFORMAT_VERSION_MAJOR >= 60
        int err = avformat_open_input(&formatContext, "video=dummy", inputFormat, &options);
#else
        int err = avformat_open_input(&formatContext, "video=dummy",
                                      const_cast<AVInputFormat*>(inputFormat),
                                      &options);
#endif
        if (err != 0) {
            char err_buf[AV_ERROR_MAX_STRING_SIZE];
            av_strerror(err, err_buf, AV_ERROR_MAX_STRING_SIZE);
            std::cerr << "Failed to open input: " << err_buf << std::endl;
            av_dict_free(&options);
            avformat_free_context(formatContext);
            return devices;
        }

        // Convert logStream to a string and parse it
        const std::string  logOutput = logStream.str();
        std::istringstream logStreamParser(logOutput);
        std::string        line;

        bool inVideoSection = false; // Track when we're in the video devices section

        // Parse the log to find video device names
        while (std::getline(logStreamParser, line)) {
            // Detect the start of the video devices section
            if (line.find("video devices:") != std::string::npos) {
                inVideoSection = true;
                continue; // Skip the header line
            }

            // Detect the end of the video devices section (when audio devices start)
            if (line.find("audio devices:") != std::string::npos) {
                inVideoSection = false;
                break; // Exit once we've finished the video device section
            }

            // Parse device names within the video devices section
            if (inVideoSection) {
                size_t startPos = line.find("] ") + 2;
                if (startPos != std::string::npos) {
                    std::string device = line.substr(startPos);
                    devices.push_back(device); // Add the device to the vector
                }
            }
        }

        av_dict_free(&options);
        avformat_free_context(formatContext);

        // Reset log callback to default
        av_log_set_callback(av_log_default_callback);

        return devices;
    }
#else // ENABLE_CAPTURE && !DISABLE_GRAPHICS && !DISABLE_VIDEO

#ifndef CAPTURE_PRINT_WARNING
#define CAPTURE_PRINT_WARNING 1
#endif

    static bool capture_warning_printed = false;

    static void print_warning() {
#if CAPTURE_PRINT_WARNING
        if (!capture_warning_printed) {
            capture_warning_printed = true;
            std::cout << "+++ WARNING / Capture is disabled. this might be intentional." << std::endl;
            std::cout << "    if not make sure to enable Capture in CMake script: `option(ENABLE_CAPTURE \"Enable camera capture\" ON)`" << std::endl;
            std::cout << "    or enable as compile option                       : `-DENABLE_CAPTURE=ON`" << std::endl;
        }
#endif // CAPTURE_PRINT_WARNING
    }

    Capture::Capture() {
        print_warning();
    }

    bool Capture::init(const char* device_name,
                       const char* resolution,
                       const char* frame_rate,
                       const char* pixel_format) {
        (void) device_name;
        (void) resolution;
        (void) frame_rate;
        (void) pixel_format;
        print_warning();
        return false;
    }

    bool Capture::available() {
        return false;
    }

    bool Capture::read(PGraphics* graphics) {
        (void) graphics;
        return false;
    }

    void Capture::start() {}

    void Capture::stop() {}

    void Capture::reload(PGraphics* graphics) {
        (void) graphics;
    }

    Capture::~Capture() = default;

    void Capture::list_capabilities(const std::string& device_name) {
        (void) device_name;
    }

    std::vector<std::string> Capture::list() {
        print_warning();
        std::vector<std::string> devices;
        return devices;
    }
#endif // ENABLE_CAPTURE && !DISABLE_GRAPHICS && !DISABLE_VIDEO
} // namespace umfeld