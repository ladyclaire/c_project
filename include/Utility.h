/************************************************************
  Copyright (C), 2011, comba. Co. Ltd.
  FileName:      Utility.h
  Author:        zouyuanhua
  Version :      1.0
  Date:          2011.5.2
  Description:   Utility头文件
  Function List:
  History:
      <author>    <time>     <version >   <desc>
     zouyuanhua  2011.5.2    1.0        create
***********************************************************/
#ifndef __UTILITY_H__
#define __UTILITY_H__

#include "CompilerConfig.h"

#include <string>
#include <set>


#include <boost/date_time.hpp>

/* 本地时间结构体 */
typedef struct
{
    char second;
    char minute;
    char hour;
    char date;
    char month;
    unsigned short year;
}S_LocalTime;


/* 空间大小定义 */
typedef struct
{
	// all values are byte counts
	unsigned long long capacity;
	unsigned long long free;
	unsigned long long available;
}S_Space_Info;


#ifdef __cplusplus
extern "C" {
#endif
	/* Process Utility */
	/* 获取进程ID */
	UTILITY_API unsigned int  GetProcessID();
	/* 系统当前时间 */
	UTILITY_API unsigned int  GetCurUTime();
	UTILITY_API unsigned long GetThreadID();

	/* Time Utility */
	UTILITY_API S_LocalTime   GetLocalTimeInfo();

	/* Net Utility */
	UTILITY_API bool ParseURL(const char* url,char* ipAddr , unsigned short &port , char* uri);

	/* String Utility */
	UTILITY_API char* StrTrim(char* s, char c);
	UTILITY_API bool  StrCaseCmp(const char* str1, const char* str2);
	UTILITY_API bool  IsNegativeDigital(char* str);
	UTILITY_API int   ConverToAscii(char* data, int dataLen, char* strBuf, int bufLen);
	UTILITY_API bool  IsDigit(const char *str);
	UTILITY_API void  ToLower(char *str);
	UTILITY_API void  ToUpper(char *str);

	/* File Utility */
	/* 文件或者文件目录是否存在 */
	UTILITY_API bool File_Exists(const char* path);



	/* 创建文件夹 */
	UTILITY_API bool Mkdir(const char* dir);
	/* 创建多级文件目录 : Mkdirs("/mnt/user1/software/test") 目前暂不支持 */
	//UTILITY_API bool Mkdirs(const char* dirs);
	/* 移除文件 */

	UTILITY_API bool File_Remove(const char* path);

	/* 移除文件夹和文件夹下内容 */
	UTILITY_API bool File_Remove_ALL(const char* path); 
	/* 文件夹、文件重命名 如果to中已经存在该文件夹和文件，则删除掉原来文件和文件夹再Rename */
	UTILITY_API bool File_Rename(const char* from , const char* to); 


	/* 文件copy，注意如果to里已经存在该文件则删除掉原来文件再Copy,to不能是路径，必须有文件名 */
	UTILITY_API bool File_Copy(const char* from, const char* to);
	/* 直接用File_copy可能会导致CPU占用率过高 可以调用 File_Copy2 */
	UTILITY_API bool File_Copy2(const char* from, const char* to);
	/* 是否是文件 */
	UTILITY_API bool IsRegularFile(const char* file);
	/* 返回大小以字节为单位，如果文件有问题返回-1,文件正常返回文件大小 */
	UTILITY_API int  File_GetSize(const char* file);
	/* 获取空间信息 */
	UTILITY_API S_Space_Info GetSpace(const char* path);

	/* 是否文件夹 */
	UTILITY_API bool IsDirectory(const char* path);

#ifdef __cplusplus
			}
#endif

	/* 取进程全路径 */
	UTILITY_API std::string  GetProcessPath();
	/* 根据进程全路径，取进程名 */
	UTILITY_API std::string  GetProcessName(const std::string& processPath);
	/* 系统当前时间 */
	UTILITY_API std::string  GetCurTime();

	/* 是否是有效的IP地址 */
	UTILITY_API bool IsValidIP(const char *ip);
	bool GetIPFromString(const std::string &searchStr, std::string &ip);
	/* 从URL（字符串）中取出IP */
	UTILITY_API std::string GetIPFromURL(const char *url);

    UTILITY_API void ScanDirectory(const std::string dir,  /* 需要扫描的目录 */
                                    std::set<std::string> &files,  /* 保存所有非目录文件的容器, 结果的文件没有路径名 */
                                    const std::string regex = "", /* 结果需要匹配的正则表达式,为空时匹配所有 */
                                    bool recursive = true); /* 是否需要递归扫描 */

    /* 此接口有问题，网管手动上传日志时扫描出来的flash日志不全，原因不详 */
	UTILITY_API bool	SearchFile(const std::string &szPath,const std::string &szFileNameTemplate,std::set<std::string> &files, bool bSearchSubDir = true);

	UTILITY_API int		TarFile(const std::string &strTarDestFile,const std::set<std::string> &srcTarFiles);

	UTILITY_API std::string FormatTime(const time_t timestamp);

	UTILITY_API std::string FormatTime(const boost::posix_time::ptime& timestamp);

    /*
    ** 查询dir目录空间是否足够
    ** dir为目录绝对路径，requireBytes为所需要的空间，rate为百分比
    ** 当requireBytes + (dir已用空间大小 - dummy) <= (dir的总空间 * rate)时，返回true, 否则返回false
    */
    UTILITY_API bool CheckSpace(const std::string &dir, 
        const unsigned long long requireBytes, 
        const unsigned long long dummy = 0, /* 已经占用了的空间，但是在判断的时候把它忽略掉来计算 */
        const float rate = 1.0);

    /* 返回dir目录下所有文件的总大小 */
    UTILITY_API unsigned long long TotalSize(const std::string &dir);


	/* linux system api utility */
	/* 实际调用linux system函数，调用前会做路径判断和权限修改 */
	UTILITY_API bool SystemEx(const std::string &file);
	/* 子进程加载，会关闭父进程打开的文件描述符 */
	UTILITY_API bool LoadChildProcess(const std::string &file); 

	UTILITY_API bool Exec(std::string &cmd, std::string &resMsg);

	/**  获取父路径，即文件的纯路径  "/user/local/xxx.hpp" --> "/user/local/"  **/
	UTILITY_API std::string GetFileDir(const std::string &file);

	/**  获取文件名 "/user/local/xxx.hpp" --> "xxx.hpp"  **/
	UTILITY_API std::string GetFilename(const std::string &file);

	/**  获取文件的扩展名 "/user/local/xxx.hpp" --> ".hpp"  **/
	UTILITY_API std::string GetExtension(const std::string &file);

	UTILITY_API std::string GetPosixPath(const std::string &path);
	UTILITY_API std::string AppendPath(const std::string &dir, const std::string &path);

#endif

