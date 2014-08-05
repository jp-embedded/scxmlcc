/*************************************************************************
 ** Copyright (C) 2014 Jan Pedersen <jp@jp-embedded.com>
 ** 
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation, either version 3 of the License, or
 ** (at your option) any later version.
 ** 
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 ** 
 ** You should have received a copy of the GNU General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************/

#include <vector>
#include <memory>

#include <iostream>

struct null_t {};

template<class P0 = null_t> class functor_impl
{
	public:
	virtual void operator()(P0) = 0;
	virtual ~functor_impl() {}
};

template<> class functor_impl<null_t>
{
	public:
	virtual void operator()() = 0;
	virtual ~functor_impl() {}
};

// hander for functors and function pointers
template <class FN, class P0> class functor_handler : public functor_impl<P0>
{
	public:
	functor_handler(const FN& fn) : fun(fn) {}

	void operator()() { fun(); }
	void operator()(P0 p0) { fun(p0); }
	
	private:
	FN fun;
};

// handler for membur function
template <class O, class PMFN, class P0> class memfun_handler : public functor_impl<P0>
{
	public: 
	memfun_handler(const O& o, PMFN pm) : obj(o), pmfn(pm) {}

	void operator()() { return ((*obj).*pmfn)(); }
	void operator()(P0 p0) { return ((*obj).*pmfn)(p0); }

	private:
	O obj;
	PMFN pmfn;
};

template<class P0 = null_t> class functor
{
	public:
	template <class FN> functor(const FN& fn) : impl(new functor_handler<FN, P0>(fn)) {};
	template <class O, class MFN> functor(const O& o, MFN mfn) : impl(new memfun_handler<O, MFN, P0>(o, mfn)) {};

	void operator()() { return (*impl)(); }
	void operator()(P0 p0) { return (*impl)(p0); }

	private:
	std::auto_ptr<functor_impl<P0> > impl;
};

template<class P0 = null_t> class signal : public std::vector<std::auto_ptr<functor_impl<P0> > >
{
	public:
	virtual void operator()() { for (typename impl::iterator i = impl::begin(); i != impl::end(); ++i) (**i)(); }
	
	private:
	typedef std::vector<std::auto_ptr<functor_impl<P0> > > impl;
};
