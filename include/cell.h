/*
 * Copyright (c) William Lenthe
 * all rights reserved
 * please see the license file for more details
 */

#ifndef _CORVUS_CELL_H_
#define _CORVUS_CELL_H_
#pragma once

#include <vector>
#include <ostream>

namespace corvus {
	// some quick weaving vocabulary (w/ a handweaving floor loom perspective)
	// warp: threads held in tension parallel to the direction the fabric is getting longer during weaving
	// heddle: eye or similar that each warp goes through so it can be lifted up or down relative to the baseline position
	// shaft / harness: object that lifts a group of heddles together
	// threading: description of which warp is lifted by which shaft
	// treadle: peddles used to lifts shafts
	// tieup: configuration of strings connecting the shafts to the treadles (to control which warps are lifted by each triadle)
	// shed: the space created between the liefted and unlifted warp threads (typically used to denote a specific configuration, i.e. n shafts can create 2^n-2 sheds (since none/all lifted don't count)
	// rising shed: a loom where the warps are actuated up from the static ones by pressing the treadles (typically indicated with 'o' in the tieup)
	// falling shed: a loom where the warps are actuated down from the static ones by pressing the treadles (typically indicated with 'x' in the tieup)
	// weft: thread that passes back and forth through the sheds to build up the fabric
	// treadling: sequence of treadle presses used to create the sheds for a weave
	// drawdown: the image version of fabric created by threading + tie up + treadling
	// block: a sub unit of the drawdown, typically a repeating rectangle

	//! a binary (black/white) drawdown that is a building block for larger pattern)
	//! thick could probably be referred to as a weave, pattern, diagram or similar
	//! I chose Cell specifically since I'm not aware of its use in weaving
	//! that way there is no worry about ambiguity, we can define the meaning ourselves
	//! the intention of this class is to be the smallest repeat unit in a full drawdown
	struct Cell {
		uint_fast32_t             warps; //!< how many warps (image width)
		uint_fast32_t             wefts; //!< how many wefts (image height)
		std::vector<uint_fast8_t> mask ; //!< is the warp (1) or weft(0) on top for each pixel
		                                 //!< this is in row major order (shed by shed) starting from the bottom left
		                                 //!< we could have used a vector<bool> here but the specialization has some issues

		//! invert from the binary drawdown to the setup needed to create it
		//! \param threading location to write the shaft (0 indexed) that each warp thread goes through
		//! \param tieup location to write the list of shafts (0 indexed) that each treadle lifts
		//! \param treadling location to write the list of treadles (0 indexed) that is pressed for each warp
		//! \return minimum number of shafts required (this is typically a limiting factor on looms)
		//! \note this assumes a rising shed, maybe we should make that an argument
		uint_fast32_t layout(std::vector<uint_fast32_t>& threading, std::vector< std::vector<uint_fast32_t> >& tieup, std::vector< std::vector<uint_fast32_t> >& treadling) const;

		//! print a single weft to a text file
		//! \param r weft row to write
		//! \param os ostream to write to
		//! \param warpSymb character to use to represent the warp on top
		//! \param weftSymb character to use to reprsent the weft on top
		void writeWeft(uint_fast32_t r, std::ostream& os, char warpSymb = '#', char weftSymb = '.') const;

		//! print cell to a text file
		//! \param os ostream to write to
		//! \param warpSymb character to use to represent the warp on top
		//! \param weftSymb character to use to reprsent the weft on top
		void write(std::ostream& os, char warpSymb = '#', char weftSymb = '.') const;
	};
}

inline std::ostream& operator<<(std::ostream& os, corvus::Cell const& c) {c.write(os); return os;}

#endif//_CORVUS_CELL_H_
