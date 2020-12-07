#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "pcre.h"

#include "rlib_types.h"
#include "define.h"
#include <sys/types.h>
#include <regex.h>


#define PCRE_STATIC // 静态库编译选项


#define OVECCOUNT 30 /* should be a multiple of 3 */
#define EBUFLEN 128
#define BUFLEN 1024

typedef struct fileMd5
{
    char current[40];
    char previous[40];
    bool changed;
} S_fileMd5;

enum
{
    start=0,
    mid,
    last,
    end

};


/**
*@brief 用pcre库来验证str是否符合pattern格式的正则表达式
*
*
*@param const char *str  要被用来匹配的字符串
*@param const char *pattern  将要被编译的字符串形式的正则表达式
*
*@return
*
*
*@author mobin
*@date 2020年9月15日
*@note 新生成函数
*
*/
int check_pcre_regex(const char *str, const char *pattern)
{

    pcre  *re;
    const char *error;
    int  erroffset;
    int  ovector[OVECCOUNT];
    int  rc, i;

    printf("\n check_pcre_regex string: %s \n", str);
    printf("\n check_pcre_regex Pattern: %s \n", pattern);
    re = pcre_compile(pattern,       // pattern, 输入参数，将要被编译的字符串形式的正则表达式
                      0,            // options, 输入参数，用来指定编译时的一些选项
                      &error,       // errptr, 输出参数，用来输出错误信息
                      &erroffset,   // erroffset, 输出参数，pattern中出错位置的偏移量
                      NULL);        // tableptr, 输入参数，用来指定字符表，一般情况用NULL
// 返回值：被编译好的正则表达式的pcre内部表示结构
    if (re == NULL)                   //如果编译失败，返回错误信息
    {
        printf("\n PCRE compilation failed at offset %d: %s \n", erroffset, error);
        return 0;
    }
    rc = pcre_exec(re,            // code, 输入参数，用pcre_compile编译好的正则表达结构的指针
                   NULL,          // extra, 输入参数，用来向pcre_exec传一些额外的数据信息的结构的指针
                   str,           // subject, 输入参数，要被用来匹配的字符串
                   strlen(str),  // length, 输入参数， 要被用来匹配的字符串的指针
                   0,             // startoffset, 输入参数，用来指定subject从什么位置开始被匹配的偏移量
                   0,             // options, 输入参数， 用来指定匹配过程中的一些选项
                   ovector,       // ovector, 输出参数，用来返回匹配位置偏移量的数组
                   OVECCOUNT);    // ovecsize, 输入参数， 用来返回匹配位置偏移量的数组的最大大小
// 返回值：匹配成功返回非负数，没有匹配返回负数
    if (rc < 0)                       //如果没有匹配，返回错误信息
    {
        if (rc == PCRE_ERROR_NOMATCH)
            printf("\n Sorry, no match ...\n");
        else
            printf("\n Matching error %d \n", rc);
        pcre_free(re);
        return 0;
    }
    printf("\n OK, has matched ...\n");   //没有出错，已经匹配
    for (i = 0; i < rc; i++)               //分别取出捕获分组 $0整个正则公式 $1第一个()
    {
        char *substring_start = str + ovector[2*i];
        int substring_length = ovector[2*i+1] - ovector[2*i];
        printf("\n %2d: %d %s \n", i, substring_length, substring_start);
    }
    pcre_free(re);                     // 编译正则表达式re 释放内存
    return 1;
}


#define PROTROPORT_PATTEN "^((tcp|udp)(/([0-9]|[1-9][0-9]{1,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])))(;((tcp|udp)(/([0-9]|[1-9][0-9]{1,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])))){0,9}$"

#define OLD_PASSWORD_PATTEN "^[a-zA-Z0-9@\\#\\.\\$\\*\\!][a-zA-Z0-9@\\#\\.\\$\\*\\!\\-]{4,32}$"

#define OLD_PASSWORD_PATTEN1 "	"

