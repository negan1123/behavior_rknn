// MIT License
//
// Copyright (c) 2020 Yuming Meng
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// @File    :  util.cc
// @Version :  1.0
// @Time    :  2020/06/24 10:04:23
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None
#include <string.h>
#include <iconv.h>
#include <util.h>


namespace libjt808 {

// 转义函数.
int Escape(std::vector<uint8_t> const& in,
           std::vector<uint8_t>* out) {
  return 0;
}

// 逆转义函数.
int ReverseEscape(std::vector<uint8_t> const& in,
                  std::vector<uint8_t>* out) {
  return 0;
}

// 奇偶校验.
uint8_t BccCheckSum(const uint8_t *src, const size_t &len) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < len; ++i) {
    checksum = checksum ^ src[i];
  }
  return checksum;
}

/*代码转换:从一种编码转为另一种编码*/
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
    iconv_t cd;
//    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;
    size_t inl = inlen;
    size_t outl = outlen;

    cd = iconv_open(to_charset,from_charset);
    if (cd==0) return -1;
    memset(outbuf,0,outlen);
    iconv(cd,pin,&inl,pout, &outl);
    iconv_close(cd);
    **pout = '\0';
    return 0;
}
/*UNICODE码转为GB2312码*/
int u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
    return code_convert(const_cast<char*>("utf-8"),const_cast<char*>("gb2312"),inbuf,inlen,outbuf,outlen);
}
/*GB2312码转为UNICODE码*/
int g2u(char *inbuf,size_t inlen,char *outbuf,int outlen)
{
    return code_convert(const_cast<char*>("gb2312"),const_cast<char*>("utf-8"),inbuf,inlen,outbuf,outlen);
}

}  // namespace libjt808
