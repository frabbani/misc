#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#include "bitmap.h"

int main() {
  setbuf(stdout, NULL);
  const char *url = "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/ElephantsDream.mp4";

  printf("hello world!\n");
  printf("FFMPEG ver. %s\n", av_version_info());

  avformat_network_init();
  AVFormatContext *fmtCtx = NULL;
  if (avformat_open_input(&fmtCtx, url, NULL, NULL) != 0) {
    printf("D'OH!\n");
    return 0;
  }
  avformat_find_stream_info(fmtCtx, NULL);

  int vidStream = -1;
  for (int i = 0; i < (int) fmtCtx->nb_streams; i++)
    if (fmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      vidStream = i;
      break;
    }

  AVCodecContext *codecCtx = avcodec_alloc_context3(NULL);

  avcodec_parameters_to_context(codecCtx, fmtCtx->streams[vidStream]->codecpar);

  const AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
  avcodec_open2(codecCtx, codec, NULL);

  int timestamp = 10 * AV_TIME_BASE;
  if (av_seek_frame(fmtCtx, vidStream, timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
    printf("crap!!!\n");
  }

  AVPacket packet;
  AVFrame *frame = av_frame_alloc();
  AVFrame *rgbFrame = av_frame_alloc();
  if(!frame || !rgbFrame){
    printf("EH?\n");
  }


  struct SwsContext *swsCtx = NULL;
  int w = codecCtx->width;
  int h = codecCtx->height;
  int nbytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, w, h, 1);
  uint8 *buffer = (uint8*) av_malloc(nbytes);

  av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, buffer, AV_PIX_FMT_RGB24, w, h, 1);


  printf("%d x %d\n", w, h);
  swsCtx = sws_getContext(w, h, codecCtx->pix_fmt, w, h, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);

  int found = 0;
  while (av_read_frame(fmtCtx, &packet) >= 0) {
    if (packet.stream_index != vidStream)
      continue;
    int resp = avcodec_send_packet(codecCtx, &packet);
    if (resp < 0) {
      printf("uuhhh\n");
      break;
    }
    while (resp >= 0) {
      resp = avcodec_receive_frame(codecCtx, frame);
      if (resp == AVERROR(EAGAIN) || resp == AVERROR_EOF) {
        // No frame available yet or we've reached end of file
        break;
      } else if (resp < 0) {
        printf("Error receiving frame from decoder\n");
        break;
      }

      if (frame->pts * av_q2d(fmtCtx->streams[vidStream]->time_base) >= 3) {
        found++;
        printf("3 seconds!!!\n");
        // Convert the frame to RGB
        
        sws_scale(swsCtx, frame->data, frame->linesize, 0, h, rgbFrame->data, rgbFrame->linesize);
        memcpy(buffer, rgbFrame->data[0], nbytes);
        break;
      }
    }
    av_packet_unref(&packet);
    if(found)
      break;
  }

  pixel_t *pixels = (pixel_t *)buffer;
  writeBMP( pixels, w, h, "frame");


  av_free(buffer);
  sws_freeContext(swsCtx);
  av_frame_free(&frame);
  av_frame_free(&rgbFrame);
  avcodec_free_context(&codecCtx);
  avformat_close_input(&fmtCtx);
  printf("goodbye!\n");
  return 0;
}
