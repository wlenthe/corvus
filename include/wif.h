#ifndef _WIF_H_
#define _WIF_H_
#pragma once

#include <string>
#include <vector>
#include <istream>
#include <ostream>
#include <array>

namespace corvus {

	//! interface for reading an writing 'weaving information files'
	//! \note everything in a wif is 1 indexed instead of 0 indexed
	struct Wif {

		typedef double                     Real   ;
		typedef std::string                String ;
		typedef uint32_t                   Integer;
		typedef bool                       Boolean;
		typedef std::array<Integer, 3>     Color  ;
		typedef char                       Symbol ;
		typedef std::vector<Integer>       VecInt ;
		typedef std::pair<Integer,Integer> Range  ;

		//! check if the current values are resonable and cleans things up a bit, throws invalid_argument if not
		void sanityCheck();

		enum class Unit {
			None       ,
			Decipoints ,
			Inches     ,
			Centimeters,
		};

		// minimum required sections

		////////////////////////////////
		//            WIF             //
		////////////////////////////////

		Real   version    = 1.1             ; //!< wif version, stopped at 1.1 but there is 1.0
		String date       = "April 20, 1997"; //!< assuming this is the date of the version, not the file
		String developers = "wif@mhsoft.com"; //!< this seems to be fixed since development is defunct
		String sourceProg = ""              ; //!< software that created the wif [required]
		String sourceVers = ""              ; //!< version of software

		////////////////////////////////
		//          CONTENTS          //
		////////////////////////////////

		// lets list include standard sections in the default constructor
		// the following sections are resevered for future use (seems unlikely there will ever be a v 1.2)
		// BITMAP IMAGE
		// BITMAP FILE
		// BITMAP IMAGE DATA
		// we don't actually need to store this here (or in the file if it wasn't part of the standard) since it is implied
		/*
		std::vector< std::pair< std::string, bool > > contents = { //!< you only have to print out the contents for sections that are true
			// informational sections
			{"COLOR PALETTE"      , false},
			{"WARP SYMBOL PALETTE", false},
			{"WEFT SYMBOL PALETTE", false},
			{"TEXT"               , false},
			{"WEAVING"            , false},
			{"WARP"               , false},
			{"WEFT"               , false},
			// data sections
			{"NOTES"              , false},
			{"TIEUP"              , false},
			// color and symbol tables
			{"COLOR TABLE"        , false},
			{"WARP SYMBOL TABLE"  , false},
			{"WEFT SYMBOL TABLE"  , false},
			// warp data sections
			{"THREADING"          , false},
			{"WARP THICKNESS"     , false},
			{"WARP THICKNESS ZOOM", false},
			{"WARP SPACING"       , false},
			{"WARP SPACING ZOOM"  , false},
			{"WARP COLORS"        , false},
			{"WARP SYMBOLS"       , false},
			// weft data sections
			{"TREADLING"          , false},
			{"LIFTPLAN"           , false},
			{"WEFT THICKNESS"     , false},
			{"WEFT THICKNESS ZOOM", false},
			{"WEFT SPACING"       , false},
			{"WEFT SPACING ZOOM"  , false},
			{"WEFT COLORS"        , false},
			{"WEFT SYMBOLS"       , false},
		};
		*/
		// there can also be PRIVATE <SourceID> <SectionName>
		// you are supposed to reguster you SourceID with the deveopers if you want to use it
		// here are the registerd pairs:
		// MHSPWSW: maple hill software
		// SWve   : swiftweave

		// end required sections

		////////////////////////////////
		//       COLOR PALETTE        //
		////////////////////////////////
		// entries is emplied from the size of the palette colorTable
		// form=rgb is obsolete constant string from v1
		Range range = {0, 0};

		// WARP SYMBOL PALETTE: entries is implied by length of warp symbol table warpSymbols
		// WEFT SYMBOL PALETTE: entries is implied by length of weft symbol table weftSymbols

		////////////////////////////////
		//            TEXT            //
		////////////////////////////////
		String title    ; //!< title of the weaving
		String author   ; //!< name of artist
		String address  ; //!< address of artist
		String email    ; //!< email of artist
		String telephone; //!< voice number of artist
		String fax      ; //!< fax number of artist

		////////////////////////////////
		//          WEAVING           //
		////////////////////////////////
		Integer shafts     = 0   ; //!< number of shafts/harnesses
		Integer treadles   = 0   ; //!< number of treadles
		Boolean risingShed = true; //!< to treadles lift (true) or lower (false) the attached warp threads
		// profile was in v1 but obsolete in 1.1

