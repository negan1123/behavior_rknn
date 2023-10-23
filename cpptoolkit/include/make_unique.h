/*
 * make_unique.h
 * c++11不支持std::make_unique，如果在低于6.2版本gcc编译程序，可能会报错：‘make_unique’不是‘std’的成员
 * 这是因为make_unique在c++14中是标准实现，c++11中不包含，为了保证正常编译通过而增加了这个头文件
 *
 *  Created on: 2019年10月31日
 *      Author: bnkj
 */

#ifndef INCLUDE_MAKE_UNIQUE_H_
#define INCLUDE_MAKE_UNIQUE_H_

#ifdef __GNUC__
#ifdef __GNUC_MINOR__
#if(__GNUC__ < 6) || ((__GNUC__ == 6) && (__GNUC_MINOR__ < 2))
	namespace std {
		template<typename T, typename... Ts>
			std::unique_ptr<T> make_unique(Ts&&... params) {
				return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
			}
	}
#endif
#endif // __GNUC_MINOR__
#endif // __GNUC__

#endif /* INCLUDE_MAKE_UNIQUE_H_ */
