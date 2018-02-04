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

#ifndef __signal
#define __signal

#include <functional>
#include <vector>

struct null_t {};

template <typename P = null_t>
class signal : public std::vector<std::function<void(const P&)>>
{
	typedef std::function<void(const P&)> FN;
public:
	void connect(const FN& fn) { this->push_back(fn); }
	void operator()(const P& p) { for (auto& fn : *this) fn(p); }
};

template<>
class signal<null_t> : public std::vector<std::function<void()>>
{
	typedef std::function<void()> FN;
public:
	void connect(const FN& fn) { this->push_back(fn); }
	void operator()() { for (auto& fn : *this) fn(); }
};

#endif
