/*
 * GnuplotPrinter.h
 *
 *  Created on: Nov 28, 2014
 *      Author: mcopik
 */

#ifndef WEEK_4_GNUPLOTPRINTER_H_
#define WEEK_4_GNUPLOTPRINTER_H_
#include <fstream>
#include <string>
#include <tuple>
#include <vector>
#include <memory>

using std::vector;
using std::tuple;
using std::string;
using std::shared_ptr;

class GnuplotPrinter {
	std::string title,xLabel,yLabel,suffix;
	int axesLog[2] = {-1,-1};
	bool axesSetLimit[2] = { false, false};
	double axesLimits[2][2] = { {-1, -1}, {-1, -1}};

	vector< tuple<uint32_t, vector<double>> > xData;
	vector< tuple<uint32_t, vector<double>,string> > yData;
public:
	GnuplotPrinter(const std::string & _title = "Default title", const std::string & _xLabel = "X",
			const std::string & _yLabel = "Y"): title(_title),xLabel(_xLabel),yLabel(_yLabel),suffix("") {}
	GnuplotPrinter(const GnuplotPrinter & copy) : title(copy.title),xLabel(copy.xLabel),yLabel(copy.yLabel)
	{
		//copy assignment
		xData = copy.xData;
		yData = copy.yData;
	}
	virtual ~GnuplotPrinter() {}
	void setTitle(const std::string & title);
	void setXLabel(const std::string & label);
	void setYLabel(const std::string & label);
	void setLogScale(const int ax, const int base = 10);
	void setLimits(int ax, double yMin, double yMax);
	void unsetLogScale(const int ax);
	int addXSet(const vector<double> & x);
	void addYSet(uint32_t x_index,const vector<double> & y,const std::string & label);
	void save(const std::string & file, bool toPng = false);
	void save(bool toPng = false);
	void save(int N, bool toPng = false);
	void setFileSuffix(const std::string & suffix);
};

#endif /* WEEK_4_GNUPLOTPRINTER_H_ */
