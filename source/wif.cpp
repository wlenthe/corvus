#include "wif.h"

#include <sstream>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <limits>

using namespace corvus;

template <typename T> void sortAndDupCheck(std::vector< std::pair<Wif::Integer, T> >& v, std::string const& name) {
	std::sort(v.begin(), v.end());
	v.erase(std::unique(v.begin(), v.end()), v.end()); // it is ok if we have double entries as long as they are the same
	for (size_t i = 0; i < v.size(); i++) {
		if (i > 0 && v[i-1].first == v[i].first) {
			throw std::invalid_argument(name + " has multiple entries for number " + std::to_string(v[i].first)); // we already got rid of duplicates, these conflict
		}
	}
}

//! get the iterator to an item in a sorted list of pairs of <int, T>
//! \param v vector of pairs to search
//! \param i index to search for
//! \return iterator such that iter->first == i or iter == v.cend()
template <typename T> typename std::vector< std::pair<Wif::Integer, T> >::const_iterator findIndex(std::vector< std::pair<Wif::Integer, T> > const& v, Wif::Integer i) {
	return std::lower_bound(v.cbegin(), v.cend(), i, [](std::pair<Wif::Integer, T> const& p, Wif::Integer const& rhs) {
		return p.first < rhs; // only compare the key, we don't care about the value
	});
}

//! check if an index exists in a sorted list of pairs of <int,T>
//! \param v vector of pairs to search
//! \param i index to search for
//! \return true if the index is found, false otherwise
template <typename T> bool searchIndex(std::vector< std::pair<Wif::Integer, T> > const& v, Wif::Integer i) {
	auto iter = findIndex(v, i);
	return (v.cend() != iter && iter->first == i);
}

//! check if an index exists in a sorted list of pairs of <int,T> and get the value
//! \param v vector of pairs to search
//! \param i index to search for
//! \param t location to write value if found
//! \return true if the index is found, false otherwise
template <typename T> bool searchIndex(std::vector< std::pair<Wif::Integer, T> > const& v, Wif::Integer i, T& t) {
	auto iter = findIndex(v, i);
	if (v.cend() != iter && iter->first == i) {
		t = iter->second;
		return true;
	} else {
		return false;
	}
}

//! densify a list of <int, vec<int>>
//! \param v list to densify, i.e. replace implied <i, {0}> with actual entries
//! \param n target number
void densifyList(typename std::vector< std::pair<Wif::Integer, Wif::VecInt > >& v, Wif::Integer n) {
	for (std::pair<Wif::Integer, Wif::VecInt >& p : v) std::sort(p.second.begin(), p.second.end()); // also sort the vecint while we're at it

	typename std::vector<std::pair<Wif::Integer, Wif::VecInt> > skipped;
	skipped.reserve(n - v.size());
	Wif::Integer curIdx = 1;
	for (Wif::Integer i = 1; i <= n; i++) {
		if (curIdx-1 >= v.size()) { // we've reached the end of our list
			skipped.emplace_back(curIdx, Wif::VecInt{0}); // i.e. advance i but leave the current thread unchanged
		} else if (v[curIdx-1].first == i) {
			// we have this thread in the list already, advance to next one
			++curIdx; // i.e. advance current thread and i together
		} else {
			// we don't have it, since the list has been sorted that means v[curIdx].first > i, leave it unchanged
			skipped.emplace_back(curIdx, Wif::VecInt{0}); // i.e. advance i but leave the current thread unchanged
		}
	}

	// now that we have the list of skipped threads add them all in
	auto iter = v.insert(v.end(), skipped.cbegin(), skipped.cend()); // split over 2 lines since c.begin may be invalidated
	std::inplace_merge(v.begin(), iter, v.end());
}

