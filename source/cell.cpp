#include "cell.h"

#include <set>
#include <unordered_map>
#include <algorithm>
#include <iterator>
#include <stdexcept>

using namespace corvus;

uint_fast32_t Cell::layout(std::vector<uint_fast32_t>& threading, std::vector< std::vector<uint_fast32_t> >& tieup, std::vector< std::vector<uint_fast32_t> >& treadling) const {
	// the algorithm we need to use here is called 'partition refinement'
	// it is essentially dual to the more common disjoint set / union find structure
	// the idea is that in the beginning all of our warps are in a single harness (set)
	// we loop over all the sheds required and use them to split sets as follows:
	// initially all warps are in 1 set
	// for each shed:
	//   loop over all the sets we have so far
	//     split set into the union and the intersection with the shed (new set)

	// it is probably worth reducing down to unique shed types instead of doing every single weft in the entire design
	// we'll nered to use a set/map of some type to achieve it
	// ultimately (when building the tie up / treadling) it will be helpful to have a vectorized version
	std::vector< std::vector<uint_fast32_t> > sheds; // every shed in the cell (list of lifted warps for each)
	std::vector<uint_fast32_t> weftSheds; // index into sheds for every weft

	{
		// start by building up a list of all the sheds in our cell
		std::set< std::vector<uint_fast32_t> > uniqueSheds; // unique sheds
		std::vector< std::set< std::vector<uint_fast32_t> >::iterator > weftTypes; // the shed type for each weft
		weftTypes.reserve(wefts);
		for (uint_fast32_t j = 0; j < wefts; j++) { // loop over warps
			std::vector<uint_fast32_t> s;
			const size_t offset = size_t(j) * size_t(warps); // get offset to start of the current warp
			for (uint_fast32_t i = 0; i < warps; i++) if (mask[offset + i]) s.push_back(i); // loop over threads building up those included in the shed (we end up with a sorted list)
			weftTypes.push_back( uniqueSheds.insert(s).first ); // first is iterator to new or existing element
		}

		// now do the conversion to vectorized
		sheds.assign(uniqueSheds.cbegin(), uniqueSheds.cend());
		std::unordered_map<std::vector<uint_fast32_t>const*, uint_fast32_t> shedMap; // we really want std::set< std::vector<uint_fast32_t> >::iterator but that doesn't work without defining a hash
		uint_fast32_t n = 0;
		for (std::set< std::vector<uint_fast32_t> >::iterator iter = uniqueSheds.cbegin(); iter != uniqueSheds.cend(); ++iter) shedMap[&(*iter)] = n++;
		weftSheds.resize(wefts);
		for (uint_fast32_t j = 0; j < wefts; j++) weftSheds[j] = shedMap[&(*weftTypes[j])];
	}
	// now sheds contains a list of unique shed configurations
	// weftSheds contains which shed is used for each weft
	// the next step is to get a list of shafts
	std::vector< std::vector<uint_fast32_t> > shafts; // for each shaft a list of warp threads that are lifted by it

	{
		// we could use std::vector<std::set> but why bother, we are going over everything in a pre-sorted order
		// instead we can just store our sets vecotrized
		// at each step we are essentially un-doing an std::inplace_merge (i.e. a std::partition)
		             std::vector< std::pair<uint_fast32_t, bool> >             shaftSets(warps); // storage for our sets (we know the total number of elements never changed), bool is flag to help with std::partition 
		std::vector< std::vector< std::pair<uint_fast32_t, bool> >::iterator > bounds = {shaftSets.begin(), shaftSets.end()}; // the index of the first element of each set in shafts, keep an end / +1 value for convenience
		for (uint_fast32_t i = 0; i < warps; i++) shaftSets[i] = std::pair<uint_fast32_t, bool>(i, false); // we start with 1 set containing every warp

		// loop over all of our sheds
		std::vector< std::pair<uint_fast32_t, bool > > setI, setD, curShed;
		setI.reserve(warps); setD.reserve(warps); curShed.reserve(warps);
		for (std::vector<uint_fast32_t> const& x : sheds) { // loop over unique sheds
			// build into pair<uint_fast32_t, bool> once
			curShed.clear();
			for (uint_fast32_t const& i : x) curShed.emplace_back(i, false);
			for (size_t i = 1; i < bounds.size(); i++) { // loop over our sets doing the partition
				// get our set bounds
				std::vector< std::pair<uint_fast32_t, bool> >::iterator start = bounds[i-1]; // iterator to first element in current set
				std::vector< std::pair<uint_fast32_t, bool> >::iterator end   = bounds[i  ]; // iterator past last element in current set

				// construct intersection/difference
				setI.clear(); std::set_intersection(start, end, curShed.begin(), curShed.end(), std::back_inserter(setI));
				setD.clear(); std::set_difference  (start, end, curShed.begin(), curShed.end(), std::back_inserter(setD));
				if (setI.empty() || setD.empty()) continue; // nothing to be done

				// do the partitioning
				for (auto iter = start; iter != end; ++iter) {
					if (std::binary_search(setI.begin(), setI.end(), *iter)) iter->second = true; // flag intersection for partitioning
				}
				std::vector< std::pair<uint_fast32_t, bool> >::iterator mid = std::partition(start, end, [](std::pair<uint_fast32_t, bool> const& p){return p.second;});
				
				// update our list of sets
				// note that std::partition doesn't preserve the relative order so the sets in 'shafts' are now unsorted
				// we need to resort so that set_intersection/set_difference work on the next pass
				std::sort(start, mid);
				std::sort(mid  , end);
				for (auto iter = start; iter != mid; ++iter) iter->second = false; // clear all the flags we just set

				// now save the new set
				bounds.insert(bounds.begin() + i, mid);
				i++; // we don't have to check against the new set we just made
			}
		}

		// save a softed copy of the shafts we have
		// there isn't any 1 particular rule that makes the most sense for numbering shafts
		// I'll sort so that the most populated harnesses come first with lexicographic compare as a tie break
		auto shaftSort = [](std::vector<uint_fast32_t>const& lhs, std::vector<uint_fast32_t>const& rhs) {
			return lhs.size() == rhs.size() ? std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend()) : lhs.size() > rhs.size();
		};
		shafts.resize(bounds.size() - 1);
		for (size_t i = 1; i < bounds.size(); i++) {
			for (auto iter = bounds[i-1]; iter != bounds[i]; ++iter) shafts[i-1].push_back(iter->first); // discard the bools that were used for partitioning
		}
		std::sort(shafts.begin(), shafts.end(), shaftSort);
	}

	// now that we have partitioned the warps into shafts we can build up the threading
	threading.resize(warps);
	for (size_t i = 1; i < shafts.size(); i++) {
		for (uint_fast32_t const& w : shafts[i]) {
			threading[w] = static_cast<uint_fast32_t>(i); // should be safe unless we have 2^32 warps...
		}
	}

	// next determine which shafts are needed for each shed
	std::vector< std::vector<uint_fast32_t> > shedShafts(sheds.size());
	for (size_t j = 0; j < sheds.size(); j++) { // loop over the shed configurations
		for (size_t i = 0; i < shafts.size(); i++) { // loop over shafts
			if (std::includes(sheds[j].cbegin(), sheds[j].cend(), shafts[i].cbegin(), shafts[i].cend())) {
				shedShafts[j].push_back(static_cast<uint_fast32_t>(i)); // if we have more than 2^32 shafts we have bigger problems
			}
		}
	}

	// next determine the tie up, this is the hardest part if we have a finite number of treadles
	// what we want is to optimize the tie up (minimize total treadle presses)
	// the objective function we would like minimize is the maximum number of simultaneous treadle presses
	// e.g. it is better to press 2 treadles on every weft than 1 treadle on most wefts and 10 on a single weft
	// in practice this is a relatively soft constraint and there are other goals weavers may prefer
	// for example having 2 treadles pressed at once matters less if they are adjacent
	// also my impression is that weavers prefer a treadling sequence with a nice cadence / pattern
	// unfortunately I'm not sure how to combine all these considerations into objective function / constrains
	// this is a harder version of the set covering problem so it is almost certainly NP hard
	// the language to search for is 'skeleton tie up'
	// there is a nice brute force approach implemeted by "tim's treadle reducer"
	// https://cs.earlham.edu/~timm/treadle/index.php

	// we may want an optional user parameter for how many treadles are available
	size_t availableTreadles = shafts.size(); // we could allow a user to say that have e.g. 6 treadles for a 4 shaft design
	if (availableTreadles < shafts.size()) throw std::invalid_argument("not enough treadles for design"); // since we have the optimum threading we need at least that many treadles

	// now that we have our set covering weights, do the greedy set cover
	if (sheds.size() <= availableTreadles) { // trivial case
		// we just assign a shed to each treadle
		tieup.resize(shedShafts.size());
		for (size_t i = 0; i < shedShafts.size(); i++) tieup[i] = shedShafts[i];

		// treadling is trivial as well
		treadling.resize(wefts);
		for (uint_fast32_t j = 0; j < wefts; j++) treadling[j] = std::vector<uint_fast32_t>(1, weftSheds[j]);
	} else {
		// TODO try building a reduced treadling using tim's algorithm
	}

	if (tieup.empty()) { // we don't have enough treadles to 
		// build straight tieup, this always works but it may not be pretty (e.g. on an n shaft loom you may need to press n-1 treadles at once)
		tieup.resize(shafts.size());
		for (size_t i = 0; i < shafts.size(); i++) tieup[i] = std::vector<uint_fast32_t>(1, static_cast<uint_fast32_t>(i)); // if we have more than 2^32 shafts we have bigger problems

		// finally build treadling
		// for a straight tie up this is easy
		treadling.resize(wefts);
		for (uint_fast32_t j = 0; j < wefts; j++) treadling[j] = shedShafts[weftSheds[j]];
	}

	// not helpful for a straight tieup but could be useful otherwise
	// we could also pass in an allowable number of shafts to make less minimal shaft but more user friendly tieups
	return static_cast<uint_fast32_t>(shafts.size()); // we have bigger issues if there are more than 2^32 shafts...
}


void Cell::writeWeft(uint_fast32_t r, std::ostream& os, char warpSymb, char weftSymb) const {
	for (uint_fast32_t c = 0; c < warps; c++) os << ' ' << (mask[r * warps + c] ? '#' : '.');
}

void Cell::write(std::ostream& os, char warpSymb, char weftSymb) const {
	for (uint_fast32_t r = 0; r < wefts; r++) {
		writeWeft(r, os, warpSymb, weftSymb);
		os << '\n';
	}
}
