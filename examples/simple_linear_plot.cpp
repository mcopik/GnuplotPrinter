#include <vector>
#include <GnuplotPrinter.h>


int main(int argc, char ** argv)
{
	GP::GnuplotPrinter<double> printer("Linear function");

	constexpr int POINTS_COUNT = 100;
	std::vector<int> x_vals(POINTS_COUNT);
	std::vector<double> y_vals(POINTS_COUNT);

	for(int i = 0; i < POINTS_COUNT; ++i) {
		x_vals[i] = y_vals[i] = i;
	}

	int idx = printer.add_xSet(x_vals);
	printer.add_ySet(idx, y_vals, "Meaningless data");
	printer.save(true);

	return 0;
}