void Wif::sanityCheck() {
	// sort all of our lists of {thread id, value} and check for duplicates
	sortAndDupCheck(notes                , "notes"              );
	sortAndDupCheck(tieUp                , "tieup"              );
	sortAndDupCheck(colorTable           , "color table"        );
	sortAndDupCheck(warpSymbolTable      , "warp symbol table"  );
	sortAndDupCheck(weftSymbolTable      , "weft symbol table"  );
	sortAndDupCheck(threading            , "threading"          );
	sortAndDupCheck(warpThicknessList    , "warp thickness"     );
	sortAndDupCheck(warpThicknessZoomList, "warp thickness zoom");
	sortAndDupCheck(warpSpacingList      , "warp spacing"       );
	sortAndDupCheck(warpSpacingZoomList  , "warp spacing zoom"  );
	sortAndDupCheck(warpColorList        , "warp colors"        );
	sortAndDupCheck(warpSymbolList       , "warp symbols"       );
	sortAndDupCheck(treadling            , "treadling"          );
	sortAndDupCheck(liftPlan             , "liftplan"           );
	sortAndDupCheck(weftThicknessList    , "weft thickness"     );
	sortAndDupCheck(weftThicknessZoomList, "weft thickness zoom");
	sortAndDupCheck(weftSpacingList      , "weft spacing"       );
	sortAndDupCheck(weftSpacingZoomList  , "weft spacing zoom"  );
	sortAndDupCheck(weftColorList        , "weft colors"        );
	sortAndDupCheck(weftSymbolList       , "weft symbols"       );

	// next make sure none of our lists are longer than they should be
	if (warpSymbolTable      .size() > warpThreads) throw std::invalid_argument("warp symbol table is longer than warp threads");
	if (threading            .size() > warpThreads) throw std::invalid_argument("threading is longer than warp threads");
	if (warpThicknessList    .size() > warpThreads) throw std::invalid_argument("warp thickness is longer than warp threads");
	if (warpThicknessZoomList.size() > warpThreads) throw std::invalid_argument("warp thickness zoom is longer than warp threads");
	if (warpSpacingList      .size() > warpThreads) throw std::invalid_argument("warp spacing is longer than warp threads");
	if (warpSpacingZoomList  .size() > warpThreads) throw std::invalid_argument("warp spacing zoom is longer than warp threads");
	if (warpColorList        .size() > warpThreads) throw std::invalid_argument("warp colors is longer than warp threads");
	if (warpSymbolList       .size() > warpThreads) throw std::invalid_argument("warp symbols is longer than warp threads");
	if (weftSymbolTable      .size() > weftThreads) throw std::invalid_argument("weft symbol table is longer than weft threads");
	if (treadling            .size() > weftThreads) throw std::invalid_argument("treadling is longer than weft threads");
	if (liftPlan             .size() > weftThreads) throw std::invalid_argument("lift plan is longer than weft threads");
	if (weftThicknessList    .size() > weftThreads) throw std::invalid_argument("weft thickness is longer than weft threads");
	if (weftThicknessZoomList.size() > weftThreads) throw std::invalid_argument("weft thickness zoom is longer than weft threads");
	if (weftSpacingList      .size() > weftThreads) throw std::invalid_argument("weft spacing is longer than weft threads");
	if (weftSpacingZoomList  .size() > weftThreads) throw std::invalid_argument("weft spacing zoom is longer than weft threads");
	if (weftColorList        .size() > weftThreads) throw std::invalid_argument("weft colors is longer than weft threads");
	if (weftSymbolList       .size() > weftThreads) throw std::invalid_argument("weft symbols is longer than weft threads");

	// check the color table doesn't have values outside of the color range
	{
		Integer cMin = std::numeric_limits<Integer>::max(), cMax = 0;
		for (size_t i = 0; i < colorTable.size(); i++) {
			// get min/max color
			cMin = std::min( std::min(cMin, colorTable[i].second[0]), std::min(colorTable[i].second[1], colorTable[i].second[2]) );
			cMax = std::max( std::max(cMax, colorTable[i].second[0]), std::max(colorTable[i].second[1], colorTable[i].second[2]) );
		}

		if (cMin < range.first || cMax > range.second) {
			throw std::invalid_argument("color table has at least one value outside of range [" + std::to_string(range.first) + ", " + std::to_string(range.second) + "]");
		}
	}

	// we we have a shaft count we must also have a treadles count
	if ((0 != shafts) != (0 != treadles)) {
		throw std::invalid_argument("shaft / treadle count must either both be positive or both be 0");
	}

	// there is a bunch of stuff that needs to be consistent across the warp information
	if (warpThreads != 0) {
		// we have warps, the numbering better be consistent
		if (warpColorIndex > 0) {
			Color c;
			if (!searchIndex(colorTable, warpColorIndex, c)) throw std::invalid_argument("warp color index not in warp colors");
			if (Color{0,0,0} == warpColorValue) warpColorValue = c;
			else if (c != warpColorValue) throw std::invalid_argument("warp color value doesn't match color table");
		}
		if (warpSymbolNum > 0) {
			Symbol s;
			if (!searchIndex(warpSymbolTable, warpSymbolNum , s)) throw std::invalid_argument("warp symbol index not in warp symbol table");
			if (0 == warpSymbol) warpSymbol = s;
			else if (s != warpSymbol) throw std::invalid_argument("warp symbol value doesn't match symbol table");

		}
		if (Unit::None == warpUnit) throw std::invalid_argument("warp units is required");
		if (warpSpacing   < 0     ) throw std::invalid_argument("warp spacing must be >= 0");
		if (warpThickness < 0     ) throw std::invalid_argument("warp thickness must be >= 0");
		for (auto const& p : warpSymbolTable      ) if (0 == p.first || p.first > warpThreads) throw std::invalid_argument("warp symbol table has warp index outside of warp thread count");
		for (auto const& p : threading            ) if (0 == p.first || p.first > warpThreads || p.second.empty()) throw std::invalid_argument("threading has warp index outside of warp thread count or empty list (please use shaft 0 for unused warps)"); // spec says use 0 not an empty list
		for (auto const& p : warpThicknessList    ) if (0 == p.first || p.first > warpThreads || p.second <  0) throw std::invalid_argument("warp thickness has warp index outside of warp thread count or thickness < 0");
		for (auto const& p : warpThicknessZoomList) if (0 == p.first || p.first > warpThreads || p.second == 0) throw std::invalid_argument("warp zoom has warp index outside of warp thread count or zoom = 0");
		for (auto const& p : warpSpacingList      ) if (0 == p.first || p.first > warpThreads || p.second <  0) throw std::invalid_argument("warp spacing has warp index outside of warp thread count or spacing < 0");
		for (auto const& p : warpSpacingZoomList  ) if (0 == p.first || p.first > warpThreads || p.second == 0) throw std::invalid_argument("warp spacing zoom has warp index outside of warp thread count or zoom = 0");
		for (auto const& p : warpColorList        ) if (0 == p.first || p.first > warpThreads || !searchIndex(colorTable     , p.second) ) throw std::invalid_argument("warp colors has warp index outside of warp thread count or index not in color table");
		for (auto const& p : warpSymbolList       ) if (0 == p.first || p.first > warpThreads || !searchIndex(warpSymbolTable, p.second) ) throw std::invalid_argument("warp symbols has warp index outside of warp thread count or index not in symbol table");
	} else {
		// we don't have warps, we'd better not have anything else
		if (0 != warpColorIndex           ) throw std::invalid_argument("cannot have warp color index without warp threads");
		if (0 != warpSymbol               ) throw std::invalid_argument("cannot have warp symbol without warp threads");
		if (0 != warpSymbolNum            ) throw std::invalid_argument("cannot have warp symbol number without warp threads");
		if (Unit::None != warpUnit        ) throw std::invalid_argument("cannot have warp unit without warp threads");
		if (!isnan(warpSpacing  )         ) throw std::invalid_argument("cannot have warp spacing without warp threads");
		if (!isnan(warpThickness)         ) throw std::invalid_argument("cannot have warp spacing without warp threads");
		if (1 != warpSpacingZoom          ) throw std::invalid_argument("cannot have warp spacing zoom without warp threads");
		if (1 != warpThicknessZoom        ) throw std::invalid_argument("cannot have warp thickness zoom without warp threads");
		// already handled empty lists with our size comparison check
	}

	// and the same for weft information
	if (weftThreads != 0) {
		// we have wefts, the numbering better be consistent
		if (weftColorIndex > 0) {
			Color c;
			if (!searchIndex(colorTable , weftColorIndex, c)) throw std::invalid_argument("weft color index not in weft colors");
			if (Color{0,0,0} == weftColorValue) weftColorValue = c;
			else if (c != weftColorValue) throw std::invalid_argument("weft color value doesn't match color table");
		}
		if (weftSymbolNum > 0) {
			Symbol s;
			if (!searchIndex(weftSymbolTable, weftSymbolNum , s)) throw std::invalid_argument("weft symbol index not in weft symbol table");
			if (0 == weftSymbol) weftSymbol = s;
			else if (s != weftSymbol) throw std::invalid_argument("weft symbol value doesn't match symbol table");

		}
		if (Unit::None == weftUnit) throw std::invalid_argument("weft units is required");
		if (weftSpacing   < 0     ) throw std::invalid_argument("weft spacing must be >= 0");
		if (weftThickness < 0     ) throw std::invalid_argument("weft thickness must be >= 0");
		for (auto const& p : weftSymbolTable      ) if (0 == p.first || p.first > weftThreads) throw std::invalid_argument("weft symbol table has weft index outside of weft thread count");
		for (auto const& p : treadling            ) if (0 == p.first || p.first > weftThreads) throw std::invalid_argument("treadling has weft index outside of weft thread count");
		for (auto const& p : liftPlan             ) if (0 == p.first || p.first > weftThreads) throw std::invalid_argument("liftPlan has weft index outside of weft thread count");
		for (auto const& p : weftThicknessList    ) if (0 == p.first || p.first > weftThreads || p.second <  0) throw std::invalid_argument("weft thickness has weft index outside of weft thread count or thickness < 0");
		for (auto const& p : weftThicknessZoomList) if (0 == p.first || p.first > weftThreads || p.second == 0) throw std::invalid_argument("weft zoom has weft index outside of weft thread count or zoom = 0");
		for (auto const& p : weftSpacingList      ) if (0 == p.first || p.first > weftThreads || p.second <  0) throw std::invalid_argument("weft spacing has weft index outside of weft thread count or spacing < 0");
		for (auto const& p : weftSpacingZoomList  ) if (0 == p.first || p.first > weftThreads || p.second == 0) throw std::invalid_argument("weft spacing zoom has weft index outside of weft thread count or zoom = 0");
		for (auto const& p : weftColorList        ) if (0 == p.first || p.first > weftThreads || !searchIndex(colorTable     , p.second) ) throw std::invalid_argument("weft colors has weft index outside of weft thread count or index not in color table");
		for (auto const& p : weftSymbolList       ) if (0 == p.first || p.first > weftThreads || !searchIndex(weftSymbolTable, p.second) ) throw std::invalid_argument("weft symbols has weft index outside of weft thread count or index not in symbol table");
	} else {
		// we don't have wefts, we'd better not have anything else
		if (0 != weftColorIndex           ) throw std::invalid_argument("cannot have weft color index without weft threads");
		if (0 != weftSymbol               ) throw std::invalid_argument("cannot have weft symbol without weft threads");
		if (0 != weftSymbolNum            ) throw std::invalid_argument("cannot have weft symbol number without weft threads");
		if (Unit::None != weftUnit        ) throw std::invalid_argument("cannot have weft unit without weft threads");
		if (!isnan(weftSpacing  )         ) throw std::invalid_argument("cannot have weft spacing without weft threads");
		if (!isnan(weftThickness)         ) throw std::invalid_argument("cannot have weft spacing without weft threads");
		if (1 != weftSpacingZoom          ) throw std::invalid_argument("cannot have weft spacing zoom without weft threads");
		if (1 != weftThicknessZoom        ) throw std::invalid_argument("cannot have weft thickness zoom without weft threads");
		// already handled empty lists with our size comparison check
	}

	// if we made it to here all the individual lists look good and all the indexing into the color/symbol tables is ok
	// now we need to check the layout itself (threading, treadling, and lift plan) a little more carefully
	// it is also helpful to 'densify' the structure (i.e. populate all the optional/implied fields)
	
	// start by densifying
	densifyList(threading, warpThreads);
	densifyList(tieUp    , treadles   );
	densifyList(treadling, weftThreads);

	// now sanity check
	for (auto const& p : threading) {
		if (0 == p.second.front() && 1 != p.second.size()) throw std::invalid_argument("warp cannot be threaded through null shaft 0 and an actual shaft");
		if (p.second.back() > shafts) throw std::invalid_argument("threading uses shaft number greater than shaft count");
	}

	for (auto const& p : tieUp) {
		if (0 == p.second.front() && 1 != p.second.size()) throw std::invalid_argument("treadle cannot be tied up to null shaft 0 and an actual shaft");
		if (p.second.back() > shafts) throw std::invalid_argument("tie up uses shaft number greater than shaft count");
	}

	for (auto const& p : treadling) {
		if (0 == p.second.front() && 1 != p.second.size()) throw std::invalid_argument("treadling cannot press null and treadle 0 and an actual treadle");
		if (p.second.back() > treadles) throw std::invalid_argument("treadling uses treadle number greater than treadle count");
	}

	// finally build / check liftplan
	std::vector< std::pair<Integer, VecInt> > reconLift;
	reconLift.reserve(warpThreads);
	for (auto const& p : treadling) {
		if (0 == p.second.front()) {
			reconLift.push_back(p); // nothing is lifted
		} else {
			// we could do a bunch of merge sorts here but it just isn't worth the trouble
			VecInt shaftsUp;
			for (Integer const& t : p.second) { // loop over treadles pressed down
				VecInt const& s = tieUp[t-1].second; // the shafts lifted by this treadle
				if (0 == s.front()) {
					// for some reason we are pressing down a treadle that doesn't do anything
				} else {
					shaftsUp.insert(shaftsUp.end(), s.cbegin(), s.cend());
				}
			}
			if (shaftsUp.empty()) shaftsUp.push_back(0);

			// instead just sort when we're done and remove duplicates
			std::sort(shaftsUp.begin(), shaftsUp.end());
			shaftsUp.erase( std::unique(shaftsUp.begin(), shaftsUp.end()), shaftsUp.end());

			reconLift.emplace_back(p.first, shaftsUp);
		}
	}

	if (!liftPlan.empty()) {
		densifyList(liftPlan , weftThreads);
		if (liftPlan != reconLift) throw std::invalid_argument("lift plan doesn't match plan generated from treadling and tie up");
	} else {
		liftPlan.swap(reconLift);
	}

}

