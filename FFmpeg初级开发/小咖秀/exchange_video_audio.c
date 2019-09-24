#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt, const char *tag)
{
    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;
    
    printf("%s: pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           tag,
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}
int exchange_video_and_audio(char* video1_filename,char *video2_filename,char *output_filename)
{
    AVOutputFormat *ofmt = NULL;
    //输入上下文
    AVFormatContext *ifmt_ctx_video1 = NULL;
    AVFormatContext *ifmt_ctx_video2 = NULL;
    //输出上下文
    AVFormatContext *ofmt_ctx = NULL;
    //输入流
    AVStream *in_audio_stream = NULL;
    AVStream *in_video_stream = NULL;
    //输出流
    AVStream *out_audio_stream = NULL;
    AVStream *out_video_stream = NULL;
    AVPacket pkt;
    
    //最长时长
    double maxDuration ;
    
    char *in_video1_filename = NULL,*in_video2_filename = NULL,*out_filename = NULL;
    
    int ret, i,errorCode;
    int stream_index = 0;
    int *stream_mapping = NULL;
    int stream_mapping_size = 0;
    
    in_video1_filename = video1_filename;
    
    in_video2_filename = video2_filename;
    
    out_filename = output_filename;
    
    av_register_all();
    
    if ((ret = avformat_open_input(&ifmt_ctx_video1, in_video1_filename, 0, 0)) < 0)
    {
        fprintf(stderr, "Could not open input file '%s'", in_video2_filename);
        goto end;
    }
    
    if ((ret = avformat_open_input(&ifmt_ctx_video2, in_video2_filename, 0, 0)) < 0)
    {
        fprintf(stderr, "Could not open input file '%s'", in_video2_filename);
        goto end;
    }
    
    if (avformat_find_stream_info(ifmt_ctx_video1, 0) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        goto end;
    }
    
    if (avformat_find_stream_info(ifmt_ctx_video2, 0) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        goto end;
    }
    
    av_dump_format(ifmt_ctx_video1, 0, in_video1_filename, 0);
    
    av_dump_format(ifmt_ctx_video2, 0, in_video2_filename, 0);
    //给输出上下文分配空间
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    
    //看输入文件是否有音频流
    int in_audio_stream_index = av_find_best_stream(ifmt_ctx_video1,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
    
    if (in_audio_stream_index < 0)
    {
        av_log(NULL,AV_LOG_ERROR,"Fail to find audio stream");
        goto end;
    }
    //通过下标拿到音频流
    in_audio_stream = ifmt_ctx_video1->streams[in_audio_stream_index];
    
    out_audio_stream = avformat_new_stream(ofmt_ctx,NULL);
    
    //拷贝音频流参数
    if ((errorCode = avcodec_parameters_copy(out_audio_stream->codecpar,in_audio_stream->codecpar)) < 0) {
        av_log(NULL,AV_LOG_ERROR,"Fail to copy audio_stream parameters");
        goto end;
    }
    
    out_audio_stream->codecpar->codec_tag = 0;
    
    //看输入文件是否有视频流
    int in_video_stream_index = av_find_best_stream(ifmt_ctx_video1,AVMEDIA_TYPE_VIDEO,-1,-1,NULL,0);
    
    if (in_video_stream_index < 0)
    {
        av_log(NULL,AV_LOG_ERROR,"Fail to find video stream");
        goto end;
    }
    //通过下标拿到视频流
    in_video_stream = ifmt_ctx_video2->streams[in_video_stream_index];
    
    out_video_stream = avformat_new_stream(ofmt_ctx,NULL);
    
    //拷贝视频流参数
    if ((errorCode = avcodec_parameters_copy(out_video_stream->codecpar,in_video_stream->codecpar)) < 0)
    {
        av_log(NULL,AV_LOG_ERROR,"Fail to copy video_stream parameters");
        goto end;
    }
    
    out_video_stream->codecpar->codec_tag = 0;
    
    //获取音频流和视频流中短的时长
    if (in_audio_stream->duration * av_q2d(in_audio_stream->time_base) < in_video_stream->duration * av_q2d(in_video_stream->time_base))
    {
        maxDuration = in_audio_stream->duration * av_q2d(in_audio_stream->time_base);
    }else
    {
        maxDuration = in_video_stream->duration * av_q2d(in_video_stream->time_base);
    }
    
    
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    
    ofmt = ofmt_ctx->oformat;
    
    av_dump_format(ofmt_ctx, 0,out_filename, 1);
    
    //打开输出文件
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open output file '%s'", out_filename);
            goto end;
        }
    }
    
    ret = avformat_write_header(ofmt_ctx, NULL);
    
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        goto end;
    }
    
    //写音频包
    av_init_packet(&pkt);
    
    while (av_read_frame(ifmt_ctx_video1,&pkt) >= 0)
    {
        //对于大于最大时长的帧跳过，比如时间基本来是1:25，这里的av_q2d把其变为1/25秒，这个25是一秒的帧率
        if (pkt.pts * av_q2d(in_audio_stream->time_base) > maxDuration)
        {
            av_packet_unref(&pkt);
            continue;
        }
        //拿到我们的在video1的音频包
        if (pkt.stream_index == in_audio_stream_index)
        {
            pkt.pts = av_rescale_q_rnd(pkt.pts
                                       ,in_audio_stream->time_base,out_audio_stream->time_base
                                       ,(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(pkt.dts,in_audio_stream->time_base,out_audio_stream->time_base,(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.duration = av_rescale_q(pkt.duration,in_audio_stream->time_base,out_audio_stream->time_base);
            pkt.pos = -1;
            //0代表音频流
            pkt.stream_index = 0;
            //写入音频包到输出上下文
            av_interleaved_write_frame(ofmt_ctx,&pkt);
            av_packet_unref(&pkt);
        }
    }
    
    //写视频包
    while(av_read_frame(ifmt_ctx_video2,&pkt) >= 0)
    {
        if (pkt.pts * av_q2d(in_video_stream->time_base) > maxDuration)
        {
            av_packet_unref(&pkt);
            continue;
        }
        
        if (pkt.stream_index == in_video_stream_index)
        {
            //转换渲染时间轴
            pkt.pts = av_rescale_q_rnd(pkt.pts,in_video_stream->time_base,out_video_stream->time_base,(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            //转换解码时间轴
            pkt.dts = av_rescale_q_rnd(pkt.dts,in_video_stream->time_base,out_video_stream->time_base,(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.duration = av_rescale_q(pkt.duration,in_video_stream->time_base,out_video_stream->time_base);
            
            pkt.pos = -1;
            //1是视频流
            pkt.stream_index = 1;
            //写入视频包
            av_interleaved_write_frame(ofmt_ctx,&pkt);
            av_packet_unref(&pkt);
        }
    }
end:
    av_write_trailer(ofmt_ctx);
    /* close input */
    if (ifmt_ctx_video1)
    {
        avformat_close_input(&ifmt_ctx_video1);
    }
    if (ifmt_ctx_video2)
    {
        avformat_close_input(&ifmt_ctx_video2);
    }
    ret = 0;
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    
    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }
    
    return 0;
}
int main(int argc, char **argv)
{
    
    av_log_set_level(AV_LOG_DEBUG);
    
    if (argc < 4)
    {
        printf("usage: %s input output\n"
               "API example program to remux a media file with libavformat and libavcodec.\n"
               "The output format is guessed according to the file extension.\n"
               "\n", argv[0]);
        return 1;
    }
    
    exchange_video_and_audio(argv[1],argv[2],argv[3]);
    
}
