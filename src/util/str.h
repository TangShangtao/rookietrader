//
// Created by root on 2025/10/10.
//

#pragma once
#include <iconv.h> //for gbk/big5/utf8
#include <cstring>
#include <string>
namespace rk::util
{

    inline size_t code_convert(char *from_charset,char *to_charset,char *inbuf, size_t inlen,char *outbuf,size_t outlen)
    {
        iconv_t cd;
//            int rc;
        char **pin = &inbuf;
        char **pout = &outbuf;
        cd = iconv_open(to_charset,from_charset);
        memset(outbuf,0,outlen);
        auto res = iconv(cd,pin,&inlen,pout,&outlen);
        iconv_close(cd);
        return res;
    }
    inline std::string any2utf8(std::string in,std::string fromEncode,std::string toEncode)
    {
        char* inbuf=(char*) in.c_str();
        int inlen=strlen(inbuf);
        int outlen=inlen*3;//in case unicode 3 times than ascii
        char outbuf[outlen]={0};
        int rst=code_convert((char*)fromEncode.c_str(),(char*)toEncode.c_str(),inbuf,inlen,outbuf,outlen);
        if(rst==0){
            return std::string(outbuf);
        }else{
            return in;
        }
    }
    inline std::string gbk2utf8(const char* in)
    {
        return any2utf8(std::string(in),std::string("gbk"),std::string("utf-8"));
    }
    inline std::string trim_copy(std::string s) {
        const char* ws = " \t\r\n";
        auto b = s.find_first_not_of(ws);
        if (b == std::string::npos) return {};
        auto e = s.find_last_not_of(ws);
        return s.substr(b, e - b + 1);
    }
    inline std::string to_hex_string(const std::string& input)
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (unsigned char c : input) {
            ss << std::setw(2) << static_cast<int>(c);
        }
        return ss.str();
    }
};