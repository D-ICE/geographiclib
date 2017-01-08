/**
 * \file ConicProj.cpp
 * \brief Command line utility for conical projections
 *
 * Copyright (c) Charles Karney (2009-2016) <charles@karney.com> and licensed
 * under the MIT/X11 License.  For more information, see
 * http://geographiclib.sourceforge.net/
 *
 * See the <a href="ConicProj.1.html">man page</a> for usage information.
 **********************************************************************/

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <GeographicLib/LambertConformalConic.hpp>
#include <GeographicLib/AlbersEqualArea.hpp>
#include <GeographicLib/DMS.hpp>
#include <GeographicLib/Utility.hpp>

#if defined(_MSC_VER)
// Squelch warnings about constant conditional expressions and potentially
// uninitialized local variables
#  pragma warning (disable: 4127 4701)
#endif

#include "ConicProj.usage"

int main(int argc, const char* const argv[]) {
  try {
    using namespace GeographicLib;
    typedef Math::real real;
    Utility::set_digits();
    bool lcc = false, albers = false, reverse = false, longfirst = false;
    real lat1 = 0, lat2 = 0, lon0 = 0, k1 = 1;
    real
      a = Constants::WGS84_a(),
      f = Constants::WGS84_f();
    int prec = 6;
    std::string istring, ifile, ofile, cdelim;
    char lsep = ';';

    for (int m = 1; m < argc; ++m) {
      std::string arg(argv[m]);
      if (arg == "-r")
        reverse = true;
      else if (arg == "-c" || arg == "-a") {
        lcc = arg == "-c";
        albers = arg == "-a";
        if (m + 2 >= argc) return usage(1, true);
        try {
          for (int i = 0; i < 2; ++i) {
            DMS::flag ind;
            (i ? lat2 : lat1) = DMS::Decode(std::string(argv[++m]), ind);
            if (ind == DMS::LONGITUDE)
              throw GeographicErr("Bad hemisphere");
          }
        }
        catch (const std::exception& e) {
          std::cerr << "Error decoding arguments of " << arg << ": "
                    << e.what() << "\n";
          return 1;
        }
      } else if (arg == "-l") {
        if (++m == argc) return usage(1, true);
        try {
          DMS::flag ind;
          lon0 = DMS::Decode(std::string(argv[m]), ind);
          if (ind == DMS::LATITUDE)
            throw GeographicErr("Bad hemisphere");
          lon0 = Math::AngNormalize(lon0);
        }
        catch (const std::exception& e) {
          std::cerr << "Error decoding argument of " << arg << ": "
                    << e.what() << "\n";
          return 1;
        }
      } else if (arg == "-k") {
        if (++m == argc) return usage(1, true);
        try {
          k1 = Utility::val<real>(std::string(argv[m]));
        }
        catch (const std::exception& e) {
          std::cerr << "Error decoding argument of " << arg << ": "
                    << e.what() << "\n";
          return 1;
        }
      } else if (arg == "-e") {
        if (m + 2 >= argc) return usage(1, true);
        try {
          a = Utility::val<real>(std::string(argv[m + 1]));
          f = Utility::fract<real>(std::string(argv[m + 2]));
        }
        catch (const std::exception& e) {
          std::cerr << "Error decoding arguments of -e: " << e.what() << "\n";
          return 1;
        }
        m += 2;
      } else if (arg == "-w")
        longfirst = !longfirst;
      else if (arg == "-p") {
        if (++m == argc) return usage(1, true);
        try {
          prec = Utility::val<int>(std::string(argv[m]));
        }
        catch (const std::exception&) {
          std::cerr << "Precision " << argv[m] << " is not a number\n";
          return 1;
        }
      } else if (arg == "--input-string") {
        if (++m == argc) return usage(1, true);
        istring = argv[m];
      } else if (arg == "--input-file") {
        if (++m == argc) return usage(1, true);
        ifile = argv[m];
      } else if (arg == "--output-file") {
        if (++m == argc) return usage(1, true);
        ofile = argv[m];
      } else if (arg == "--line-separator") {
        if (++m == argc) return usage(1, true);
        if (std::string(argv[m]).size() != 1) {
          std::cerr << "Line separator must be a single character\n";
          return 1;
        }
        lsep = argv[m][0];
      } else if (arg == "--comment-delimiter") {
        if (++m == argc) return usage(1, true);
        cdelim = argv[m];
      } else if (arg == "--version") {
        std::cout << argv[0] << ": GeographicLib version "
                  << GEOGRAPHICLIB_VERSION_STRING << "\n";
        return 0;
      } else
        return usage(!(arg == "-h" || arg == "--help"), arg != "--help");
    }

    if (!ifile.empty() && !istring.empty()) {
      std::cerr << "Cannot specify --input-string and --input-file together\n";
      return 1;
    }
    if (ifile == "-") ifile.clear();
    std::ifstream infile;
    std::istringstream instring;
    if (!ifile.empty()) {
      infile.open(ifile.c_str());
      if (!infile.is_open()) {
        std::cerr << "Cannot open " << ifile << " for reading\n";
        return 1;
      }
    } else if (!istring.empty()) {
      std::string::size_type m = 0;
      while (true) {
        m = istring.find(lsep, m);
        if (m == std::string::npos)
          break;
        istring[m] = '\n';
      }
      instring.str(istring);
    }
    std::istream* input = !ifile.empty() ? &infile :
      (!istring.empty() ? &instring : &std::cin);

    std::ofstream outfile;
    if (ofile == "-") ofile.clear();
    if (!ofile.empty()) {
      outfile.open(ofile.c_str());
      if (!outfile.is_open()) {
        std::cerr << "Cannot open " << ofile << " for writing\n";
        return 1;
      }
    }
    std::ostream* output = !ofile.empty() ? &outfile : &std::cout;

    if (!(lcc || albers)) {
      std::cerr << "Must specify \"-c lat1 lat2\" or "
                << "\"-a lat1 lat2\"\n";
      return 1;
    }

    const LambertConformalConic lproj =
      lcc ? LambertConformalConic(a, f, lat1, lat2, k1)
      : LambertConformalConic(1, 0, 0, 0, 1);
    const AlbersEqualArea aproj =
      albers ? AlbersEqualArea(a, f, lat1, lat2, k1)
      : AlbersEqualArea(1, 0, 0, 0, 1);

    // Max precision = 10: 0.1 nm in distance, 10^-15 deg (= 0.11 nm),
    // 10^-11 sec (= 0.3 nm).
    prec = std::min(10 + Math::extra_digits(), std::max(0, prec));
    std::string s, eol, stra, strb, strc;
    std::istringstream str;
    int retval = 0;
    while (std::getline(*input, s)) {
      try {
        eol = "\n";
        if (!cdelim.empty()) {
          std::string::size_type m = s.find(cdelim);
          if (m != std::string::npos) {
            eol = " " + s.substr(m) + "\n";
            s = s.substr(0, m);
          }
        }
        str.clear(); str.str(s);
        real lat, lon, x, y, gamma, k;
        if (!(str >> stra >> strb))
          throw GeographicErr("Incomplete input: " + s);
        if (reverse) {
          x = Utility::val<real>(stra);
          y = Utility::val<real>(strb);
        } else
          DMS::DecodeLatLon(stra, strb, lat, lon, longfirst);
        if (str >> strc)
          throw GeographicErr("Extraneous input: " + strc);
        if (reverse) {
          if (lcc)
            lproj.Reverse(lon0, x, y, lat, lon, gamma, k);
          else
            aproj.Reverse(lon0, x, y, lat, lon, gamma, k);
          *output << Utility::str(longfirst ? lon : lat, prec + 5) << " "
                  << Utility::str(longfirst ? lat : lon, prec + 5) << " "
                  << Utility::str(gamma, prec + 6) << " "
                  << Utility::str(k, prec + 6) << eol;
        } else {
          if (lcc)
            lproj.Forward(lon0, lat, lon, x, y, gamma, k);
          else
            aproj.Forward(lon0, lat, lon, x, y, gamma, k);
          *output << Utility::str(x, prec) << " "
                  << Utility::str(y, prec) << " "
                  << Utility::str(gamma, prec + 6) << " "
                  << Utility::str(k, prec + 6) << eol;
        }
      }
      catch (const std::exception& e) {
        *output << "ERROR: " << e.what() << "\n";
        retval = 1;
      }
    }
    return retval;
  }
  catch (const std::exception& e) {
    std::cerr << "Caught exception: " << e.what() << "\n";
    return 1;
  }
  catch (...) {
    std::cerr << "Caught unknown exception\n";
    return 1;
  }
}