// 基本字符转义之后，但是这个格式Linux POSIX不支持，需要写出BRGs 或者ERGs 的格式
#define NEW_PASSWORD_PATTEN_PCRE "^(?![A-Z]+$)(?![a-z]+$)(?!\\d+$)(?![\\W@#$.*!-]+$)[a-zA-Z0-9\\W@#$.*!-]{12,32}$"

#define NEW_PASSWORD_PATTEN_PCRE_S "^(?![a-z]+$)(?![0-9]+$)(?![A-Z]+$)(?!([@#$.*!-])+$)[a-zA-Z0-9@#$.*!-]{12,32}$"


#define NEW_PASSWORD_PATTEN "^(([a-zA-Z]+[0-9]+)|([0-9]+[a-zA-Z]+)|([a-zA-Z]+[@#%]+)|([0-9]+[@#%]+))([a-zA-Z0-9@#%]*){5,12}$"

#define safe_free(p)            do { if (p != NULL) { free(p); p = NULL; } } while(0)

bool string_reg_verify(const char *str, const char *pattern)
{
    bool result = true;
    regex_t reg;
    bool free_needed = false;
    char *ebuf[128];

    if (NULL == str)
    {
        result = false;
        goto end;
    }
    int code = regcomp(&reg, pattern, REG_EXTENDED | REG_NOSUB);
    if (0 !=code)
    {
        result = false;
        printf("\n regcomp false\n");
        regerror(code, &reg, ebuf, sizeof(ebuf));
        fprintf(stderr, "%s: pattern '%s' \n",ebuf, pattern);

        goto end;
    }
    free_needed = true;

    if (regexec(&reg, str, 0, NULL, 0))
    {
        result = false;
        printf("\n regexec false\n");
    }

end:
    if (free_needed)
    {
        regfree(&reg);
    }

    return result;
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
bool Exec(char *cmd, char *resMsg,int resSize)
{
    memset(resMsg, 0, resSize);
    char cmdBuf[126]= {""};
    /* 加此符号可以使popen时返回信息连续完整、若不加会读取不完整信息 */
    //cmd += " 2>&1";
    snprintf(cmdBuf,"%s 2>&1",cmd);
    printf("\nExec cmd[%s]\n", cmd);

    FILE *fd = NULL;
    fd = popen(cmdBuf, "r");

    if(fd == NULL)
    {
        resMsg = "popen error";
        return false;
    }

    char buf[512];
    memset(buf, 0, sizeof(buf));

    int readlen = 0;
    while(fgets(buf, sizeof(buf), fd) != NULL)
    {

        memcpy(resMsg+readlen,buf,strlen(buf));
        readlen += strlen(buf);

        memset(buf, 0, sizeof(buf));
    }
    memset(resMsg+readlen,0,1);
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

                printf("cmd[%s] operation success\nresault:\n[%s]\n", cmdBuf,resMsg);
                return true;
            }
            else
            {
                printf("return errno[%d], cmd[%s]", WEXITSTATUS(status), cmdBuf);
            }
        }
        else
        {
            printf("popen errno[%d], cmd[%s]", WIFEXITED(status), cmdBuf);
        }
    }


    return false;
}


char *string_skip_whitespace(char *string)
{
    if (!string)
    {
        return NULL;
    }
    while ((*string && isspace(*string)))
    {
        string++;
    }

    return string;
}

char* replaceMultiSpaceToOne(char *splited,const char *src)
{
    if(NULL != src && NULL != splited)
    {
        int j=0;


        while(*src)
        {
            if(*src !=' ' || *(src+1) != ' ') 
            {
                splited[j++] = *src;
            }
            src++;
        }
        splited[j]='\0';
        return splited;
    }
    else
    {
        return NULL;
    }
}