namespace wif_io {
	typename Wif::VecInt  parse_vint(std::string const& str);

	std::string trimWs(std::string const& str) {
		size_t idx = str.find_first_not_of(" \t");
		if (std::string::npos == idx) throw std::invalid_argument("empty string cannot be parsed into WIF value");
		std::string cpy = str.substr(idx);
		while (isspace(cpy.back())) cpy.pop_back();
		return cpy;
	}

	typename Wif::Real    parse_real(std::string const& str) {
		static_assert(std::is_same<double, Wif::Real>::value, "wif parser assumes real is double");
		return atof(trimWs(str).c_str());
	}

	typename Wif::String  parse_str (std::string const& str) {
		std::string cpy(str);
		size_t idx = 0;
		while( std::string::npos != ( idx = cpy.find("//", idx) ) ) cpy.replace(idx, 2, "\n");
		return cpy;
	}

	void put_str(std::ostream& os, Wif::String const& str) {
		for (char const& c : str) {
			if ('\n' == c) os << "//";
			else os << c;
		}
	}

	typename Wif::Integer parse_int (std::string const& str) {
		int i = atoi(trimWs(str).c_str());
		if (i < 0 || unsigned int(i) > std::numeric_limits<Wif::Integer>::max()) throw std::invalid_argument(std::to_string(i) + " is outside representable range for WIF integer");
		return static_cast<Wif::Integer>(i);
	}

	typename Wif::Boolean parse_bool(std::string const& str) {
		std::string cpy = trimWs(str);
		if      ("true"  == cpy || "on"  == cpy || "yes" == cpy || "1" == cpy) return true;
		else if ("false" == cpy || "off" == cpy || "no"  == cpy || "0" == cpy) return false;
		else throw std::invalid_argument("string \"" + str + "\" cannot be parsed into WIF boolean");
	}

