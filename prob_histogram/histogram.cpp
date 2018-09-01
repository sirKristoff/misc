/*
 * histogram.cpp
 *
 *  Created on: 01.09.2018
 *      Author: Krzysztof Lasota
 */

#include "histogram.hpp"

#include <algorithm>
#include <iterator>


Histogram::size_type Histogram::size() const
{
	return mContainer.size();
}


Histogram& Histogram::operator+=(const Histogram& rhs)
{
	if (mContainer.size() < rhs.mContainer.size())
		mContainer.resize(rhs.mContainer.size(), value_type());

	for (size_type idx = 0; idx < rhs.mContainer.size(); ++idx)
			mContainer[idx] += rhs.mContainer[idx];

	return *this;
}


Histogram Histogram::operator<<(const size_type offset) const
{
	Histogram ret(0);
	auto& container = ret.mContainer;

	container.resize(mContainer.size() + offset, value_type());
	std::copy(mContainer.rbegin(), mContainer.rend(), container.rbegin());

	return ret;
}


std::ostream& operator<<(std::ostream& ostr, const Histogram& h)
{
	const char* tagOpen = "";
	const char* tagClose = "";
	const char* tagSeparator = "\n";

	ostr << tagOpen;
	std::copy(h.mContainer.begin(), h.mContainer.end(),
			std::ostream_iterator<Histogram::value_type>(ostr, tagSeparator));
	ostr << tagClose;

	return ostr;
}
