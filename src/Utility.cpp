/************************************************************
  Copyright (C), 2011, comba. Co. Ltd.
  FileName:      Utility.cpp
  Author:        zouyuanhua
  Version :      1.0
  Date:          2011.5.2
  Description:   Utility实现
  Function List:
  History:
      <author>    <time>     <version >   <desc>
     zouyuanhua  2011.5.2    1.0        create
***********************************************************/
#include "SystemFramework/Utility/Utility.h"
#include "SystemFramework/Utility/ObjectHandle.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#ifndef WIN32
#include <unistd.h>
#include <dirent.h>
#endif

#include <boost/filesystem.hpp>   
#include <boost/foreach.hpp>
#include <boost/thread/tss.hpp>

#include "SystemFramework/Utility/RegexTemplatePool.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winsock2.h>
#elif defined(__linux__)
#include <time.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#elif defined(__VXWORKS__)
#include <sys/time.h>
#include <vxWorks.h>
#include <usrFsLib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#endif


#ifdef __VXWORKS__
#include "SystemFramework/NetLib/NetLib.h"
#endif

using namespace boost::filesystem;
using namespace std;

static void GetTimeOfDay(struct timeval* val, void* nothing)
{
#ifdef WIN32
		DWORD ticks = GetTickCount();
		val->tv_sec = ticks / 1000;
		val->tv_usec = (ticks % 1000) * 1000;
#elif defined(__VXWORKS__)
		struct timespec spec;
		//pthread_getcpuclockid()
		clock_gettime(CLOCK_REALTIME,&spec);
		val->tv_sec  = spec.tv_sec;
		val->tv_usec = spec.tv_nsec / 1000;
#elif defined(__linux__) || defined(__CYGWIN__)
		gettimeofday(val,NULL);
#endif
}

/*************************************************************
  Function:    GetProcessID()
  Description: 获取进程ID函数
*************************************************************/
unsigned int GetProcessID()
{
#ifndef WIN32
		return getpid();
#else
		return GetCurrentProcessId();
#endif
}

class GetThreadIDClass{};

unsigned long GetThreadID()
{
#if 1
	static boost::thread_specific_ptr<ObjectHandle<GetThreadIDClass> > ThreadCount;  


	if(ThreadCount.get() == NULL)
		ThreadCount.reset(	new ObjectHandle<GetThreadIDClass>());
	return ThreadCount->getObjectID();
#else
		#ifndef WIN32
        return pthread_self();
		#else
        return GetCurrentThreadId();
		#endif
#endif
}
/*************************************************************
  Function:    GetProcessPath()
  Description: 取进程全路径函数
*************************************************************/
std::string GetProcessPath()
{
	char processPath[1024];
	memset(processPath,0,sizeof(processPath));
#ifdef WIN32
	GetModuleFileName(GetModuleHandle(NULL),processPath,sizeof(processPath));
#elif defined __linux__
	if(readlink("/proc/self/exe",processPath,sizeof(processPath)) < 0)
	{
		cout << "GetProcessPath : Get application path error" << endl;
	}
#elif defined __VXWORKS__
	//TODO
#endif
	return processPath;
}

/*************************************************************
  Function:    GetProcessName()
  Description: 根据进程全路径，取进程名函数
 *************************************************************/
std::string GetProcessName(const std::string& processPath)
{
	return "";
}

/*************************************************************
  Function:    GetCurUTime()
  Description: 系统当前时间函数
*************************************************************/
unsigned int  GetCurUTime()
{
	struct timeval current;        
	GetTimeOfDay(&current,NULL); 
	unsigned int timems = (current.tv_sec)*1000;
	timems += (current.tv_usec)/1000;
	return(timems);
}

