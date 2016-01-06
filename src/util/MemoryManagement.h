#ifndef UTIL_MEMORYMANAGEMENT_H_
#define UTIL_MEMORYMANAGEMENT_H_

#include <memory>
#include <utility>

template<typename T, typename ... Ts>
std::unique_ptr<T> make_unique(Ts&&... params) {
	return std::unique_ptr < T > (new T(std::forward<Ts>(params)...));
}

template<typename T, typename ... Ts>
std::shared_ptr<T> make_shared(Ts&&... params) {
	return std::shared_ptr < T > (new T(std::forward<Ts>(params)...));
}
#endif
