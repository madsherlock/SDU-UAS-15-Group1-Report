/**
 * @file waypointgen.h
 * @author Mikael Westermann
 */
#pragma once
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <tuple>
#include <vector>

/**
  * Please refer to https://pixhawk.ethz.ch/mavlink/
  * and to http://qgroundcontrol.org/mavlink/
  * the former being thorough and the latter giving a nice overview.
  *
  * Waypoint file format (http://qgroundcontrol.org/mavlink/waypoint_protocol):
  * QGC WPL <VERSION>
  * <INDEX> <CURRENT WP> <COORD FRAME> <COMMAND> <PARAM1> <PARAM2> <PARAM3> <PARAM4> <PARAM5/X/LONGITUDE> <PARAM6/Y/LATITUDE> <PARAM7/Z/ALTITUDE> <AUTOCONTINUE>
  *
  * There's just one problem: They seem to have switched longitude with latitude.
  * Parameter 5 should be latitude and parameter 6 should be longitude.
  * Just like MAV_CMD_NAV_WAYPOINT format described at https://pixhawk.ethz.ch/mavlink/.
  * So the file format of the waypoints in this code is:
  * QGC WPL 110
  * <INDEX> <CURRENT WP> <COORD FRAME> <COMMAND> <PARAM1> <PARAM2> <PARAM3> <PARAM4> <PARAM5/X/LATITUDE> <PARAM6/Y/LONGITUDE> <PARAM7/Z/ALTITUDE> <AUTOCONTINUE>
  *
  * Google kml coordinate format:
  * LONGITUDE,LATITUDE,ALTITUDE LONGITUDE,LATITUDE,ALTITUDE
  */

using namespace std;

/**
 * @brief The file_error struct An exception for file access.
 */
struct file_error : public runtime_error {
	/**
	 * @brief file_error ctor
	 * @param filename name of file that can't be opened/created
	 * @param opening_file is this a file we're trying to open (not create) ?
	 */
	file_error(const string& filename, bool opening_file)
		:runtime_error(
			 (string("Error ")+string(opening_file?"opening":"creating")
			  +string(" file \"")+filename+string("\".")))
	{ }
};

namespace MAVLink {
/**
 * @brief The coordinate struct 3 doubles: latitude, longitude and altitude
 */
struct coordinate : public tuple<double,double,double> {
	/**
	 * @brief coordinate ctor
	 * @param lat Latitude
	 * @param lon Longitude
	 * @param alt Altitude
	 */
	coordinate(double lat, double lon, double alt)
		:tuple<double,double,double>::tuple(lat,lon,alt) { }

	/**
	 * @brief get_lat
	 * @return Latitude
	 */
	const double &get_lat() const { return get<0>((*this)); }
	/**
	 * @brief get_lon
	 * @return Longitude
	 */
	const double &get_lon() const { return get<1>((*this)); }
	/**
	 * @brief get_alt
	 * @return Altitude
	 */
	const double &get_alt() const { return get<2>((*this)); }
	/**
	 * @brief operator<< Comma-separated coordinate output
	 * @param out Output stream
	 * @param c Coordinate to output
	 * @return out
	 */
	friend ostream& operator<<(ostream &out, const coordinate &c) {
		return out << fixed << setprecision(14)
				   << c.get_lat() << ", " << c.get_lon() << ", " << c.get_alt();
	}
};

/**
 * @brief The command struct A MAVLink command body
 */
struct command {
	/** @brief cmd CMD_ID*/
	size_t cmd;
	/**
	 * @brief p1 Param 1
	 * @brief p2 Param 2
	 * @brief p3 Param 3
	 * @brief p4 Param 4
	 */
	double p1,p2,p3,p4;
	/** @brief coord Coordinate*/
	coordinate coord;
	/**
	 * @brief command ctor
	 * @param cmd CMD_ID
	 * @param p1 Param 1
	 * @param p2 Param 2
	 * @param p3 Param 3
	 * @param p4 Param 4
	 * @param coord Coordinate
	 */
	command(size_t cmd, double p1, double p2, double p3, double p4, coordinate coord)
		:cmd(cmd),p1(p1),p2(p2),p3(p3),p4(p4),coord(coord) { }

