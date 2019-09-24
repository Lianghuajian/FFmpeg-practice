#include <libavutil/log.h>
#include <libavformat/avformat.h>

int main(int argc , char * argv[])
{
   av_log_set_level(AV_LOG_INFO);

   av_register_all();

   AVFormatContext * fmt_cxt = NULL;
   int ret;
   ret = avformat_open_input(&fmt_cxt,"test.mp4",NULL,NULL);
   if (ret < 0)
   {
      av_log(NULL,AV_LOG_ERROR,"Failed to open file,error code = %s",av_err2str(ret)); 
}

   av_dump_format(fmt_cxt,0,"./test.mp4",0);

   avformat_close_input(&fmt_cxt);

   return 0;
}