	void put_bool(std::ostream& os, Wif::Boolean const& b) {
		os << (b ? "true" : "false");
	}

	typename Wif::Color     parse_rgb (std::string const& str) {
		Wif::VecInt v = parse_vint(str);
		if (3 != v.size()) throw std::invalid_argument("\"" + str + "\" is invalid WIF color, expected 3 values but got " + std::to_string(v.size()));
		return {v[0], v[1], v[2]};
	}

	void put_rgb(std::ostream& os, Wif::Color const& c) {
		os << c[0] << ',' << c[1] << ',' << c[2];
	}

	typename Wif::Symbol  parse_symb(std::string const& str) {
		char c = 0;
		std::string cpy = trimWs(str);
		if ('#' == cpy.front()) {
			// we have a number
			if (1 == cpy.size()) throw std::invalid_argument("\"" + str + "\" is invaled WIF symbol (should be '#' or #___)");
			cpy = cpy.substr(1);
			for (char const& c : cpy) if (!isdigit(c)) throw std::invalid_argument("\"" + str + "\" is invaled WIF symbol (# must be followed by numbers)");
			int i = atoi(cpy.c_str());
			if (i > 255) throw std::invalid_argument("\"" + str + "\" is invaled WIF symbol (number following # must be [0,255])");
			c = static_cast<char>(i); // signed vs unsigned shouldn't matter much here since 
			// just to be safe lets throw an error the first time we encounter something and look at the file by hand
			if (i > 127) throw std::invalid_argument("\"" + str + "\" is invalid WIF symbol, extended ascii codes (greater than 127) are not currently supported, please contact the developers");
		} else if (1 == cpy.size()) {
			c = cpy.front();
		} else if (3 == cpy.size() && '\'' == cpy.front() && '\'' == cpy.back()) {
			c = cpy[1];
		}
		if (iscntrl(c)) throw std::invalid_argument("\"" + str + "\" is invaled WIF symbol");
		return c;
	}

	void put_symb(std::ostream& os, Wif::Symbol const& symb) {
		if (isprint(symb)) os << '\'' << symb << '\'';
		else os << '#' << int(symb);
	}

	typename Wif::VecInt  parse_vint(std::string const& str) {
		Wif::VecInt v;
		std::string tok;
		std::istringstream ss(str);
		while (std::getline(ss, tok, ',')) v.push_back(parse_int(tok));
		return v;
	}

	void put_vint(std::ostream& os, Wif::VecInt const& v) {
		if (!v.empty()) {
			os << v.front();
			for (size_t i = 1; i < v.size(); i++) os << ',' << v[i];
		}
	}

	typename Wif::Range   parse_rng (std::string const& str) {
		Wif::VecInt v = parse_vint(str);
		if (2 != v.size()) throw std::invalid_argument("\"" + str + "\" is invalid WIF range, expected 2 values but got " + std::to_string(v.size()));
		if (v.back() <= v.front()) throw std::invalid_argument("\"" + str + "\" is invalid WIF range, second value must be >= first");
		return {v.front(), v.back()};
	}

	void put_rng(std::ostream& os, Wif::Range const& r) {
		os << r.first << ',' << r.second;
	}

	Wif::Unit    parse_unit(std::string const& str) {
		std::string cpy = trimWs(str);
		for (char& c : cpy) c = tolower(c);
		if      ("decipoints"  == cpy) return Wif::Unit::Decipoints ;
		else if ("inches"      == cpy) return Wif::Unit::Inches     ;
		else if ("centimeters" == cpy) return Wif::Unit::Centimeters;
		throw std::invalid_argument("WIF units must be 'decipoints', 'inches', or 'centimeters' (got '" + str + "')");
	}

	void put_unit(std::ostream& os, Wif::Unit const& u) {
		switch (u) {
			case Wif::Unit::None       : throw std::invalid_argument("units not set");
			case Wif::Unit::Decipoints : os << "decipoints" ;
			case Wif::Unit::Inches     : os << "inches"     ;
			case Wif::Unit::Centimeters: os << "centimeters";
		}
	}

	template <typename T>
	void parse_vecSect(std::pair<std::string, std::unordered_map<std::string, std::string> > const& sectMap, std::vector< std::pair< Wif::Integer, T > >& vec, std::function<T(std::string)> parse) {
		// parse value
		for (std::pair<std::string, std::string> const& p : sectMap.second) {
			try {
				vec.emplace_back(parse_int(p.first), parse(p.second));
			} catch (std::exception& e) {
				throw std::invalid_argument("WIF section [" + sectMap.first + "] \"" + p.first + "=" + p.second + "\" contains invalid value or integer key that couldn't be parsed: " + e.what());
			}
		}

		// sort and check for duplicate keys (different strings that parse to the same value)
		std::sort(vec.begin(), vec.end());
		for (size_t i = 1; i < vec.size(); i++) { // we could use std::unique but it isn't worth the trouble
			if (vec[i-1].first == vec[i].first) throw std::invalid_argument("WIF section [" + sectMap.first + "] contains duplicate key \"" + std::to_string(vec[i].first) + "\"");
		}
	};
}