char * string_replace(char const * const original, char const * const pattern, char const * const replacement)
{
    uint32_t replen = strlen(replacement);
    uint32_t patlen = strlen(pattern);
    uint32_t orilen = strlen(original);

    uint16_t patcnt = 0;
    const char * oriptr;
    const char * patloc;

    // find how many times the pattern occurs in the original string
    for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
    {
        patcnt++;
    }

    {
        // allocate memory for the new string
        uint32_t retlen = orilen + patcnt * (replen - patlen);
        char * const returned = (char *) malloc( sizeof(char) * (retlen + 1) );

        if (returned != NULL)
        {
            // copy the original string,
            // replacing all the instances of the pattern
            char * retptr = returned;
            for (oriptr = original; patloc = strstr(oriptr, pattern); oriptr = patloc + patlen)
            {
                uint32_t const skplen = patloc - oriptr;
                // copy the section until the occurence of the pattern
                strncpy(retptr, oriptr, skplen);
                retptr += skplen;
                // copy the replacement
                strncpy(retptr, replacement, replen);
                retptr += replen;
            }
            // copy the rest of the string.
            strcpy(retptr, oriptr);
        }
        return returned;
    }
}

int string_split(char *string, const char * delimiters, char *words[], int max)
{
    char *p;
    int cnt = 0;

    if (!string)
    {
        return 0;
    }

    p = strtok(string, delimiters);
    while (p)
    {
        words[cnt] = p;
        p = strtok(NULL, delimiters);
        cnt++;
        if (cnt >= max)
        {
            break;
        }
    }

    return cnt;
}

bool string_matched(const char *src, const char *dst)
{
    if (!src || !dst)
    {
        return false;
    }
    while (*src && *dst && (*src == *dst))
    {
        src ++;
        dst ++;
    }

    return *src == *dst;
}

char *chage_to_get_user_password_expire_date(char *username)
{
    //chage -l aaaaa | awk 'NR==2{print $4,$5,$6}'
}
//extract chage cmd expire date and convert to digital format
//eg. Jul 20, 2020 to 2020-07-20
char *convert_english_date_to_digital(char *converted,int size,const char *src)
{
    //Jul 20, 2020
    //replace ',' to ''

    char *tmp=NULL;
    tmp = malloc(size);
    tmp = string_replace(src,",","");
    char *word[3];

    int cnt=0;
    int month =0;
    cnt = string_split(tmp, " ", word, ARRAY_SIZE(word));

    if(3 == cnt)
    {
        for(int i=0; i<3; i++)
        {
            printf("word[%d]:%s\n",i,word[i]);
        }
        if(string_matched("Jan",word[0]))
        {
            month = 1;
        }
        else if(string_matched("Feb",word[0]))
        {
            month = 2;
        }
        else if(string_matched("Mar",word[0]))
        {
            month = 3;
        }
        else if(string_matched("Apr",word[0]))
        {
            month = 4;
        }
        else if(string_matched("May",word[0]))
        {
            month = 5;
        }
        else if(string_matched("Jun",word[0]))
        {
            month = 6;
        }
        else if(string_matched("Jul",word[0]))
        {
            month = 7;
        }
        else if(string_matched("Aug",word[0]))
        {
            month = 8;
        }
        else if(string_matched("Sept",word[0]) || string_matched("Sep",word[0]) )
        {
            month = 9;
        }
        else if(string_matched("Oct",word[0]))
        {
            month = 10;
        }
        else if(string_matched("Nov",word[0]))
        {
            month = 11;
        }
        else if(string_matched("Dec",word[0]))
        {
            month = 12;
        }

        memset(converted,0,size);
        snprintf(converted, size,"%s-%02d-%s",word[2],month,word[1]);
        free(tmp);
        return converted;
    }
    free(tmp);
    return NULL;
}









