/**
 * Copyright (C) 2015-2019，大连商品交易所大连飞创信息技术有限公司
 *
 * @file <collect.h>
 * @brief <穿透监管采集库接口声明>
 * @author <X1项目组>
 * @date <2018年11月8日>
 */


#ifndef __DFITC_COLLECTION_H_
#define __DFITC_COLLECTION_H_

#if !defined(WIN32) && !defined(_WIN32)
#define DLL_API __attribute__((visibility("default")))
#else
#define DLL_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif  //__cplusplus
/**
 * @brief 获取采集信息密文
 * {@param[out] <pSysteminfo><用于存放采集信息密文>}
 * {@param[in&out] <nLen><输入缓冲区的最大值，目前采集信息密文最大长度不会超过1024> 输出采集信息密文的实际长度}
 * {@param[in] <pub_key_idx><加密秘钥索引，0正式，1测试}
 * {@retval <int>}
 * {@return <0 表示正常; -1 表示参数错误; -2 表示权限不足; 正数 表示out大小应大于此值>}
 */
DLL_API int Dfitc_GetSystemInfo(char* pSysteminfo, int& nLen, unsigned int pub_key_idx = 0);
/**
 * @brief 获取穿透监管采集库版本号
 * {@retval <const char *>}
 * {@return <获取穿透监管采集库版本号>}
 */
DLL_API const char* GetDfitcVersion();
/**
 * @brief: 获取加密key值
 * @author: wangchongqi
 * @param {int} type
 */
DLL_API const char* GetRsaPublicKey(int type);

#ifdef __cplusplus
};
#endif  //__cplusplus

#endif  //__DFITC_COLLECTION_H_