/*************************************************************
  Function:    GetCurTime()
  Description: 系统当前时间函数
*************************************************************/
std::string   GetCurTime()
{
	/* Method 1: */
	//boost::posix_time::ptime now= boost::posix_time::second_clock::local_time();
	//std::string str = to_simple_string(now);
	//return str;
	/* Method 2: */
	//boost::system_time now = boost::get_system_time();
	//boost::posix_time::time_duration time = now.time_of_day();
	//unsigned short y = now.date().year();
	//unsigned short hour = time.hours() % 24;
	//long min  = time.minutes();
	//long s = time.seconds();
	//long ms = time.total_milliseconds() % 1000;
	//char buf[100];
	//sprintf(buf,"%04d-%02d-%02d %02d:%02d:%02d:%04d",
	//	y ,now.date().month(),now.date().day(),hour , min ,s,ms);
	//std::string str(buf);

	/* Method 3: */
	time_t now = ::time(NULL);
	struct tm* t = localtime(&now);

	char nowBuf[64] = {0};
	sprintf(nowBuf, "%04d-%02d-%02d %02d:%02d:%02d",
		t->tm_year + 1900,
		t->tm_mon + 1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min,
		t->tm_sec);
	return std::string(nowBuf);
}

/*************************************************************
  Function:    GetLocalTimeInfo()
  Description: 系统当前时间信息函数
*************************************************************/
S_LocalTime   GetLocalTimeInfo()
{
	S_LocalTime  localTime;
    time_t nowTime = ::time(NULL);
    struct tm* tmTime = gmtime(&nowTime);

    localTime.second = tmTime->tm_sec;
    localTime.minute = tmTime->tm_min;
    localTime.hour   = tmTime->tm_hour + 8;
    localTime.date   = tmTime->tm_mday;
    localTime.month  = tmTime->tm_mon + 1;
    localTime.year   = tmTime->tm_year + 1900;
    //printf("System time: %u-%u-%u %u:%u:%u \n", localTime.year, localTime.month, localTime.date,
    //                                             localTime.hour, localTime.minute, localTime.second);
    return localTime;
}


std::string FormatTime(const time_t timestamp)
{
	char buf[128];
	strftime(buf, sizeof(buf), "%Y-%m-%dT%XZ", gmtime(&timestamp));
	return buf;

}

std::string FormatTime(const boost::posix_time::ptime& timestamp)
{
	tm tm_ = boost::posix_time::to_tm(timestamp);
	return (FormatTime(mktime(&tm_)));
}


/*************************************************************
  Function:    ParseURL()
  Description: URL解析函数
*************************************************************/
bool ParseURL(const char* url,char* ipAddr , unsigned short &port , char* uri)
{
	bool parse = false;
	char *p = (char*)url;
	char strPort[8] = "80";
	p = (char*)strstr(url, "http://");
	if (p)
	{
		p += strlen("http://");
		int i = 0;
		while (*p!=':' && *p!='/')
		{
			ipAddr[i++] = *p++;
		}
		ipAddr[i+1] = '\0';

		i = 0;
		if (*p ==':') 
		{
			p++;
			while(*p != '/')
			{
				strPort[i++] = *p++;
			}
			strPort[i+1] = '\0';
		}
		strcpy(uri,p);
		parse = true;
		port = (unsigned short)atoi(strPort);
	}
	return parse;
}

/*String Utility*/
char* StrTrim(char* s, char c)
{
	char* begin = s;
	int i = 0;
	while (s[i]==c)
	{
		begin++;
		i++;
	}
	i = strlen(s) - 1;
	while (s[i]==c)
	{
		s[i] = 0;
		i--;

	}
	strcpy(s, begin);
	return s;
}

bool StrCaseCmp(const char* str1, const char* str2)
{
	if ((NULL == str1) || (NULL == str2)) return false;
	if (strlen(str1) != strlen(str2)) return false;

	int len = strlen(str1);
	for(int i=0; i<len; i++)
	{
		if (tolower(str1[i]) != tolower(str2[i])) return false;
	}
	return true;
}

