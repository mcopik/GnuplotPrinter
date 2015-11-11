//  Copyright (c) 2015 Marcin Copik mcopik@gmail.com
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)


#ifndef _GNUPLOTPRINTER_H_
#define _GNUPLOTPRINTER_H_

#include <fstream>
#include <string>
#include <tuple>
#include <vector>
#include <memory>
#include <array>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <memory>
#include <map>

#include <cmath>
#include <ctime>

namespace GP { namespace util {

	/**
		Idea taken from Meyers' book "Effective Modern C++"
	**/
	template<typename T>
	constexpr typename std::underlying_type<T>::type get_underlying_type(T enumer) noexcept
	{
		return static_cast< typename std::underlying_type<T>::type >(enumer);
	}

	class GnuplotPrinterExc : public std::runtime_error {
	public:
		GnuplotPrinterExc(const std::string & msg) : std::runtime_error(msg) {}
	};
	
	template<typename ValueType>
	class Iterator {
	public:
		virtual void increment() = 0;
		virtual ValueType operator*() = 0;
	};

	template<typename FwdIter, typename ValueType = typename std::iterator_traits<FwdIter>::value_type>
	class DataIterator : public Iterator< ValueType >  {
	protected:
		FwdIter begin, end;
		bool finished;
	public:
		DataIterator(FwdIter begin, FwdIter end) : begin(begin), end(end) {}
		void increment() override
		{
			++begin;
			finished = begin == end;
		}
		ValueType operator*() override
		{
			if( !finished )
				return *begin;
			throw GnuplotPrinterExc("Dereferencing of an iterator pointing to end!");
		}
	};

	template<typename FwdIter, bool UseFirst, 
		typename IteratorType = typename std::iterator_traits<FwdIter>::value_type,
		typename ValueType = typename std::conditional<UseFirst, typename IteratorType::first_type, typename IteratorType::second_type>::value>
	class PairIterator : public DataIterator<FwdIter, ValueType> {
		using DataIterator<FwdIter, ValueType>::finished;
		using DataIterator<FwdIter, ValueType>::begin;
	public:
		PairIterator(FwdIter begin, FwdIter end) : DataIterator<FwdIter, ValueType>(begin, end) {}
		ValueType operator*() override
		{
			if( !finished )
				return UseFirst ? (*begin).first : (*begin).second;
			throw GnuplotPrinterExc("Dereferencing of an iterator pointing to the end!");
		}
	};
	
} }

namespace GP {

	template<typename YCoordType, typename XCoordType = int>
	class GnuplotPrinter
	{
		using XDataContainer = std::vector< std::unique_ptr< util::Iterator<XCoordType> > >;
		using index_t = typename XDataContainer::size_type;
		using YDataContainer = std::multimap< index_t, std::unique_ptr< util::Iterator<YCoordType> > >;

		std::string title, xLabel, yLabel, suffix;

		static constexpr int AXES_COUNT = 2;
		int axesLog[AXES_COUNT] = {-1, -1};
		YCoordType axesLimits[AXES_COUNT][2] = { {NAN, NAN}, {NAN, NAN}};

		std::vector< std::tuple<uint32_t, std::vector<XCoordType>> > xData;
		std::vector< std::tuple<uint32_t, std::vector<YCoordType>, std::string> > yData;

		XDataContainer xAxisData;
		YDataContainer yAxisData;
	public:

		enum class Axis {
			X = 0,
			Y
		};

		GnuplotPrinter(const std::string & _title = "Default title", const std::string & _xLabel = "X",
				const std::string & _yLabel = "Y"): title(_title),xLabel(_xLabel),yLabel(_yLabel),suffix("") {}

		GnuplotPrinter(const GnuplotPrinter & copy) : title(copy.title),xLabel(copy.xLabel),yLabel(copy.yLabel)
		{
			//copy assignment
			xData = copy.xData;
			yData = copy.yData;
		}

		virtual ~GnuplotPrinter() {}
		
		template<typename FwdIter,
				 typename = std::enable_if< std::is_same<XCoordType, typename std::iterator_traits<FwdIter>::value_type >::value > >
		index_t addXSet(FwdIter begin, FwdIter end)
		{
			xAxisData.push_back( std::make_unique< util::DataIterator<FwdIter> >( move(begin), move(end)) );
			return xAxisData.size() - 1;
		}

		template<typename FwdIter,
				 typename = std::enable_if< std::is_same<YCoordType, typename std::iterator_traits<FwdIter>::value_type >::value > >
		void addYSet(index_t xIndex, FwdIter begin, FwdIter end)
		{
			if(xIndex >= xData.size()) {
				throw std::invalid_argument("Wrong X data set indice!");
			}
			yAxisData.insert( xIndex, std::make_unique< util::DataIterator<FwdIter> >( move(begin), move(end)) );
		}

		void setTitle(const std::string & title)
		{
			this->title = title;
		}

		void setXlabel(const std::string & label)
		{
			xLabel = label;
		}

		void setYLabel(const std::string & label)
		{
			yLabel = label;
		}

		void setLogScale(Axis axis, int base = 10)
		{
			axesLog[util::get_underlying_type(axis)] = base;
		}

		void resetLogScale(Axis axis)
		{
			axesLog[util::get_underlying_type(axis)] = -1;
		}