bool check_inofitywait_job_pid_exist(char *src)
{
    bool ret =false;
    char res[10]= {""};
    if(!Exec(src, res, 100))
    {
        ret = false;
    }
    else
    {
        if(string_matched(res,"1"))
        {
            printf("match 1\n");
            ret = true;
        }
        else if(string_matched(res,"0"))
        {
            printf("match 0\n");
        }
        else
        {
            printf("no match 0 or 1 ,value[%s]",res);

        }
    }

    char *config_string = NULL;
    config_string = string_replace(res, "\n", "");

    if(string_matched(config_string,"1"))
    {
        printf("replace match 1\n");
        ret = true;
    }
    else if(string_matched(config_string,"0"))
    {
        printf("replace match 0\n");
    }
    else
    {
        printf("replace no match 0 or 1 ,value[%s]",config_string);

    }

    safe_free(config_string);
    return ret;
}

int main(void)
{
//char resMsg[1024]={""};
//  Exec("touch /var/log/mylog &&chmod 777 /var/log/mylog ",resMsg,sizeof(resMsg));
//  memset(resMsg, 0, sizeof(resMsg));
//  Exec("touch /var/log/mywlog &&chmod 777 /var/log/wmylog ",resMsg,sizeof(resMsg));
//  char la = 0xA;
//  printf("%c",la);
//
//  char lr = '\r';
//  printf("%d",lr);
//
//  char ln = '\n';
//  printf("%d",ln);
//    check_inofitywait_job_pid_exist("ps -axu | grep syslogggg | grep -v grep | wc -l");
    char *src =NULL;
    char *dst = NULL;
    char *skipWhited = NULL;
    char *exitStr = "exit";
    char *replaced=NULL;
    src = (char*)malloc(200*sizeof(char));
    char resMsg[1024]= {""};
    //dst = (char*)malloc(strlen(s)*sizeof(char));//100*sizeof(char)
    char resTmp[256]= {""};

    //char cmdBuf[256]= {""};
    //sprintf(cmdBuf,"md5sum %s | awk {'print $1'}","/var/log/wtmp");
    //printf("\n%s\n",cmdBuf);
//	    char test[4]= {"123"};
//		
//	    printf("test[0]=%c",test[start]);
//	    printf("test[1]=%c",test[mid]);
//	    printf("test[2]=%c",test[last]);
//	    printf("test[3]=%c",test[end]);
//	    char digitalDate[20]= {""};
//	    printf("\nconvert date[%s]\n",convert_english_date_to_digital(digitalDate,ARRAY_SIZE(digitalDate),"Jul 20, 2020"));
    while(0!=strcmp(exitStr,src))
    {
        gets(src); 
        //fgets(src, sizeof(src), stdin);
        dst = (char*)malloc(strlen(src)*sizeof(char));//100*sizeof(char)
        //printf("splited string :%s\n" ,replaceMultiSpaceToOne(dst,src));
        //printf("skip white string :%s\n" ,string_skip_whitespace(src));


        //Exec(src,resMsg,sizeof(resMsg));
        //replaced = string_replace(src, " ", "");
        //printf("\nreplaced[%s]\n",replaced);
        //safe_free(replaced);

//	
//	        if(string_reg_verify(src,NEW_PASSWORD_PATTEN_PCRE))
//	        {
//	            printf("string_reg_verify string:%s	pass\n",src);
//	        }
//	        else
//	        {
//	             printf("string_reg_verify string:%s fail\n",src);
//	        }

		printf("\n ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");		
		check_pcre_regex(src,OLD_PASSWORD_PATTEN);
		printf("\n ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");


		printf("\n ######################################################## \n");	
		check_pcre_regex(src,NEW_PASSWORD_PATTEN_PCRE);
		printf("\n ######################################################## \n");	

		printf("\n $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");	
		check_pcre_regex(src,NEW_PASSWORD_PATTEN_PCRE_S);
        printf("\n $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ \n");	

		check_pcre_regex(src,PROTROPORT_PATTEN);
		
        free(dst);
        dst = NULL;
    }

    printf("just add a single printf line \n");
    printf("second printf");
////  char *cmd="md5sum rep | awk {'print $1'}" ;
////  char resMsg[1024]={""};
////  Exec(cmd,resMsg,sizeof(resMsg));
//    free(src);
}