bool  IsNegativeDigital(char* str)
{
	int len = strlen(str);
	for (int i=0; i< len; i++)
	{
		if (str[i] != ' ')
		{
			if (str[i] == '-')
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}
	return false;
}

int ConverToAscii(char* data, int dataLen, char* strBuf, int bufLen)
{
	if (bufLen < dataLen*2 + 1)  return -1;

	char c = 0;
	int j=0;
	for (int i=0; i<dataLen; i++, j=+2)
	{
		c = (data[i]>>4) & 0x0F;
		if (c < 10)
		{
			strBuf[j+1] = 48 + c;
		}
		else
		{
			strBuf[j+1] = 65 + c;
		}

		c = data[i]&0x0F;
		if (c < 10)
		{
			strBuf[j] = 48 + c;
		}
		else
		{
			strBuf[j] = 65 + c;
		}
	}
	strBuf[j] = '\0';
	return j;
}

bool IsDigit(const char *str)
{
	int len = strlen(str);
	for(int i = 0; i < len; i++)
	{
		if(!isdigit(str[i]))
		{	    	
			return false;
		}
	}
	return true;
}

void  ToLower(char *str)
{
	int len = strlen(str);
	for(int i = 0; i < len; i++)
	{
		str[i] = tolower(str[i]);
	}
}

void  ToUpper(char *str)
{
	int len = strlen(str);
	for(int i = 0; i < len; i++)
	{
		str[i] = toupper(str[i]);
	}
}

/* File Utility */
bool File_Exists(const char* path)
{
#if defined(WIN32) || defined(__linux__) || defined(__CYGWIN__)
	return boost::filesystem::exists(path);
#else
		struct stat buf;
		if (stat(path, &buf) == 0) return true;
		else return false;
#endif
}

bool Mkdir(const char* dir)
{
#if defined(WIN32) || defined(__linux__)  || defined(__CYGWIN__)
	try
	{
		return boost::filesystem::create_directory(dir);
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		cout << "Utility Mkdir :" << ex.what() << endl;
	}
#else
		if (mkdir(dir, 755) == 0) return true;
		else return false;
#endif
	return false;
}

bool File_Copy2(const char* from, const char* to)
{

	if(!File_Exists(from) || IsDirectory(to))
	{
		if(!File_Exists(from)) cout<<from<<" is not exist!";
		if(IsDirectory(to)) cout<<to<<" is a directory!It must be a file!";
		return false;
	}

	if(File_Exists(to))
	{
		if(!File_Remove(to))
			return false;
	}


	/* 直接用File_copy可能会导致CPU占用率过高 */
	FILE *pSrcfile, *pSavefile;
	if(!(pSrcfile = fopen(from, "rb")))
	{
		cout<<"Move File Open From File " << from <<" Failed. "<< errno <<" "<< strerror(errno) <<endl;
		return false;
	}
	if(!(pSavefile = fopen(to, "wb")))
	{
		cout<<"Move File Open Save File " << to <<" Failed. "<< errno <<" "<< strerror(errno) <<endl;
		fclose(pSrcfile);
		return false;
	}

	size_t rc = 0;
	size_t nWriteSize = 0;
	unsigned char buf[8192];
	while( 0 != (rc = fread(buf, sizeof(unsigned char), sizeof(buf), pSrcfile)))
	{
		nWriteSize = fwrite(buf, sizeof(unsigned char), rc, pSavefile);
		if(nWriteSize != rc) 
		{ 
			cout << "Write file[" << to <<"] error."<<endl;
			File_Remove(to);
			fclose(pSrcfile);
			fclose(pSavefile);
			return false;
		}
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
	fclose(pSrcfile);
	fclose(pSavefile);

	return true;

}
bool File_Copy(const char* from, const char* to)
{
#if defined(WIN32) || defined(__linux__) || defined(__CYGWIN__)
	if(!File_Exists(from) || IsDirectory(to))
		return false;
	if(File_Exists(to))
	{
		if(!File_Remove(to))
			return false;
	}
	try
	{
		boost::filesystem::copy_file(from,to);
		return true;
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		cout << "Utility File_Copy failed :" << ex.what() << endl;
	}
#else 
	if(!File_Exists(from) || IsDirectory(to))
			return false;
	if(File_Exists(to))
	{
		if(!File_Remove(to))
			return false;
	}
	if (cp(from, to) == 0) return true;
	else return false;

#endif 
	return false;
}

bool IsDirectory(const char* path)
{
#if defined(WIN32) || defined(__linux__) || defined(__CYGWIN__)
	try
	{
		return boost::filesystem::is_directory(path);
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		cout << "Utility IsDirectory failed :" << ex.what() << endl;
	}
#else
		struct stat buf;
		if (stat(path, &buf) == 0)
		{
			if (S_ISDIR(buf.st_mode)) return true;
			else return false;
		}
		else return false;
#endif
	return  false;
}

bool IsValidIP(const char *ip)
{
	static std::string ipRegexTemplate("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])){3}");
	if(!RegexTemplatePool::Instance()->match(ip, ipRegexTemplate)) { return false; }
	return true;
}

std::string GetIPFromURL(const char *url)
{
	const char ipRegexTemplate[] = ("(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])){3}");
	boost::xpressive::cmatch match;
	if(RegexTemplatePool::Instance()->search(url, ipRegexTemplate, match))
	{
		return match[0].str();
	}
    return "";
}

bool GetIPFromString(const std::string &searchStr, std::string &ip)
{
	std::string getIp("");

	getIp = GetIPFromURL(searchStr.c_str());

	return ("" == getIp);
}

bool IsRegularFile(const char* file)
{
#if defined(WIN32) || defined(__linux__) || defined(__CYGWIN__)
	try
	{
		return boost::filesystem::is_regular_file(file);
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		cout << "Utility IsRegularFile failed :" << ex.what() << endl;
	}
#else
		struct stat buf;
		if (stat(file, &buf) == 0)
		{
			if (S_ISREG(buf.st_mode)) return true;
		}
#endif
	return false;
}


bool File_Remove(const char* path)
{
#if defined(WIN32) || defined(__linux__) || defined(__CYGWIN__)
	if(!IsRegularFile(path))
		return false;
	try
	{
		return boost::filesystem::remove(path);
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		cout << "Utility File_Remove failed :" << ex.what() << endl;
	}
#else
	if(!IsRegularFile(path)) return false;
	if (remove(path) == 0) return true;
#endif
	return false;
}

bool File_Remove_ALL(const char* path)
{
	try
	{
		return (1 <= boost::filesystem::remove_all(path) );
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		cout << "Utility File_Remove_ALL failed :" << ex.what() << endl;
		return false;
	}
}


bool File_Rename(const char* from , const char* to)
{
#if defined(WIN32) || defined(__linux__) || defined(__CYGWIN__)
	if(File_Exists(to))
	{
		File_Remove_ALL(to);
	}
	try
	{
		boost::filesystem::rename(from,to);
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		cout << "Utility File_Rename failed :" << ex.what() << endl;
		return false;
	}
#else
	/* vxworks has it's rename method,but...
	 * Only certain devices support rename( ). To confirm that your device supports it, consult the respective xxDrv or xxFs listings to verify that ioctl FIORENAME exists. 
	 * For example, dosFs, HRFS and NFS support rename( ), but netDrv does not.
	 * so use mv for implementation
	 */
	if (!File_Exists(from)) return false;
	if(File_Exists(to))
	{
		File_Remove_ALL(to);
	}
	if (mv(from, to) == 0) return true;
	else return false;
#endif
	return true;
}

int  File_GetSize(const char* file)
{
#if defined(WIN32) || defined(__linux__) || defined(__CYGWIN__)
	try
	{
		return (int)boost::filesystem::file_size(file);
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		cout << "Utility File_GetSize :" << ex.what() << endl;
	}
#else
	if (!IsRegularFile(file)) return -1;
	
	struct stat buf;
	if (stat(file, &buf) == 0) return buf.st_size;	/* File size in bytes */
#endif
	return -1;
}

S_Space_Info GetSpace(const char* path)
{
	S_Space_Info v;
	memset(&v,0,sizeof(v));
#if defined(WIN32) || defined(__linux__) || defined(__CYGWIN__)
	if (IsDirectory(path))
	{
		try
		{
			boost::filesystem::space_info info = boost::filesystem::space(path);
			v.capacity = info.capacity;
			v.free     = info.free;
			v.available= info.available;
		}
		catch (const boost::filesystem::filesystem_error& ex)
		{
			cout << "Utility GetSpace :" << ex.what() << endl;
		}
	}
#else
	if (IsDirectory(path))
	{
		struct statfs pstat;
		if (statfs(path, &pstat) == 0)
		{
			v.free = pstat.f_bfree * pstat.f_bsize;
			v.available = pstat.f_bavail * pstat.f_bsize;
			v.capacity = pstat.f_blocks * pstat.f_bsize;
		}
	}
#endif
	return v;
}

#ifndef WIN32
void ScanDirectory(const std::string dir,
                   std::set<std::string> &files,
                   const std::string regex, 
                   bool recursive)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    if((dp = opendir(dir.c_str())) == NULL) return;

    chdir(dir.c_str());
    while((entry = readdir(dp)) != NULL)
    {
#ifdef __VXWORKS__
    	stat(entry->d_name, &statbuf);
#else
    	lstat(entry->d_name, &statbuf);
#endif 
        if(S_ISDIR(statbuf.st_mode)) 
        {
            if(strcmp(".", entry->d_name) == 0 
                || strcmp("..", entry->d_name) == 0)
                continue;
            
            if(recursive) ScanDirectory(entry->d_name, files);
        }
        else
        {
            std::string file_name(entry->d_name);
            if(regex.empty())
            {
                files.insert(file_name);
            }
            else
            {
                boost::xpressive::cmatch match;
                if(RegexTemplatePool::Instance()->search(file_name, regex, match))
                {
                    files.insert(file_name);
                }
            }
        }
    }
    chdir("..");
    closedir(dp);
}
#endif

bool SearchFile(const std::string &szPath,
				const std::string &szFileNameTemplate,
				std::set<std::string> &files,
				bool bSearchSubDir
				)
{
	path filePath(szPath);

	if(is_directory(filePath))
	{
		for (directory_iterator it(filePath); it != directory_iterator(); ++it)
		{

			if(is_regular_file(*it))
			{
				std::string filename = it->path().string();
				if(files.count(filename))
					return false;
				else
				{
					boost::xpressive::cmatch match;;

					if(!szFileNameTemplate.empty())
					{
						if(RegexTemplatePool::Instance()->search(filename,szFileNameTemplate,match))
						{
							//OAMLogger(LOG_DEBUG,"file [%s] Match filename template[%s]",filename.c_str(),szFileNameTemplate.c_str());
							#if defined( WIN32) || defined( __i386) || defined(i386)|| defined(__i386__)
							for (unsigned i = 0;i< match.size();i++)
								if(match[i].matched)
									cerr<<"matched "<<i<<" "<<match[i].str()<<endl;
							#endif
							files.insert(filename);
						}
						else
						{
#if 0 //defined( WIN32) || defined( __i386) || defined(i386)|| defined(__i386__)
							OAMLogger(LOG_DEBUG,"file [%s] No Match template [%s]",filename.c_str(),szFileNameTemplate.c_str());
#endif
						}
					}
					else
					{
						files.insert(filename);
					}
				}
			}
			else if(bSearchSubDir && is_directory(*it))
			{
				SearchFile(it->path().string(),szFileNameTemplate,files,bSearchSubDir);
			}
		}
	}

	return true;

}



int TarFile(const std::string &strTarDestFile,
			 const std::set<std::string> &srcTarFiles)
{
	string strTarSrcFile;

	string strCmdBuf("tar zcvf ");
	strCmdBuf += strTarDestFile + " ";

	BOOST_FOREACH(std::string filename,srcTarFiles)
	{
		strTarSrcFile += filename + ' ';
	}


	if(strTarSrcFile.empty())
	{
		cout<<"====No File To Tar !====="<<endl;
		return 0;
	}
	strCmdBuf += strTarSrcFile;
	cout<<"====ExecuteTarCmd["<<strCmdBuf<<"]!====="<<endl;
#if defined(__VXWORKS__) 
	::system(strCmdBuf.c_str());
	ThreadSleep(30000);
	return  File_GetSize(strTarDestFile.c_str());
#elif defined(__linux__) || defined(__CYGWIN__)
	FILE *fp = popen(strCmdBuf.c_str(),"r");

	char buf[256];
	while(fgets(buf,sizeof(buf),fp))
	{
		#if  defined( __i386) || defined(i386)|| defined(__i386__)
		cout<<"====Excute["<<buf<<"]======"<<endl;
		#endif
	}
	pclose(fp);

	//ThreadSleep(5000);
	path pathUploadFilePathName(strTarDestFile);

	if(is_regular_file(pathUploadFilePathName))
	{
		return File_GetSize(strTarDestFile.c_str());
	}
	else
		return 0;
#else
	return true;
#endif

}

bool CheckSpace(const std::string &dir, const unsigned long long requireBytes, const unsigned long long dummy, const float rate)
{
    /* 仅针对目录 */
    if(!IsDirectory(dir.c_str())) return false;

    /* rate必须大于等于0或小于等于1 */
    if(0 > rate || 1 < rate) return false;

    S_Space_Info spaceInfo = GetSpace(dir.c_str());

#ifdef __CYGWIN__
    return spaceInfo.available - requireBytes > 100*1024*1024;
#endif

    /* 已用空间 - subtract + requireBytes */
    return ((spaceInfo.capacity - spaceInfo.available) - dummy + requireBytes)
        <= (spaceInfo.capacity * rate);

    //return spaceInfo.available - requireBytes  >= (spaceInfo.capacity * (1 - rate));
}

unsigned long long TotalSize(const std::string &dir)
{
    unsigned long long used = 0;
	path fullPath = dir;

    try
    {
        directory_iterator end_iter;
        for(directory_iterator file_ite(fullPath); file_ite != end_iter; file_ite++)
        {
            std::string name = file_ite->path().filename().string();
            std::string fullPath = dir + "/" + name;

            if(is_directory(*file_ite))
            {
                used += TotalSize(fullPath);
            }
            else
            {
                used += File_GetSize(fullPath.c_str());
            }
        }
    }
	catch(const filesystem_error &e)
	{
        std::cout << e.path1().string() << ": " << e.what() << std::endl;
        return 0;
	}

	return used;
}

bool SystemEx(const std::string &file)
{
#if defined (__linux__) || defined(__CYGWIN__)
	if ( ! File_Exists(file.c_str()) )
	{
		printf("SystemEx:could not run %s : file is not exist.\n",file.c_str());
		return false;
	}
	string cmd ("chmod 777 ");
	cmd += file;
	system(cmd.c_str());
	cmd = file + string(" > /dev/null");
	system(cmd.c_str());
	return true;
#endif
	return false;
}

bool LoadChildProcess(const std::string &file)
{
#if defined (__linux__)
	if(chmod(file.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) < 0)
	{
		printf("LoadChildProcess:chmod(%s, S_IRWXU | S_IRWXG | S_IRWXO) failed: %s.\n", file.c_str(), strerror(errno));
		return false;
	}

	/* 获取最大文件描述符数 */
	struct rlimit rl;
	if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
	{
		printf("LoadChildProcess:getrlimit() failed: %s.\n", strerror(errno));
		return false;
	}

	int pid = fork();
	if(pid > 0)
	{
		int status;
		while(waitpid(pid, &status, 0) < 0)
		{
			if(errno != EINTR)
			{
				break;
			}
		}

		return true;
	}
	else if(0 == pid)
	{
		if(rl.rlim_max == RLIM_INFINITY)
		{
			rl.rlim_max = 1024;
		}
		/* 关闭父进程打开的文件描述符 */
		for(int i = 0; i < rl.rlim_max; ++i)
		{
			close(i);
		}

		/* 切换当前目录到根目录 */
		chdir("/");

		/* 重定向0,1,2文件描述符到/dev/null */
		open("/dev/null", O_RDWR);
		dup(0);
		dup(0);

		pid = fork();
		if(pid < 0)
		{
			_exit(-1);
		}
		else if(pid > 0)
		{
			_exit(0);
		}

		execl("/bin/sh", "sh", "-c", file.c_str(), (char *)0);
		_exit(0);
	}
	else
	{
		printf("fork() failed: %s.\n", strerror(errno));
		return false;
	}
	return true;
#endif
	return false;
}

/*******************************************************************************
  Function:     Exec
  Description:  执行shell命令并且获取最终的执行结果（命令内容）
  Input:        cmd ---> shell命令
                resMsg ---> 指示信息
  Output:       none
  Return:       命令执行结果
  Others:
**********************************************************************************/
bool Exec(std::string &cmd, std::string &resMsg)
{
	resMsg.clear();

	/* 加此符号可以使popen时返回信息连续完整、若不加会读取不完整信息 */
	cmd += " 2>&1";
	printf("Exec cmd[%s]", cmd.c_str());

	FILE *fd = NULL;
	fd = popen(cmd.c_str(), "r");

	if(fd == NULL)
	{
		resMsg = "popen error";
		return false;
	}

	char buf[512];
	memset(buf, 0, sizeof(buf));
	stringstream ss;

	while(fgets(buf, sizeof(buf), fd) != NULL)
	{
//		if (ss.tellp() < 4096)
		{
			ss << buf;
		}
		memset(buf, 0, sizeof(buf));
	}

	int status = pclose(fd);
	/* status为管道进程的终止状态 */
	if (status != -1)
	{
		/* WIFEXITED(status)指示子进程是否正常退出 */
		if (WIFEXITED(status))
		{
			/* WEXITSTATUS(status)为cmd执行的返回结果 */
			if (WEXITSTATUS(status) == 0)
			{
				resMsg = ss.str();
				printf("cmd[%s] operation success", cmd.c_str());
				return true;
			}
			else
			{
				printf("return errno[%d], cmd[%s]", WEXITSTATUS(status), cmd.c_str());
			}
		}
		else
		{
			printf("popen errno[%d], cmd[%s]", WIFEXITED(status), cmd.c_str());
		}
	}

	resMsg = ss.str();
	return false;
}


/*******************************************************************************
  Function:     GetFileDir
  Description:  获取父路径，即文件的纯路径  "/user/local/xxx.hpp" --> "/user/local/"
  Input:        none
  Output:       none
  Return:       none
  Others:       none
**********************************************************************************/
std::string GetFileDir(const std::string &file)
{
	try
	{
		return boost::filesystem::path(file).parent_path().string();
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		std::cout << "GetFileDir error: file[" << file << "], " << ex.what() << std::endl;
	}
	return "";
}

/*******************************************************************************
  Function:     GetFilename
  Description:  获取文件名 "/user/local/xxx.hpp" --> "xxx.hpp"
  Input:        none
  Output:       none
  Return:       none
  Others:       none
**********************************************************************************/
std::string GetFilename(const std::string &file)
{
	try
	{
		return boost::filesystem::path(file).filename().string();
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		std::cout << "GetFilename error: file[" << file << "], " << ex.what() << std::endl;
	}
	return "";
}

/*******************************************************************************
  Function:     GetExtension
  Description:  获取文件的扩展名 "/user/local/xxx.hpp" --> ".hpp"
  Input:        none
  Output:       none
  Return:       none
  Others:       none
**********************************************************************************/
std::string GetExtension(const std::string &file)
{
	try
	{
		return boost::filesystem::path(file).extension().string();
	}
	catch (const boost::filesystem::filesystem_error& ex)
	{
		std::cout << "GetExtension error: file[" << file << "], " << ex.what() << std::endl;
	}
	return "";
}

std::string GetPosixPath(const std::string &path)
{
	std::string posixPath(path);
	for(unsigned int i = 0; i < posixPath.size(); i++)
	{
		if (posixPath[i] == '\\') posixPath[i] = '/';
	}
	return posixPath;
}

/*******************************************************************************
  Function:     AppendPath
  Description:  dir后面追加path
  Input:        none
  Output:       none
  Return:       none
  Others:       dir最后有一字符可能带有'/'，也可能不带'/'，path可以是路径也可以文件名，也可以是带路径的文件名
                dir是使用标准的POSIX语法提供可移植的路径表示，而path则不限制
**********************************************************************************/
std::string AppendPath(const std::string &dir, const std::string &path)
{
	std::string filename(GetPosixPath(path));
	if (!filename.empty() && '/' == filename[0])
	{
		filename = 1 == filename.size() ? "" : filename.substr(1);
	}
	std::string posixDir(GetPosixPath(dir));
	if (posixDir.empty() || '/' != posixDir[posixDir.size()-1])
	{
		return posixDir + "/" + filename;
	}
	return posixDir + filename;
}
/* end of file */