	/**
	 * @brief operator<< Tab-separated command output
	 * @param out Output stream
	 * @param c Command
	 * @return out
	 */
	friend ostream& operator<<(ostream &out, const command &c) {
		out.unsetf(ios_base::floatfield);
		return out << (c.cmd) << "\t" << fixed << setprecision(14)
				   << (c.p1) << "\t" << (c.p2)
				   << "\t" << (c.p3) << "\t" << (c.p4) << "\t"
				   << (c.coord.get_lat()) << "\t"
				   << (c.coord.get_lon()) << "\t"
				   << (c.coord.get_alt());
	}
};

/**
 * @brief The waypoint struct Wraps command for MAVLink waypoint.
 */
struct waypoint {
	/**
	 * @brief index Index
	 * @brief current_wp Current waypoint (boolean)
	 * @brief coord_frame Coordinate frame (16)
	 */
	size_t index, current_wp, coord_frame;
	/** @brief my_cmd Command */
	command my_cmd;
	/** @brief autocontinue Autocontinue (boolean) */
	size_t autocontinue;
	/**
	 * @brief waypoint ctor
	 * @param index Index
	 * @param current_wp Current waypoint (boolean)
	 * @param coord_frame Coordinate frame (16)
	 * @param commd Command
	 * @param autocontinue Autocontinue (boolean)
	 */
	waypoint(size_t index, size_t current_wp, size_t coord_frame, command commd,
			 size_t autocontinue=1)
		:
		  index(index),current_wp(current_wp),coord_frame(coord_frame),
		  my_cmd(commd),autocontinue(autocontinue)
	{ }

	/**
	 * @brief waypoint ctor
	 * @param idx Index
	 * @param curwp Current waypoint (boolean)
	 * @param cframe Coordinate frame (16)
	 * @param cmd CMD_ID
	 * @param p1 Param 1
	 * @param p2 Param 2
	 * @param p3 Param 3
	 * @param p4 Param 4
	 * @param lat Latitude
	 * @param lon Longitude
	 * @param alt Altitude
	 * @param autocontinue Autocontinue (boolean)
	 */
	waypoint(size_t idx, size_t curwp, size_t cframe,
			 size_t cmd, double p1, double p2, double p3, double p4,
			 double lat, double lon, double alt,
			 size_t autocontinue=1)
		:
		  waypoint(idx,curwp,cframe,
				   command(cmd,p1,p2,p3,p4,
						   coordinate(lat,lon,alt)),
				   autocontinue)
	{ }

	/**
	 * @brief operator<< MAVLink command waypoint file format
	 * @param out Output stream
	 * @param w Waypoint
	 * @return out
	 */
	friend ostream& operator<<(ostream &out, const waypoint &w) {
		out.unsetf(ios_base::floatfield);
		out << w.index << "\t" << w.current_wp << "\t"
			<< w.coord_frame << "\t" << w.my_cmd << "\t";
		out.unsetf(ios_base::floatfield);
		return out << w.autocontinue;
	}
};

/**
 * @brief The autopath struct A vector of waypoints with autocontinue set
 */
struct autopath : public vector<waypoint> {
	/**
	 * @brief autopath ctor
	 * @param coords Coordinates
	 */
	autopath(const vector<coordinate> &coords) {
		size_t i=0;
		for(auto c:coords)
			this->emplace_back(i++,0,0,16,0,0,0,0,c.get_lat(),c.get_lon(),c.get_alt(),1);
		(this->front()).current_wp=1;
	}

	/**
	 * @brief operator<< Outputs all coordinates on individual lines
	 * @param out Output stream
	 * @param ap autopath
	 * @return out
	 */
	friend ostream& operator<<(ostream &out, const autopath &ap) {
		for(auto wp:ap)
			//out << wp << endl;
			out << wp << "\r\n"; //Apparantly, line ending format is extremely important.
		return out;
	}

	/**
	 * @brief save Saves autopath as waypoint text file
	 * @param filename Filename (.txt)
	 */
	void save(const string &filename) const {
		ofstream f(filename);
		if(!f.is_open())
			//cerr << "Waypoint file \"" << filename << "\" could not be created." << endl;
			throw(file_error(filename,false));
		//f << "QGC WPL 110" << endl;
		//f << (*this) << endl;
		f << "QGC WPL 110\r\n"; //Apparantly, line ending format is extremely important.
		f << (*this);
		f.close();
	}
};
/** @brief VecCoord vector of coordinates */
typedef vector<coordinate> VecCoord;
/** @brief VecWP vector of waypoints */
typedef vector<waypoint> VecWP;
} //namespace MAVLink

using namespace MAVLink;
/**
 * @brief The Kmlmanip class kml manipulation function wrapper
 */