void Wif::read(std::istream& is) {
	clear();

	// first define a lambda for error generation
	size_t lineNum = 0; // current line in file
	std::string line; // contents of current line in file
	size_t sectLineNum = 0; // line that has the header of the current section we are in
	std::string curSection; // name of current section
	std::function<std::string()> lineDescr = [&]() {
		// most users probably won't have a text editor with line numbers so it is nice to give a little more context
		// the longest standard section name is only 21 characters "[WARP SYMBOL PALETTE]"
		// we should guard against generating a super long error string in case of "[some unterminated line that goes on for a really long time"
		constexpr size_t maxSnip = 64; // what is the longest snippet to pull from the file
		std::ostringstream ss;
		ss << "line number " << lineNum << ' ';
		if (!curSection.empty()) {
			ss << (lineNum - sectLineNum) << " lines into [";
			// don't let our error message get so long it isn't useful
			// this cutoff is arbitrtary but should be longer than ever needed in practice
			if (curSection.size() < maxSnip) {
				ss << curSection << "] ";
			} else {
				ss << curSection.substr(0, maxSnip) << "...] ";
			}
		}
		if (!line.empty()) {
			if (line.size() < maxSnip) {
				ss << '"' << line << "\" ";
			} else {
				ss << "starting with \"" << line.substr(0, maxSnip) << "\"";
			}
		}
		return ss.str();
	};

	// loop over the file line by line
	std::unordered_map< std::string, std::unordered_map<std::string, std::string> > section_key_value;
	std::unordered_map< std::string, std::unordered_map<std::string, std::string> >::iterator sectionIter = section_key_value.end();
	while (std::getline(is, line)) {
		// making substring is somewhat wasteful and wrapping with an istringstream is overkill
		// as a compromise I'll just keep track of the current index we've made it to in the line
		size_t idx = 0;

		// skip comment and blank lines
		++lineNum;
		bool obsKey = false;
		if (line.empty()) continue;
		else if (';' == line.front()) {
			if (line.size() > 2 && '-' == line[1]) {
				// this is an obsolete keyline
				idx = 2;
				obsKey = true;
			} else {
				continue; // just a regular comment
			}
		}

		// skip any leading whitespace
		idx = line.find_first_not_of(" \t", idx); // skip leading white space (skipping ;- if needed)
		if (std::string::npos == idx) continue; // all whitespace

		// now we'd better have a key/value pair or a section header
		if ('[' == line.front()) { // section header
			const size_t idxClose = line.find(']', idx);
			if (std::string::npos == idxClose) throw std::invalid_argument(lineDescr() + "has [ without matching ]");

			// we found our closing ']' grab the section
			curSection = line.substr(idx+1, idxClose - idx - 1);
			if (curSection.empty()) throw std::invalid_argument(lineDescr() + "has empty section name");
			for (char& c : curSection) c = toupper(c);
			auto p = section_key_value.insert({curSection, std::unordered_map<std::string,std::string>()});
			if (!p.second) throw std::invalid_argument(lineDescr() + "is second instance of section");
			sectionIter = p.first;
			sectLineNum = lineNum;
			idx = idxClose + 1;
		} else { // keyline
			// make sure we have made it to our first section
			const size_t idxEq = line.find('=', idx);
			if (curSection.empty()) {
				// some ini formats support global keys but wif isn't one of them
				throw std::invalid_argument(lineDescr() + "isn't a [SECTION] or \";comment\" but appears before first section");
			}

			// make sure we have an equals sign
			if (std::string::npos == idxEq) throw std::invalid_argument(lineDescr() += "isn't a comment and doesn't contain a '='");

			// pull out key
			std::string key = line.substr(idx, idxEq - idx);
			if (key.empty()) throw std::invalid_argument(lineDescr() + "has empty key (no text before '=')");
			for (char& c : key) c = tolower(c);
			idx = idxEq + 1;
			if (idx >= line.size()) throw std::invalid_argument(lineDescr() + "has empty value (no text after '='");

			// save the value
			auto p = sectionIter->second.insert({key,line.substr(idx)});
			if (!p.second) throw std::invalid_argument(lineDescr() + "is second instance of key in section"); // you're supposed to allow this and just accept the ambiguity but that seems risky
			idx = std::string::npos;
		}

		// make sure there is no data after section header or key=value
		if (std::string::npos != idx) {
			if (idx < line.size()) idx = line.find_first_not_of(" \t", idx); // skip trailing whitespace
			if (idx < line.size()) { // we have something other than whitespace after the ']'
				if (';' == line[idx]) {
					// that something is an inline comment, ignore it
				} else {
					// that something is stray data
					throw std::invalid_argument(lineDescr() + "has trailing text that doesn't start with comment symbol ';'");
				}
			}
		}
	}

	// now that we have all our key values we can do the parsing, start with the WIF section
	std::string str;
	sectionIter = section_key_value.find("WIF");
	if (section_key_value.end() == sectionIter) throw std::invalid_argument("no [WIF] section found");
	std::unordered_map<std::string, std::string>::iterator keyIter;
	keyIter = sectionIter->second.find("version"       ); if (sectionIter->second.end() == keyIter) throw std::invalid_argument("no [WIF] version key found"          ); str        = keyIter->second;
	version = atof(str.c_str()); if (1.1 != version) throw std::invalid_argument("only WIF version 1.1 is supported");
	keyIter = sectionIter->second.find("date"          ); if (sectionIter->second.end() == keyIter) throw std::invalid_argument("no [WIF] date key found"             ); date       = keyIter->second;
	keyIter = sectionIter->second.find("developers"    ); if (sectionIter->second.end() == keyIter) throw std::invalid_argument("no [WIF] developers key found"       ); developers = keyIter->second;
	keyIter = sectionIter->second.find("source program"); if (sectionIter->second.end() == keyIter) throw std::invalid_argument("no [WIF] source program key found"   ); sourceProg = keyIter->second;
	keyIter = sectionIter->second.find("source version");                                                                                                               sourceVers = keyIter->second;

	// now the contents section
	sectionIter = section_key_value.find("CONTENTS");
	if (section_key_value.end() == sectionIter) throw std::invalid_argument("no [CONTENTS] section found");
	std::unordered_map<std::string, bool> contents;
	for (keyIter = sectionIter->second.begin(); keyIter != sectionIter->second.end(); ++keyIter) {
		// we converted all of our key names to lower case but section names to upper case
		// fix that now quick
		std::string sectName = keyIter->first;
		for (char& c : sectName) c = toupper(c);
		contents[sectName] = wif_io::parse_bool(keyIter->second);
	}

	// check for forbidden sections
	if (contents.end() != contents.find("BITMAP IMAGE")) throw std::invalid_argument("WIF contents lists [BITMAP IMAGE] which isn't implemented in the 1.1 standard");
	if (contents.end() != contents.find("BITMAP FILE" )) throw std::invalid_argument("WIF contents lists [BITMAP FILE] which isn't implemented in the 1.1 standard");
	if (section_key_value.end() != section_key_value.find("TRANSLATIONS")) throw std::invalid_argument("WIF has [TRANSLATIONS] which is suspended in the 1.1 standard");

	// finally loop over the actual data
	// we can do this in any order we want since we have everything pulled in
	// some orders are definately easier than others though
	// the standard recomments 4 passes:
	// 1. translations if they are ever implemented (this would actually be pretty easy for us, just an additional map layer for indirection)
	// 2. WIF (done), CONTENTS (done), COLOR PALETTE, WARP SYMBOL PALLETE, WEFT SYMBOL PALLETE
	// 3. TEXT, WEAVING, WARP, WEFT
	// 4. everything else
	// the logic for splitting 3 and 4 is to give the user an opportunity to down select which values to read
	// we've already read the whole thing so it doesn't matter
	// theorhetically reading the remaining ___ PALETTE type sections first would help us speed things up
	// we could call vector::reserve to pre allocate space
	// in practice it doesn't matter on modern computers given how small wif files are
	// I'm just going to loop in arbitrary order since ranged based for loops make the code more readable

	Integer colorEntries = 0, warpSymbolEntries = 0, wefSymbolEntries = 0;
	for (std::pair<std::string, std::unordered_map<std::string, std::string> > const& sectMap : section_key_value) {
		// check that our section is listed in the table of contents
		std::string const& sectName = sectMap.first;
		if ("WIF" == sectName || "CONTENTS" == sectName) continue; // we already took care of these
		std::unordered_map<std::string, bool>::iterator iter = contents.find(sectName);
		if (contents.end() == iter) throw std::invalid_argument("WIF contains section " + sectName + " that is not listed in contents");
		else if (!iter->second) throw std::invalid_argument("WIF contains section " + sectName + " that is explicitly excluded in contents");
		iter->second = false; // mark this section as visited

		// skip private sections
		if (sectName.size() > 8 && "PRIVATE " == sectName.substr(0, 8) ) continue;

		// now we'd better have a section that is listed in the standard
		if ("COLOR PALETTE"       == sectName) {
			colorEntries = -1;
			range.first = -1;
			for (std::pair<std::string, std::string> const& p : sectMap.second) {
				if      ("entries" == p.first) colorEntries = wif_io::parse_int(p.second);
				else if ("form"    == p.first) {} // deprecated, always RGB
				else if ("range"   == p.first) range        = wif_io::parse_rng(p.second);
				else throw std::invalid_argument("unexpected key '" + p.first + "' in [COLOR PALETTE]");
			}
			if (-1 == colorEntries) throw std::invalid_argument("[COLOR PALETTE] missing 'entries' key");
			if (-1 == range.first ) throw std::invalid_argument("[COLOR PALETTE] missing 'range' key");
		} else if ("WARP SYMBOL PALETTE" == sectName) {
			if (1 != sectMap.second.size() || "entries" != sectMap.second.begin()->first) throw std::invalid_argument("[WARP SYMBOL PALETTE] must have exactly 1 key 'entries'");
			warpSymbolEntries = wif_io::parse_int(sectMap.second.begin()->second);
		} else if ("WEFT SYMBOL PALETTE" == sectName) {
			if (1 != sectMap.second.size() || "entries" != sectMap.second.begin()->first) throw std::invalid_argument("[WEFT SYMBOL PALETTE] must have exactly 1 key 'entries'");
			wefSymbolEntries = wif_io::parse_int(sectMap.second.begin()->second);
		} else if ("TEXT"                == sectName) {
			for (std::pair<std::string, std::string> const& p : sectMap.second) {
				if      ("title"     == p.first) title     = wif_io::parse_str(p.second);
				else if ("author"    == p.first) author    = wif_io::parse_str(p.second);
				else if ("address"   == p.first) address   = wif_io::parse_str(p.second);
				else if ("email"     == p.first) email     = wif_io::parse_str(p.second);
				else if ("telephone" == p.first) telephone = wif_io::parse_str(p.second);
				else if ("fax"       == p.first) fax       = wif_io::parse_str(p.second);
				else throw std::invalid_argument("unexpected key '" + p.first + "' in [TEXT]");
			}
		} else if ("WEAVING"             == sectName) {
			shafts = treadles = -1;
			for (std::pair<std::string, std::string> const& p : sectMap.second) {
				if      ("shafts"      == p.first) shafts     = wif_io::parse_int (p.second);
				else if ("treadles"    == p.first) treadles   = wif_io::parse_int (p.second);
				else if ("rising shed" == p.first) risingShed = wif_io::parse_bool(p.second);
				else throw std::invalid_argument("unexpected key '" + p.first + "' in [WEAVING]");
			}
			if (-1 == shafts  ) throw std::invalid_argument("[WEAVING] missing 'shafts' key");
			if (-1 == treadles) throw std::invalid_argument("[WEAVING] missing 'treadles' key");
		} else if ("WARP"                == sectName) {
			VecInt color;
			for (std::pair<std::string, std::string> const& p : sectMap.second) {
				if      ("threads"        == p.first) warpThreads       = wif_io::parse_int (p.second);
				else if ("color"          == p.first) color             = wif_io::parse_vint(p.second);
				else if ("symbol"         == p.first) warpSymbol        = wif_io::parse_symb(p.second);
				else if ("symbol number"  == p.first) warpSymbolNum     = wif_io::parse_int (p.second);
				else if ("units"          == p.first) warpUnit          = wif_io::parse_unit(p.second);
				else if ("spacing"        == p.first) warpSpacing       = wif_io::parse_real(p.second);
				else if ("thickness"      == p.first) warpThickness     = wif_io::parse_real(p.second);
				else if ("spacing zoom"   == p.first) warpSpacingZoom   = wif_io::parse_int (p.second);
				else if ("thickness zoom" == p.first) warpThicknessZoom = wif_io::parse_int (p.second);
				else throw std::invalid_argument("unexpected key '" + p.first + "' in [WARP]");
			}
			if (color.empty()) {
				// no big deal
			} else if (1 == color.size()) {
				warpColorIndex = color.front(); // we got an index only
			} else if (4 == color.size()) {
				warpColorIndex = color.front(); // we got an index + RGB
				warpColorValue = {color[1], color[2], color[3]};
			} else {
				throw std::invalid_argument("[WARP] color must be either 1 or 4 values (got " + std::to_string(color.size()) + ")");
			}
		} else if ("WEFT"                == sectName) {
			VecInt color;
			for (std::pair<std::string, std::string> const& p : sectMap.second) {
				if      ("threads"        == p.first) weftThreads       = wif_io::parse_int (p.second);
				else if ("color"          == p.first) color             = wif_io::parse_vint(p.second);
				else if ("symbol"         == p.first) weftSymbol        = wif_io::parse_symb(p.second);
				else if ("symbol number"  == p.first) weftSymbolNum     = wif_io::parse_int (p.second);
				else if ("units"          == p.first) weftUnit          = wif_io::parse_unit(p.second);
				else if ("spacing"        == p.first) weftSpacing       = wif_io::parse_real(p.second);
				else if ("thickness"      == p.first) weftThickness     = wif_io::parse_real(p.second);
				else if ("spacing zoom"   == p.first) weftSpacingZoom   = wif_io::parse_int (p.second);
				else if ("thickness zoom" == p.first) weftThicknessZoom = wif_io::parse_int (p.second);
				else throw std::invalid_argument("unexpected key '" + p.first + "' in [WEFT]");
			}
			if (color.empty()) {
				// no big deal
			} else if (1 == color.size()) {
				weftColorIndex = color.front(); // we got an index only
			} else if (4 == color.size()) {
				weftColorIndex = color.front(); // we got an index + RGB
				weftColorValue = {color[1], color[2], color[3]};
			} else {
				throw std::invalid_argument("[WARP] color must be either 1 or 4 values (got " + std::to_string(color.size()) + ")");
			}
		} else if ("NOTES"               == sectName) { wif_io::parse_vecSect<String >(sectMap, notes                , wif_io::parse_str );
		} else if ("TIEUP"               == sectName) { wif_io::parse_vecSect<VecInt >(sectMap, tieUp                , wif_io::parse_vint);
		} else if ("COLOR TABLE"         == sectName) { wif_io::parse_vecSect<Color  >(sectMap, colorTable           , wif_io::parse_rgb );
		} else if ("WARP SYMBOL TABLE"   == sectName) { wif_io::parse_vecSect<Symbol >(sectMap, warpSymbolTable      , wif_io::parse_symb);
		} else if ("WEFT SYMBOL TABLE"   == sectName) { wif_io::parse_vecSect<Symbol >(sectMap, weftSymbolTable      , wif_io::parse_symb);
		} else if ("THREADING"           == sectName) { wif_io::parse_vecSect<VecInt >(sectMap, threading            , wif_io::parse_vint);
		} else if ("WARP THICKNESS"      == sectName) { wif_io::parse_vecSect<Real   >(sectMap, warpThicknessList    , wif_io::parse_real);
		} else if ("WARP THICKNESS ZOOM" == sectName) { wif_io::parse_vecSect<Integer>(sectMap, warpThicknessZoomList, wif_io::parse_int );
		} else if ("WARP SPACING"        == sectName) { wif_io::parse_vecSect<Real   >(sectMap, warpSpacingList      , wif_io::parse_real);
		} else if ("WARP SPACING ZOOM"   == sectName) { wif_io::parse_vecSect<Integer>(sectMap, warpSpacingZoomList  , wif_io::parse_int );
		} else if ("WARP COLORS"         == sectName) { wif_io::parse_vecSect<Integer>(sectMap, warpColorList        , wif_io::parse_int );
		} else if ("WARP SYMBOLS"        == sectName) { wif_io::parse_vecSect<Integer>(sectMap, warpSymbolList       , wif_io::parse_int );
		} else if ("TREADLING"           == sectName) { wif_io::parse_vecSect<VecInt >(sectMap, treadling            , wif_io::parse_vint);
		} else if ("LIFTPLAN"            == sectName) { wif_io::parse_vecSect<VecInt >(sectMap, liftPlan             , wif_io::parse_vint);
		} else if ("WEFT THICKNESS"      == sectName) { wif_io::parse_vecSect<Real   >(sectMap, weftThicknessList    , wif_io::parse_real);
		} else if ("WEFT THICKNESS ZOOM" == sectName) { wif_io::parse_vecSect<Integer>(sectMap, weftThicknessZoomList, wif_io::parse_int );
		} else if ("WEFT SPACING"        == sectName) { wif_io::parse_vecSect<Real   >(sectMap, weftSpacingList      , wif_io::parse_real);
		} else if ("WEFT SPACING ZOOM"   == sectName) { wif_io::parse_vecSect<Integer>(sectMap, weftSpacingZoomList  , wif_io::parse_int );
		} else if ("WEFT COLORS"         == sectName) { wif_io::parse_vecSect<Integer>(sectMap, weftColorList        , wif_io::parse_int );
		} else if ("WEFT SYMBOLS"        == sectName) { wif_io::parse_vecSect<Integer>(sectMap, weftSymbolList       , wif_io::parse_int );
		} else {
			throw std::invalid_argument("WIF contains unknown section type [" + sectName + "] that isn't marked as PRIVATE");
		}
	}

	// check that tables sizes match what was specified
	if (colorEntries      != colorTable     .size()) throw std::invalid_argument("[COLOR PALETTE] Entries="       + std::to_string(colorEntries) + " doesn't match number of [COLOR TABLE] keys:"       + std::to_string(colorTable     .size()));
	if (warpSymbolEntries != warpSymbolTable.size()) throw std::invalid_argument("[WARP SYMBOL PALETTE] Entries=" + std::to_string(colorEntries) + " doesn't match number of [WARP SYMBOL TABLE] keys:" + std::to_string(warpSymbolTable.size()));
	if (wefSymbolEntries  != warpSymbolTable.size()) throw std::invalid_argument("[WEFT SYMBOL PALETTE] Entries=" + std::to_string(colorEntries) + " doesn't match number of [WEFT SYMBOL TABLE] keys:" + std::to_string(warpSymbolTable.size()));

	// now that we have parsed everything look for any sections we didn't find
	for (std::pair<std::string, bool> const& p : contents) {
		if (p.second) throw std::invalid_argument("WIF [CONTENTS] lists " + p.first + " but it wasn't found");
	}

	sanityCheck();
}