		void setLimits(Axis axis, YCoordType yMin, YCoordType yMax)
		{
			axesLimits[util::get_underlying_type(axis)][0] = yMin;
			axesLimits[util::get_underlying_type(axis)][1] = yMax;
		}

		void resetLimits(Axis axis)
		{
			axesLimits[util::get_underlying_type(axis)][0] = NAN;
			axesLimits[util::get_underlying_type(axis)][1] = NAN;
		}


		int add_xSet(const std::vector<XCoordType> & x)
		{
			xData.push_back(std::make_tuple(x.size(),x));
			return xData.size() - 1;
		}

		void add_ySet(uint32_t x_index,const std::vector<YCoordType> & y,const std::string & label)
		{
			if(x_index >= xData.size()) {
				throw std::invalid_argument("Wrong X data set indice!");
			}
			yData.push_back(std::make_tuple(x_index,y,label));
		}

		void save(const std::string & file, bool toPng = false)
		{

			std::fstream fileOut(file, std::fstream::out);
			//scale axes
			fileOut << "set autoscale" << std::endl;
			fileOut << "set xtic auto" << std::endl;
			fileOut << "set ytic auto" << std::endl;
			fileOut << "set grid" << std::endl;
			fileOut << "set title \"" << title << "\"" << std::endl;
			fileOut << "set xlabel \"" << xLabel << "\"" << std::endl;
			fileOut << "set ylabel \"" << yLabel << "\"" << std::endl;
			fileOut << "set format y '%.2e'" << std::endl;
			if( !isnan(axesLimits[0][0]) ) {
				fileOut << "set xrange [" << axesLimits[0][0] << ":" << axesLimits[0][1] << "]" << std::endl;
			}
			if( !isnan(axesLimits[1][0]) ) {
				fileOut << "set yrange [" << axesLimits[1][0] << ":" << axesLimits[1][1] << "]" << std::endl;
			}
			fileOut << "set key box opaque" << std::endl;
			if(toPng) {
				fileOut << "set terminal pngcairo size 1000,800 enhanced font \"Helvetica,20\"" << std::endl;
				fileOut << "set output '" << file << ".png'" << std::endl;
			}
			for(int i = 0;i < 2;++i) {
				if(axesLog[i] != -1) {
					fileOut << "set logscale " << (axesLog[i] == 0 ? "x " : "y ") << axesLog[i] << std::endl;
				}
			}
			fileOut << "plot ";
			for(size_t i = 0;i < yData.size();++i) {
				auto & x = yData[i];
				fileOut << "\"" << file << ".dat\"" << " using " << std::get<0>(x)+1 << ":" <<
						i+xData.size()+1 << " title \"" << std::get<2>(x) << "\" with line";
				if(i != yData.size() - 1) {
					fileOut << " , ";
				} else {
					fileOut << std::endl;
				}
			}

			fileOut << "pause -1 \"Press any key to close\"" << std::endl;
			fileOut.close();

			//write data
			fileOut.open(file + ".dat",std::fstream::out);

			std::vector< const std::vector<XCoordType> * > xptrs;
			std::vector< const std::vector<YCoordType> * > yptrs;
			xptrs.reserve(xData.size());
			yptrs.reserve(yData.size());
			uint32_t max_size = 0;
			for(auto & x : xData) {
				xptrs.push_back( &std::get<1>(x) );
				max_size = std::max(max_size,std::get<0>(x));
			}
			for(auto & x : yData) {
				yptrs.push_back( &std::get<1>(x) );
			}
			for(uint32_t i = 0;i < max_size;++i) {

				auto x_it = xptrs.begin(), y_it = yptrs.begin();

				for(auto & x : xData) {
					if(std::get<0>(x) > i) {
						// get pointer and increment iterator; then get double value and increment pointer
						fileOut << (*x_it++)->at(i) << "\t";
					} else {
						++x_it;
						fileOut << ".\t";
					}
				}

				for(auto & y : yData) {
					if(std::get<0>(xData[std::get<0>(y)]) > i && std::get<1>(y).size() > i) {
						fileOut << (*y_it++)->at(i) << "\t";
					} else {
						++y_it;
						fileOut << ".\t";
					}
				}
				fileOut << std::endl;
			}
			fileOut.close();

		}

		void save(bool toPng = false)
		{
			/**
			 * C++11 put_time is not yet supported by GCC, so we have to use
			 * C tools.
			 */
			std::time_t result = std::time(nullptr);
			struct tm * timeinfo = std::localtime(&result);
			char buffer[256];
			std::strftime (buffer,256,"gnuplot_data_%H_%M_%S_%d_%m_%y",timeinfo);

			return save(std::string(buffer)+"_"+suffix, toPng);
		}

		void save(int N, bool toPng = false)
		{	
			std::time_t result = std::time(nullptr);
			struct tm * timeinfo = std::localtime(&result);
			char buffer[256];
			std::strftime (buffer,256,"gnuplot_data_%H_%M_%S_%d_%m_%y",timeinfo);

			return save(std::string(buffer)+"_"+suffix+"_"+std::to_string(N), toPng);
		}

		void set_file_suffix(const std::string & suffix)
		{
			this->suffix = suffix;
		}

	};

}

#endif /* _GNUPLOTPRINTER_H_ */