class Kmlmanip {
protected:
	/**
	 * @brief tokenize Splits a string into a vector of tokens (substrings)
	 * @param str String to split
	 * @param delimiters Delimiters
	 * @return Vector of tokens
	 */
	static const vector<string> tokenize(const string& str, const string& delimiters=" \n\t") {
		vector<string> tokens;
		string::size_type lastPos = str.find_first_not_of(delimiters, 0);
		string::size_type pos     = str.find_first_of(delimiters, lastPos);
		while (string::npos != pos || string::npos != lastPos) {
			tokens.push_back(str.substr(lastPos, pos - lastPos));
			lastPos = str.find_first_not_of(delimiters, pos);
			pos = str.find_first_of(delimiters, lastPos);
		}
		return move(tokens);
	}

	/**
	 * @brief kml_extract_coordstrings Opens kml file and stores coordinate strings,
	 * @param filename kml filename,
	 * @return Vector of coordinate strings, eg. "45.67,56.55,550".
	 */
	static const vector<string> kml_extract_coordstrings(const string &filename) {
		ifstream kml(filename);
		if(!kml.is_open())
			//cerr << "Problem!!! Could not open file!" << endl;
			throw(file_error(filename,true));
		string line;
		string accum="";
		while(!kml.eof()) {
			getline(kml,line);
			if(line.find("<coordinates>")!=string::npos) {
				getline(kml,line);
				while(line.find('<')==string::npos) {
					accum.append(line.append({'\n'}));
					getline(kml,line);
				}
			}
		}
		kml.close();
		return move(tokenize(accum," \n\t"));
	}

public:
	/**
	 * @brief kml2csv_coords Converts kml file to csv of coordinates.
	 * @param kmlfile kml filename
	 */
	static void kml2csv_coords(const string &kmlfile) {
		vector<string> coordstring = kml_extract_coordstrings(kmlfile);
		string csvfilename = kmlfile.substr(0,kmlfile.find_last_of('.'))+".csv";
		ofstream csvfile(csvfilename,ofstream::out);
		if(!csvfile.is_open())
			//cerr << "Problem!!! Could not create file!" << endl;
			throw(file_error(kmlfile,false));
		for(auto c:coordstring)
			csvfile << c << endl;
		csvfile.close();
	}

	static const vector<coordinate> csv_extract_coordinates(const string &csvfile) {
		vector<coordinate> result;
		ifstream csv(csvfile);
		if(!csv.is_open())
			throw(file_error(csvfile,true));
		string line;
		while(!csv.eof()) {
			getline(csv,line);
			vector<string> tokens = tokenize(line,",");
			result.emplace_back(
						stod(tokens.at(0)),
						stod(tokens.at(1)),
						stod(tokens.at(2)));
		}
		csv.close();
		return move(result);
	}

	static const autopath csv_to_autopath(const string &csvfile) {
		return move(autopath(csv_extract_coordinates(csvfile)));
	}

	static void csv2wp(const string &csvfile) {
		string wpfilename = csvfile.substr(0,csvfile.find_last_of('.'))+".txt";
		autopath p = csv_to_autopath(csvfile);
		p.save(wpfilename);
	}

	/**
	 * @brief kml_extract_coordinates Opens kml file and extracts coordinates
	 * @param kmlfile kml filename
	 * @return Vector of coordinates in kml file
	 */
	static const vector<coordinate> kml_extract_coordinates(const string &kmlfile) {
		vector<string> coordstr = kml_extract_coordstrings(kmlfile);
		vector<coordinate> result;
		for(auto cs:coordstr) {
			vector<string> tokens = tokenize(cs,",");
			result.emplace_back( //--------------------WEIRD GOOGLE FORMAT! LONGITUDE FIRST???---------//
						stod(tokens.at(1)), //lat
						stod(tokens.at(0)), //lon
						stod(tokens.at(2))); //alt
		}
		return move(result);
	}

	/**
	 * @brief kml_to_autopath Generates autopath from kml file
	 * @param kmlfile kml filename
	 * @return autopath of waypoints generated from coordinates in kmlfile
	 */
	static const autopath kml_to_autopath(const string &kmlfile) {
		return move(autopath(kml_extract_coordinates(kmlfile)));
	}

	/**
	 * @brief kml2wp Converts kml file to autopath waypoint text file
	 * @param kmlfile kml filename
	 */
	static void kml2wp(const string &kmlfile) {
		string wpfilename = kmlfile.substr(0,kmlfile.find_last_of('.'))+".txt";
		autopath p = kml_to_autopath(kmlfile);
		p.save(wpfilename);
	}
};

