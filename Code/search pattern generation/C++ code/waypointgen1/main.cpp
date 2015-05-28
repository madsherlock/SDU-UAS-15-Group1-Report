/**
 * @file main.cpp
 * @author Mikael Westermann
 */
#include <iostream>
#include <vector>
#include "waypointgen.h"

using namespace std;

/**
 * @brief kmltest Tests some kml manipulation functions.
 */
void kmltest() {
	VecCoord c = Kmlmanip::kml_extract_coordinates("teststi.kml");
	for(auto s:c)
		cout << s << endl;
	Kmlmanip::kml2csv_coords("teststi.kml");
	cout << "home coordinate:" << endl;
	MAVLink::coordinate home(55.476130,10.330925,550.000000);
	cout << home << endl;
	cout << "Command:" << endl;
	MAVLink::command cmd(16,0,0,0,0,home);
	cout << cmd << endl;
	size_t index = 0;
	cout << "Waypoint:" << endl;
	MAVLink::waypoint wpt(index++,1,0,cmd,1);
	cout << wpt << endl;
	MAVLink::waypoint lolpoint(index++,0,0,16,0,0,0,0,55.476130,10.330925,550.000000,1);
	cout << lolpoint << endl;
}

/**
 * @brief main Converts kml files to waypoint txt files, while outputting to console.
 * @param argc 1 + Number of arguments
 * @param argv kml file names
 * @return 0
 */
int main(int argc, char** argv) {
	int no_of_files = argc-1;
	cout << "kml to waypoint file converter by Mikael Westermann.\n"
		 << "Pass kml file name(s) as argument(s) "
		 << "when running this program." << endl;
	if(no_of_files>0) {
		cout << "Converting " << no_of_files << " file" << (no_of_files!=1?"s:":":") << endl;
		for(int i=1; i<argc; ++i) {
			string kmlfile(argv[i]);
			cout << " Converting kml file \"" << kmlfile
				 << "\" to waypoint file \""
				 << (kmlfile.substr(0,kmlfile.find_last_of('.'))+".txt")
				 << "\"... " << flush;
			try {
				Kmlmanip::kml2wp(string(argv[i]));
				cout << "Done!";
			} catch(const file_error &e) {
				cout << (e.what());
				--no_of_files;
			}
			cout << endl;
		}
		if(no_of_files==argc-1)
			cout << "All files have been converted." << endl;
		else
			cout << no_of_files << " file" << (no_of_files!=1?"s have":" has")
				 << " been converted." << endl;
	}
	else {
		cout << "No file names passed as arguments. Exiting." << endl;
	}
	return 0;
}

