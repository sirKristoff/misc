/*
 * histogram.hpp
 *
 *  Created on: 01.09.2018
 *      Author: Krzysztof Lasota
 */

#ifndef HISTOGRAM_HPP_
#define HISTOGRAM_HPP_

#include <ostream>
#include <vector>


class Histogram
{
	typedef  std::vector<double>  Container;

public:
	typedef  double  value_type;
	typedef  typename Container::size_type  size_type;

	explicit
	Histogram(size_type size_ = size_type(1), const value_type& seed_ = value_type())
	 : mContainer(size_, seed_)
	{
	}

	size_type size() const;

	Histogram& operator+=(const Histogram& rhs);
	Histogram operator<<(const size_type offset) const;
	friend std::ostream& operator<<(std::ostream& ostr, const Histogram& h);

private:
	Container mContainer;
};

#endif /* HISTOGRAM_HPP_ */
