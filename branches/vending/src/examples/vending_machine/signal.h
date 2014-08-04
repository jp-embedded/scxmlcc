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

/*
   todo test

#include "signal.h"

class test
{
	public:
	void a() { std::cout << 'a' << std::endl; }
};

int main()
{
	signal<> s;
	//functor<> f;
//	s.push_back(f);
//	s.push_back(f);

	functor<> f2(&test::a);
	f2();

	s();
	return 0;
}

   */


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

template<class P0 = null_t> class functor
{
	public:
	template <class F> functor(const F& f) : impl(new functor_handler<functor, F>(f) {};

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