		////////////////////////////////
		//            WARP            //
		////////////////////////////////
		// obsolete colors (integer)
		// obsolete pallete = yes
		// obsolete colormix ?
		Integer warpThreads       = 0         ; //!< number of warp threads, may not match threading.size if threading is sparse
		Integer warpColorIndex    = 0         ; //!< index into default 
		Color   warpColorValue    ={0,0,0}    ; //!< optional value to use for 2 color drafts instead of lookup table
		Symbol  warpSymbol        = 0         ; //!< symbol to use for 2 symbol drafts instead of lookup table
		Integer warpSymbolNum     = 0         ; //!< default value for warp threads that don't have an explictly listed symbol
		Unit    warpUnit          = Unit::None; //!< for thickness / spacing
		Real    warpSpacing       = NAN       ; //!< base warp spacing
		Real    warpThickness     = NAN       ; //!< base warp thickness
		Integer warpSpacingZoom   = 1         ; //!< zoom factor for base warp spacing
		Integer warpThicknessZoom = 1         ; //!< zoom factor for base warp thickness

		////////////////////////////////
		//            WEFT            //
		////////////////////////////////
		Integer weftThreads       = 0         ; //!< number of weft threads, may not match treadling.size if treadling is sparse
		Integer weftColorIndex    = 0         ; //!< index into default 
		Color   weftColorValue    ={0,0,0}    ; //!< optional value to use for 2 color drafts instead of lookup table
		Symbol  weftSymbol        = 0         ; //!< symbol to use for 2 symbol drafts instead of lookup table
		Integer weftSymbolNum     = 0         ; //!< default value for weft threads that don't have an explictly listed symbol
		Unit    weftUnit          = Unit::None; //!< for thickness / spacing
		Real    weftSpacing       = NAN       ; //!< base wef spacing
		Real    weftThickness     = NAN       ; //!< base wef thickness
		Integer weftSpacingZoom   = 1         ; //!< zoom factor for base wef spacing
		Integer weftThicknessZoom = 1         ; //!< zoom factor for base wef thickness

		////////////////////////////////
		//     n=value sections       //
		////////////////////////////////
		std::vector< std::pair< Integer, String  > > notes                ; //!< NOTES: list of notes for humans
		std::vector< std::pair< Integer, VecInt  > > tieUp                ; //!< TIEUP: a list of shafts/harnesses connected for each treadle
		std::vector< std::pair< Integer, Color   > > colorTable           ; //!< COLOR TABLE: rgb triples in [range.first, range.second]
		std::vector< std::pair< Integer, Symbol  > > warpSymbolTable      ; //!< WARP SYMBOL TABLE: a glyph character to represent each warp instead of a number (not sure how this is used in practice)
		std::vector< std::pair< Integer, Symbol  > > weftSymbolTable      ; //!< WEFT SYMBOL TABLE: a glyph character to represent each weft instead of a number (not sure how this is used in practice)
		std::vector< std::pair< Integer, VecInt  > > threading            ; //!< THREADING: for each warp a list of the shafts that it goes through
		
		std::vector< std::pair< Integer, Real    > > warpThicknessList    ; //!< WARP THICKNESS: thickness of individual warps
		std::vector< std::pair< Integer, Integer > > warpThicknessZoomList; //!< WARP THICKNESS ZOOM: zoom factor for individual warps
		std::vector< std::pair< Integer, Real    > > warpSpacingList      ; //!< WARP SPACING: spacing around individual warps
		std::vector< std::pair< Integer, Integer > > warpSpacingZoomList  ; //!< WARP SPACING ZOOM: spacing around individual warps
		std::vector< std::pair< Integer, Integer > > warpColorList        ; //!< WARP COLORS: index into colorTable for each warp
		std::vector< std::pair< Integer, Integer > > warpSymbolList       ; //!< WARP SYMBOLS: index into warpSymbolTable for each warp

		std::vector< std::pair< Integer, VecInt  > > treadling            ; //!< TREADLING: for each weft row a list of treadles pressed
		std::vector< std::pair< Integer, VecInt  > > liftPlan             ; //!< LIFTPLAN: for weft row a list of shafts/harnesses lifted

		std::vector< std::pair< Integer, Real    > > weftThicknessList    ; //!< WEFT THICKNESS: thickness of individual werfs
		std::vector< std::pair< Integer, Integer > > weftThicknessZoomList; //!< WEFT THICKNESS ZOOM: zoom factor for individual werfs
		std::vector< std::pair< Integer, Real    > > weftSpacingList      ; //!< WEFT SPACING: spacing around individual werfs
		std::vector< std::pair< Integer, Integer > > weftSpacingZoomList  ; //!< WEFT SPACING ZOOM: spacing around individual werfs
		std::vector< std::pair< Integer, Integer > > weftColorList        ; //!< WEFT COLORS: index into colorTable for each weft
		std::vector< std::pair< Integer, Integer > > weftSymbolList       ; //!< WEFT SYMBOLS: index into weftSymbolTable for each weft

		void read(std::istream& is);

		void write(std::ostream& os) const;

		void clear() {*this = Wif();}

	};
}

inline std::istream& operator>>(std::istream& is, corvus::Wif      & w) {w.read (is); return is;}
inline std::ostream& operator<<(std::ostream& os, corvus::Wif const& w) {w.write(os); return os;}


#endif//_WIF_H_
