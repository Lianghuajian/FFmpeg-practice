#include <libavformat/avformat.h>
int main(int argc,char* argv[])
{
int ret;
if (avpriv_io_delete("abc.txt") < 0)
{
av_log(NULL,AV_LOG_ERROR,"Failed to delete %s \n","abc.txt");
}else
{
av_log(NULL,AV_LOG_INFO,"Succeed to delete %s \n","abc.txt");
}

if (avpriv_io_move("cba.txt","def.txt") < 0)
{
av_log(NULL,AV_LOG_ERROR,"Failed to rename %s to %s \n","cba.txt","def.txt");
}else 
{
av_log(NULL,AV_LOG_INFO,"Succeed to rename %s to %s \n","cba.txt","def.txt");
}

return 0;
}
