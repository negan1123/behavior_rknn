/* Interface for CRC-24Q cyclic redundancy chercksum code
 *
 * This file is Copyright (c) 2010 by the GPSD project
 * SPDX-License-Identifier: BSD-2-clause
 */
#ifndef _CRC24Q_H_
#define _CRC24Q_H_

//在数据后面增加crc24q的校验数据
void crc24q_sign(unsigned char *data, int len);

//对数据进行crc24q检查，返回true表示crc24q校验通过，false表示校验失败
bool crc24q_check(unsigned char *data, int len);

//返回指定数据的crc24q的数据项，返回的是unsigned，四个字节，其中第一个字节为00
unsigned crc24q_hash(unsigned char *data, int len);
#endif /* _CRC24Q_H_ */