void Wif::write(std::ostream& os) const {
	// start by printing the wif section
	os << "[WIF]\n";
	os << "Version="        << version    << '\n'; // TODO handle decimal (not clear why they didn't go with major/minor)
	os << "Date="           << date       << '\n';
	os << "Developers="     << developers << '\n';
	os << "Source Program=" << "libWIF"   << '\n';
	os << "Source Version=" << "0.1"      << '\n';
	os << '\n';

	// determine which sections we have (that arent trivial to check)
	const bool haveText = !title.empty() || !author.empty() || !address.empty() || !email.empty() || !telephone.empty();
	const bool haveWeaving = shafts > 0 || treadles > 0 || !risingShed;

	// now the contents
	os << "[CONTENTS]\n";
	if (!colorTable           .empty()) os << "COLOR PALETTE=true\n";
	if (!warpSymbolTable      .empty()) os << "WARP SYMBOL PALETTE=true\n";
	if (!weftSymbolTable      .empty()) os << "WEFT SYMBOL PALETTE=true\n";
	if ( haveText                     ) os << "TEXT=true\n";
	if ( haveWeaving                  ) os << "WEAVING=true\n";
	if ( 0 != warpThreads             ) os << "WARP=true\n";
	if ( 0 != weftThreads             ) os << "WEFT=true\n";
	if (!notes                .empty()) os << "NOTES=true\n";
	if (!tieUp                .empty()) os << "TIEUP=true\n";
	if (!colorTable           .empty()) os << "COLOR TABLE=true\n";
	if (!warpSymbolTable      .empty()) os << "WARP SYMBOL TABLE=true\n";
	if (!weftSymbolTable      .empty()) os << "WEFT SYMBOL TABLE=true\n";
	if (!threading            .empty()) os << "THREADING=true\n";
	if (!warpThicknessList    .empty()) os << "WARP THICKNESS=true\n";
	if (!warpThicknessZoomList.empty()) os << "WARP THICKNESS ZOOM=true\n";
	if (!warpSpacingList      .empty()) os << "WARP SPACING=true\n";
	if (!warpSpacingZoomList  .empty()) os << "WARP SPACING ZOOM=true\n";
	if (!warpColorList        .empty()) os << "WARP COLORS=true\n";
	if (!warpSymbolList       .empty()) os << "WARP SYMBOLS=true\n";
	if (!treadling            .empty()) os << "TREADLING=true\n";
	if (!liftPlan             .empty()) os << "LIFTPLAN=true\n";
	if (!weftThicknessList    .empty()) os << "WEFT THICKNESS=true\n";
	if (!weftThicknessZoomList.empty()) os << "WEFT THICKNESS ZOOM=true\n";
	if (!weftSpacingList      .empty()) os << "WEFT SPACING=true\n";
	if (!weftSpacingZoomList  .empty()) os << "WEFT SPACING ZOOM=true\n";
	if (!weftColorList        .empty()) os << "WEFT COLORS=true\n";
	if (!weftSymbolList       .empty()) os << "WEFT SYMBOLS=true\n";
	os << '\n';

	// sections that are more than just a list
	if (haveText) {
		os << "[TEXT]\n";
		if(!title    .empty()) os << "Title="     << title     << '\n';
		if(!author   .empty()) os << "Author="    << author    << '\n';
		if(!address  .empty()) os << "Address="   << address   << '\n';
		if(!email    .empty()) os << "Email="     << email     << '\n';
		if(!telephone.empty()) os << "Telephone=" << telephone << '\n';
		os << '\n';
	}

	if (haveWeaving) {
		os << "[WEAVING]\n";
		if (0 != shafts  ) os << "Shafts="   << shafts   << '\n';
		if (0 != treadles) os << "Treadles=" << treadles << '\n';
		os << "Rising Shed=" << (risingShed ? "true" : "false") << '\n'; // maybe we should use boolapha and reset the os flags?
		os << '\n';
	}

	if (0 != warpThreads) {
		os << "[WARP]\n";
		                             os << "Threads="        << warpThreads                        << '\n';
		if (0 != warpColorIndex   ) {os << "Color="          << warpColorIndex << ','; wif_io::put_rgb(os, warpColorValue); os << '\n';}
		if (0 != warpSymbol       ) {os << "Symbol="        ; wif_io::put_symb(os, warpSymbol); os << '\n';}
		if (0 != warpSymbolNum    ) {os << "Symbol Number="  << warpSymbolNum                      << '\n';}
		                             os << "Units="         ; wif_io::put_unit(os, warpUnit  ); os << '\n';
		if (!isnan(warpSpacing  ) ) {os << "Spacing="        << warpSpacing                        << '\n';}
		if (!isnan(warpThickness) ) {os << "Thickness="      << warpThickness                      << '\n';}
		if (1 != warpSpacingZoom  ) {os << "Spacing Zoom="   << warpSpacingZoom                    << '\n';}
		if (1 != warpThicknessZoom) {os << "Thickness Zoom=" << warpThicknessZoom                  << '\n';}
		os << '\n';
	}

	if (0 != weftThreads) {
		os << "[WEFT]\n";
		                             os << "Threads="        << weftThreads                        << '\n';
		if (0 != weftColorIndex   ) {os << "Color="          << weftColorIndex << ','; wif_io::put_rgb(os, weftColorValue); os << '\n';}
		if (0 != weftSymbol       ) {os << "Symbol="        ; wif_io::put_symb(os, weftSymbol); os << '\n';}
		if (0 != weftSymbolNum    ) {os << "Symbol Number="  << weftSymbolNum                      << '\n';}
		                             os << "Units="         ; wif_io::put_unit(os, weftUnit  ); os << '\n';
		if (!isnan(weftSpacing  ) ) {os << "Spacing="        << weftSpacing                        << '\n';}
		if (!isnan(weftThickness) ) {os << "Thickness="      << weftThickness                      << '\n';}
		if (1 != weftSpacingZoom  ) {os << "Spacing Zoom="   << weftSpacingZoom                    << '\n';}
		if (1 != weftThicknessZoom) {os << "Thickness Zoom=" << weftThicknessZoom                  << '\n';}
		os << '\n';
	}

	if (!notes                .empty()) {os << "[NOTES]\n"              ; for (auto const& p : notes                ) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!tieUp                .empty()) {os << "[TIEUP]\n"              ; for (auto const& p : tieUp                ) {os << p.first << '='; wif_io::put_vint(os, p.second); os << '\n';} os << '\n';}
	if (!colorTable           .empty()) {os << "[COLOR TABLE]\n"        ; for (auto const& p : colorTable           ) {os << p.first << '='; wif_io::put_rgb (os, p.second); os << '\n';} os << '\n';}
	if (!warpSymbolTable      .empty()) {os << "[WARP SYMBOL TABLE]\n"  ; for (auto const& p : warpSymbolTable      ) {os << p.first << '='; wif_io::put_symb(os, p.second); os << '\n';} os << '\n';}
	if (!weftSymbolTable      .empty()) {os << "[WEFT SYMBOL TABLE]\n"  ; for (auto const& p : weftSymbolTable      ) {os << p.first << '='; wif_io::put_symb(os, p.second); os << '\n';} os << '\n';}
	if (!threading            .empty()) {os << "[THREADING]\n"          ; for (auto const& p : threading            ) {os << p.first << '='; wif_io::put_vint(os, p.second); os << '\n';} os << '\n';}
	if (!warpThicknessList    .empty()) {os << "[WARP THICKNESS]\n"     ; for (auto const& p : warpThicknessList    ) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!warpThicknessZoomList.empty()) {os << "[WARP THICKNESS ZOOM]\n"; for (auto const& p : warpThicknessZoomList) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!warpSpacingList      .empty()) {os << "[WARP SPACING]\n"       ; for (auto const& p : warpSpacingList      ) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!warpSpacingZoomList  .empty()) {os << "[WARP SPACING ZOOM]\n"  ; for (auto const& p : warpSpacingZoomList  ) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!warpColorList        .empty()) {os << "[WARP COLORS]\n"        ; for (auto const& p : warpColorList        ) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!warpSymbolList       .empty()) {os << "[WARP SYMBOLS]\n"       ; for (auto const& p : warpSymbolList       ) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!treadling            .empty()) {os << "[TREADLING]\n"          ; for (auto const& p : treadling            ) {os << p.first << '='; wif_io::put_vint(os, p.second); os << '\n';} os << '\n';}
	if (!liftPlan             .empty()) {os << "[LIFTPLAN]\n"           ; for (auto const& p : liftPlan             ) {os << p.first << '='; wif_io::put_vint(os, p.second); os << '\n';} os << '\n';}
	if (!weftThicknessList    .empty()) {os << "[WEFT THICKNESS]\n"     ; for (auto const& p : weftThicknessList    ) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!weftThicknessZoomList.empty()) {os << "[WEFT THICKNESS ZOOM]\n"; for (auto const& p : weftThicknessZoomList) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!weftSpacingList      .empty()) {os << "[WEFT SPACING]\n"       ; for (auto const& p : weftSpacingList      ) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!weftSpacingZoomList  .empty()) {os << "[WEFT SPACING ZOOM]\n"  ; for (auto const& p : weftSpacingZoomList  ) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!weftColorList        .empty()) {os << "[WEFT COLORS]\n"        ; for (auto const& p : weftColorList        ) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}
	if (!weftSymbolList       .empty()) {os << "[WEFT SYMBOLS]\n"       ; for (auto const& p : weftSymbolList       ) {os << p.first << '='                    << p.second      << '\n';} os << '\n';}

}
