#include <libavformat/avformat.h>
#include <libavutil/log.h>
int main(int argc,char* argv[])
{

av_log_set_level(AV_LOG_INFO);
//上下文，用来装载当前目录的基本信息
AVIODirContext *ctx = NULL;
//存放目录项，存放文件名和文件大小等信息
AVIODirEntry *entry = NULL;
//打开目录
int ret = avio_open_dir(&ctx,"./",NULL);
if (ret < 0)
{
av_log(NULL,AV_LOG_ERROR,"Failed to open dir , error code = %s",av_err2str(ret));
goto __fail;
}else
{
av_log(NULL,AV_LOG_INFO,"Open dir successfull");
}
//开始查看目录
while(1)
{
ret = avio_read_dir(ctx,&entry);
if (ret < 0){
av_log(NULL,AV_LOG_ERROR,"Failed to read dir , error code = %s",av_err2str(ret));
goto __fail;
}else
{
if (!entry)
{
break;
}
av_log(NULL,AV_LOG_INFO,"%12"PRId64" %s \n",entry->size,entry->name);
avio_free_directory_entry(&entry);
}

}
__fail:
avio_close_dir(&ctx);
return 0;
}
