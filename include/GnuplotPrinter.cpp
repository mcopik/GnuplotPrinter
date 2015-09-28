/*
 * GnuplotPrinter.cpp
 *
 *  Created on: Nov 28, 2014
 *      Author: mcopik
 */
#include <stdexcept>
#include <cmath>
#include <ctime>
#include <iostream>
#include "GnuplotPrinter.h"

void GnuplotPrinter::setTitle(const std::string & title)
{
	this->title = title;
}

void GnuplotPrinter::setXLabel(const std::string & label)
{
	xLabel = label;
}

void GnuplotPrinter::setYLabel(const std::string & label)
{
	yLabel = label;
}

void GnuplotPrinter::setLogScale(const int ax, const int base)
{
	if(ax < 0 || ax > 1) {
		throw std::invalid_argument("Invalid ax number; 0 - X, 1 - Y, given " + ax);
	}
	axesLog[ax] = base;
}

void GnuplotPrinter::setLimits(int ax, double min, double max)
{
	if(ax < 0 || ax > 1) {
		throw std::invalid_argument("Invalid ax number; 0 - X, 1 - Y, given " + ax);
	}
	axesLimits[ax][0] = min;
	axesLimits[ax][1] = max;
	axesSetLimit[ax] = true;
}

void GnuplotPrinter::unsetLogScale(const int ax)
{
	if(ax < 0 || ax > 1) {
		throw std::invalid_argument("Invalid ax number; 0 - X, 1 - Y, given " + ax);
	}
	axesLog[ax] = -1;
}

int GnuplotPrinter::addXSet(const vector<double> & x)
{
	xData.push_back(std::make_tuple(x.size(),x));
	return xData.size() - 1;
}

void GnuplotPrinter::addYSet(uint32_t x_index, const vector<double> & y,const std::string & label)
{
	if(x_index >= xData.size()) {
		throw std::invalid_argument("Wrong X data set indice!");
	}
	yData.push_back(std::make_tuple(x_index,y,label));
}

void GnuplotPrinter::save(const std::string & file, bool toPng)
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
	if( axesSetLimit[0] ) {
		fileOut << "set xrange [" << axesLimits[0][0] << ":" << axesLimits[0][1] << "]" << std::endl;
	}
	if( axesSetLimit[1] ) {
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
	std::vector< vector<double> > ptrs;
	ptrs.reserve(xData.size() + yData.size());
	uint32_t max_size = 0;
	for(auto & x : xData) {
		ptrs.push_back( std::get<1>(x) );
		max_size = std::max(max_size,std::get<0>(x));
	}
	for(auto & x : yData) {
		ptrs.push_back( std::get<1>(x) );
	}
	for(uint32_t i = 0;i < max_size;++i) {

		std::vector< vector<double> >::iterator it = ptrs.begin();
		for(auto & x : xData) {
			if(std::get<0>(x) > i) {
				// get pointer and increment iterator; then get double value and increment pointer
				fileOut << (*it++).at(i) << "\t";
			} else {
				++it;
				fileOut << ".\t";
			}
		}
		for(auto & y : yData) {
			if(std::get<0>(xData[std::get<0>(y)]) > i && std::get<1>(y).size() > i) {
				fileOut << (*it++).at(i) << "\t";
			} else {
				++it;
				fileOut << ".\t";
			}
		}
		fileOut << std::endl;
	}
	fileOut.close();
}

void GnuplotPrinter::save(bool toPng)
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

void GnuplotPrinter::save(int N, bool toPng)
{
    std::time_t result = std::time(nullptr);
        struct tm * timeinfo = std::localtime(&result);
    char buffer[256];
    std::strftime (buffer,256,"gnuplot_data_%H_%M_%S_%d_%m_%y",timeinfo);
    return save(std::string(buffer)+"_"+suffix+"_"+std::to_string(N), toPng);
}

void GnuplotPrinter::setFileSuffix(const std::string & suffix)
{
	this->suffix = suffix;
}
