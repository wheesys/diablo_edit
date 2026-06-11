#pragma once

#include <vector>
#include <cassert>
#include <utility>

#include "BinDataStream.h"

template<class T, int N = 1>
class MayExist;

template<class T, int N>
class MayExist : std::vector<T>
{
	static_assert(N > 0);
	typedef std::vector<T> __Base;
public:
	__Base & ensure() { this->resize(N); return *this; }
	void reset() { __Base::clear(); }
	bool exist() const { return !__Base::empty(); }
	using __Base::begin;
	using __Base::end;
	using __Base::size;
	using __Base::operator [];
};

template<int N>
class MayExist<BYTE, N> : std::vector<BYTE>
{
	static_assert(N > 0);
	typedef std::vector<BYTE> __Base;
public:
	__Base & ensure() { this->resize(N); return *this; }
	void reset() { __Base::clear(); }
	bool exist() const { return !__Base::empty(); }
	using __Base::begin;
	using __Base::end;
};

template<class T>
class MayExist<T, 1>
{
	std::vector<T> v_;
public:
	template<class ...A>
	T & ensure(A &&... args) {
		if(v_.empty())
			v_.emplace_back(std::forward<A>(args)...);
		return v_.front();
	}
	void reset() { v_.clear(); }
	bool exist() const { return !v_.empty(); }
	const T * operator ->() const { return &operator *(); }
	T * operator ->() { return &operator *(); }
	const T & operator *() const { return v_.front(); }
	T & operator *() { return v_.front(); }
	void swap(MayExist & a) { v_.swap(a.v_); }
};

template<class T>
CInBitsStream & operator >>(CInBitsStream & bs, MayExist<T, 1> & v) {
	return bs >> v.ensure();
}

template<class T>
COutBitsStream & operator <<(COutBitsStream & bs, const MayExist<T, 1> & v) {
	return bs << *v;
}

// MAY_EXIST_POD specializations for BYTE and WORD

template<>
class MayExist<BYTE, 1> {
	BYTE v_ = 0;
	bool e_ = false;
public:
	MayExist() = default;
	MayExist(const MayExist&) = default;
	BYTE & ensure(BYTE v = 0) { v_ = v, e_ = true; return v_; }
	void reset() { e_ = false; v_ = 0; }
	bool exist() const { return e_; }
	BYTE operator =(BYTE v) { assert(e_); return v_ = v; }
	MayExist & operator =(const MayExist & a) { v_ = a.v_; e_ = a.e_; return *this; }
	operator BYTE() const { assert(e_); return v_; }
};

inline CInBitsStream & operator >>(CInBitsStream & bs, const Bits<MayExist<BYTE, 1>>& m) {
	return bs >> bits(m.value().ensure(), m.bits());
}
inline COutBitsStream & operator <<(COutBitsStream & bs, const Bits<const MayExist<BYTE, 1>>& m) {
	BYTE val = m.value();
	return bs << bits(val, m.bits());
}

template<>
class MayExist<WORD, 1> {
	WORD v_ = 0;
	bool e_ = false;
public:
	MayExist() = default;
	MayExist(const MayExist&) = default;
	WORD & ensure(WORD v = 0) { v_ = v, e_ = true; return v_; }
	void reset() { e_ = false; v_ = 0; }
	bool exist() const { return e_; }
	WORD operator =(WORD v) { assert(e_); return v_ = v; }
	MayExist & operator =(const MayExist & a) { v_ = a.v_; e_ = a.e_; return *this; }
	operator WORD() const { assert(e_); return v_; }
};

inline CInBitsStream & operator >>(CInBitsStream & bs, const Bits<MayExist<WORD, 1>>& m) {
	return bs >> bits(m.value().ensure(), m.bits());
}
inline COutBitsStream & operator <<(COutBitsStream & bs, const Bits<const MayExist<WORD, 1>>& m) {
	WORD val = m.value();
	return bs << bits(val, m.bits());
}
