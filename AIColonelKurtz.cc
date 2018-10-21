#include "Player.hh"
#include <vector>
#include <stdio.h>
#include <queue>
#include <iostream>
#include <map>
using namespace std;


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME ColonelKurtz


struct PLAYER_NAME : public Player {

	/**
	* Factory: returns a new instance of this class.
	* Do not modify this function.
	*/
	static Player* factory () {	return new PLAYER_NAME;	}

	/**
	* Types and attributes for your player can be defined here.
	*/
	
	typedef vector<int> Vect;
	typedef vector< vector<bool> > Mat_bool;
	
	const int MINE 		= 1;
	const int ENEMY		= 2;
	const int OTHERS	= 3;
	const int ALL 		= 4;
	
	const int SOUTH		= 0;
	const int EAST		= 1;
	const int NORTH		= 2;
	const int WEST		= 3;
		
	const int BFS_S_NORMAL		= 1;
	const int BFS_S_FOREST		= 2;
	const int BFS_S_LOW_VALUE	= 3;
	const int BFS_S_HIGH_VALUE	= 4;
	const int BFS_S_ENEMIES		= 5;
	const int BFS_S_BUDDIES		= 6;
	const int BFS_S_POSITION	= 7;
	
	const int MIN_ENEMIES_UNDER_NAPALM			= 1;
	const int MAX_BUDDIES_UNDER_NAPALM			= 0;
	const int PARACHUTER_ALMOST_DEAD_JUMP 		= 16;
	
	const int ENEMY_RANGE       = 1;
	
	const int ATTACK_MAX_PROBABILITY_ALL 		= 1; 		// ALL_BIOMES: attack if ((random(1, ATTACK_MAX_PROBABILITY_ALL)) == 1);
	const int ATTACK_MIN_HEALTH_ME_ALL	 		= 50;		// ALL_BIOMES: attack if (my_health > ATTACK_MIN_HEALTH_ME_ALL)
	const int ATTACK_MAX_HEALTH_ENEMY_ALL	 	= 51;		// ALL_BIOMES: attack if (enemy_health < ATTACK_MAX_HEALTH_ENEMY_ALL)
	
	const double MAX_CPU_time_allowed = 0.92;
	
	const Position PosErr = Position(-1,-1);
	
	int parachuters_max_jump;
	int zero = 0;
	
	// Stores, if needed, the next instruction for an helicopter
	Position nhi_1, nhi_2;
			
	// displacement of current position
	const vector<Position> displ = {  	Position(-1,0),
										Position(-1,1),
										Position(0,1),
                                		Position(1,1),
                                		Position(1,0),
                                		Position(1,-1),
                                		Position(0,-1),
                                		Position(-1,-1) };

	
	struct TypePathInfo {
	    bool is_valid;
		
	    int dist_nearest_post;
	    Position pos_nearest_post;
		
	    int dist_nearest_low_post;
	    Position pos_nearest_low_post;
		
	    int dist_nearest_high_post;
	    Position pos_nearest_high_post;
		
        vector<int> enemies_id;
		
        int dist_nearest_buddy_soldier;
        int id_nearset_buddy_soldier;
	    
		//////////////////////////////////

	    TypePathInfo() : is_valid(true) {}
	};
	
	///*****************************************************************************************************///
	///*****************************************************************************************************///
	///*****************************************************************************************************///

	/**
	*	fillDisplacements()
	*
	void fillDisplacements()
	{
		displ.push_back(Position(-1,0));		// N 	->	displ[0]
		displ.push_back(Position(-1,1));		// NE	->	displ[1]
		displ.push_back(Position(0,1));			// E    ->	displ[2]
		displ.push_back(Position(1,1));			// SE   ->	displ[3]
		displ.push_back(Position(1,0));			// S    ->	displ[4]
		displ.push_back(Position(1,-1));		// SW   ->	displ[5]
		displ.push_back(Position(0,-1));		// W    ->	displ[6]
		displ.push_back(Position(-1,-1));		// NW   ->	displ[7]
	}
	*/
	
	/**
	*	elemInVector()
	*/
	bool elemInVector(const vector<int> &v, int x)
	{
		for (unsigned int i = 0; i < v.size(); ++i) if (v[i] == x) return true;
		return false;
	}
	
	/**
	*	printPos()
	*/
	void printPos(Position p, const string what_is_this_pos)
	{
		cerr << what_is_this_pos << ": (" << p.i << ", " << p.j << ")\n";
	}
	
	/**
	*	printVec()
	*/
	void printVec(const vector<int> &v, const string &s)
	{
		cerr << "printVector-> " << s << "  --> ";
		for (unsigned int j = 0; j < v.size(); ++j) cerr << v[j] << " ";
		cerr << endl;
	}
	
	/**
	*	printMat()
	*/
	void printMat(const Mat_bool &visited)
	{
		for (int i = 0; i < MAX; ++i) {
			for (int j = 0; j < MAX; ++j) cerr << visited[i][j];
			cerr << endl;
		}
		cerr << endl;
	}
	
	/**
	*	printError()
	*/
	void printError(int type, const string &error_passed_by_param)
	{
		string texte;
		if (type == 0) texte = error_passed_by_param;
		if (type == 1) texte = "Position not OK or invalid unit_type.";
		if (type == 2) texte = "Position not OK";
		if (type == 3) texte = "Position_ini has to be smaller than Position_End";
		if (type == 4) texte = "Invalid unit_type";
		if (type == 5) texte = "Not any unit at this pos";
		if (type == 6) texte = "";
		if (type == 7) texte = "";
		if (type == 8) texte = "";
		if (type == 9) texte = "";
		if (type == 10) texte = "";
		if (type == 11) texte = "";
		if (type == 12) texte = "";

		cerr << endl << "<Error!>  " << texte << " </Error!>" << endl;
	}
	
	/**
	*	validUnitType()
	*/
	bool validUnitType(int unit_type)
	{
		if (unit_type == SOLDIER or unit_type == HELICOPTER) return true;
		else { printError(4,""); return false; }
	}
	
	/**
	*	amIWinning()
	*/
	bool amIWinning()
	{
		for (int pl = 0; pl < nb_players(); ++pl) {
			if (pl != me() and total_score(pl) >= total_score(me())) return false;
		}
		return true;
	}

	/**
	*	inFire()
	*/
	bool inFire(Position p)
	{
		if (not pos_ok(p)) { printError(2,""); return false; }
		int f = fire_time(p.i, p.j);
		if (f == 0 or f == -1) return false;
		return true;
	}

	/**
	*	willProbablyBurnNextRound()
	*/
	bool willProbablyBurnNextRound(Position p)
	{
		if (not pos_ok(p)) { printError(0,"Err! willProbBurn..."); return false; }
		if (inFire(p)) return true;
		// check whether or not an adjacent cell is burning
		return (inFire(Position(p.i-1, p.j)) or inFire(Position(p.i+1, p.j)) or inFire(Position(p.i, p.j-1)) or inFire(Position(p.i, p.j+1)));
	}
	
	/**
	*	isPost()
	*/
	bool isPost(Position p)
	{
		Post post_k = Post();
		vector<Post> posts = State::posts();
		for (unsigned int k = 0; k < posts.size(); ++k) {
			post_k = posts[k];
			if (isEqualPos(post_k.pos, p)) return true;	// there is a post at 'P'
		}
		return false;
	}
	
	/**
	*	sumPosPos()
	*/
	Position sumPosPos(Position p1, Position p2)
	{
		Position r = Position(p1.i + p2.i, p1.j + p2.j);
		return r;
	}
	
	/**
	*	quadrant_pos_ok()
	*/
	bool quadrant_pos_ok(Position p)
	{
		//Each player starts in a quadrant of the board: players 0, 1, 2 and 3 are initially in the NW, NE, SE and SW quadrants, respectively.
		
		if (Player::me() == 0) return (p.i >= 0 and p.i < MAX/2 and p.j >= 0 and p.j < MAX/2);
		if (Player::me() == 1) return (p.i >= 0 and p.i < MAX/2 and p.j >= MAX/2 and p.j < MAX);
		if (Player::me() == 2) return (p.i >= MAX/2 and p.i < MAX and p.j >= MAX/2 and p.j < MAX);
		if (Player::me() == 3) return (p.i >= MAX/2 and p.i < MAX and p.j >= 0 and p.j < MAX/2);
		return false;
		
		if (Player::me() == 0 or Player::me() == 3) return (p.j < MAX/2);
		else return (p.j >= MAX/2);
		
	}
	
	/**
	*	isAccessiblePos()
	*/
	bool isAccessiblePos(Position p, int unit_type)
	{
		// Check if 'new_pos' is inside the board and 'unit_type' is a valid one
		if (pos_ok(p) and validUnitType(unit_type)) {
			if (unit_type == SOLDIER) {
				if (what(p.i,p.j) == WATER or what(p.i,p.j) == MOUNTAIN) return false;
				else return true;	// 'p' is GRASS or FOREST
			}
			if (unit_type == HELICOPTER) {
				if (what(p.i,p.j) == MOUNTAIN) return false;
				else return true;	// 'p' is in GRASS, FOREST or WATER
			}
			return false;
		}
		else { /*printError(1,"");*/ return false; }
	}
	
	/**
	*	isPosEmptyOfThisUnitType()
	*/
	bool isPosEmptyOfThisUnitType(Position p, int unit_type, int heli_id)
	{
		if (not validUnitType(unit_type)) { printError(0,"Error! isPosEmptyOf..."); return false; }
		if (not isAccessiblePos(p, unit_type)) { /*printError(2,"");*/ return false; }
		if (unit_type == SOLDIER) {
			if (inFire(p)) return false; // ¿?
			if (which_soldier(p.i, p.j) == 0) return true;
		}
		if (unit_type == HELICOPTER) {
			if (not posInHelicopterRange(p, OTHERS, heli_id)) return true;
		}
		return false;
		
	}
	
	/**
	*	isEqualPos()
	*/
	bool isEqualPos(Position p1, Position p2)
	{
		if ((p1.i == p2.i) and (p1.j == p2.j)) return true;
		return false;
	}
	
	/**
	*	isPosErr()
	*/
	bool isPosErr(Position p)
	{
		return (isEqualPos(p,PosErr));
	}
	
	/**
	*	posIncludedInRange()
	*/
	bool posIncludedInRange(Position p, Position p_ini, Position p_end)
	{
		if (not pos_ok(p) or not pos_ok(p_ini) or not pos_ok(p_end)) { /*printError(2,"");*/ return false; }
		if ((p_ini.i > p_end.i) or (p_ini.j > p_end.j)) { printError(3,""); return false; }
		
		if ((p.i >= p_ini.i) and (p.j >= p_ini.j)) {
			if ((p.i <= p_end.i) and (p.j <= p_end.j)) { return true; }
		}
		return false;
	}
	
	/**
	*	getPosOfPostUnderHelicopter()
	*/
	Position getPosOfPostUnderHelicopter(Position p)
	{
		if (not pos_ok(p)) { return PosErr; }
		
		Position p_under_heli_ini = Position(p.i - 2, p.j - 2);
		Position p_under_heli_end = Position(p.i + 2, p.j + 2);
		vector<Post> posts = State::posts();
		for (unsigned int k = 0; k < posts.size(); ++k) {
			if(posIncludedInRange(posts[k].pos, p_under_heli_ini, p_under_heli_end)) return posts[k].pos;
		}
		return PosErr;
	}
	
	/**
	* getCPosFromHPos()
	*/
	Position getCPosFromHPos(Position hpos, int ori)
	{
		if (ori == SOUTH)	return Position(	hpos.i-2, hpos.j	);
		if (ori == EAST) 	return Position(	hpos.i	, hpos.j-2	);
		if (ori == NORTH)	return Position(	hpos.i+2, hpos.j	);
		if (ori == WEST) 	return Position(	hpos.i	, hpos.j+2	);
		
		return PosErr;
	}
	
	/**
	*	getIdOfHelicopterAt()
	*/
	int getIdOfHelicopterAt(Position pos)
	{
		if (not pos_ok(pos)) { printError(1,""); return -1; }
		
		Position ph = Position();
		for (int pl = 0; pl < nb_players(); ++pl) {
			vector<int> h = helicopters(pl);
			for (unsigned int i = 0; i < h.size(); ++i) {
				ph = data(h[i]).pos;
				if (isEqualPos(ph, pos)) return data(h[i]).id;
				
				Position ph_ini = Position(ph.i-2, ph.j-2);
				Position ph_end = Position(ph.i+2, ph.j+2);
				if (posIncludedInRange(pos, ph_ini, ph_end)) return data(h[i]).id;
			}
		}
		
		return 0; 	// This position does NOT contain an helicopter
	}
	
	/**
	*	pos1FurtherThanPos2()
	*/
	bool pos1FurtherThanPos2(Position p, Position p1, Position p2)
	{
		int i1 = abs(p.i - p1.i);
		int j1 = abs(p.j - p1.j);
		int i2 = abs(p.i - p2.i);
		int j2 = abs(p.j - p2.j);
		
		int d1 = max(i1, j1);
		int d2 = max(i2, j2);
		
		return (d1 > d2);
	}
	
	/**
	*	getContraryPos()
	*/
	Position getContraryPos(Position p_me, Position p_en)
	{
		if (not pos_ok(p_me) or not pos_ok(p_en)) return PosErr;
		
		if (p_en.i < p_me.i  and p_en.j <  p_me.j) return Position(p_me.i + 1, p_me.j + 1);
		if (p_en.i < p_me.i  and p_en.j == p_me.j) return Position(p_me.i + 1, p_me.j    );
		if (p_en.i < p_me.i  and p_en.j >  p_me.j) return Position(p_me.i + 1, p_me.j - 1);
		
		if (p_en.i == p_me.i and p_en.j <  p_me.j) return Position(p_me.i    , p_me.j + 1);
		if (p_en.i == p_me.i and p_en.j == p_me.j) return Position(p_me.i    , p_me.j    );
		if (p_en.i == p_me.i and p_en.j >  p_me.j) return Position(p_me.i    , p_me.j - 1);
		                                  
		if (p_en.i > p_me.i  and p_en.j <  p_me.j) return Position(p_me.i - 1, p_me.j + 1);
		if (p_en.i > p_me.i  and p_en.j == p_me.j) return Position(p_me.i - 1, p_me.j    );
		if (p_en.i > p_me.i  and p_en.j >  p_me.j) return Position(p_me.i - 1, p_me.j - 1);
		
		return PosErr;
	}
	
	/**
	*	getCloseToPos()
	*/
	Position getCloseToPos(Position p_me, Position p_en)
	{
		if (not pos_ok(p_me) or not pos_ok(p_en)) return PosErr;
		
		if (p_en.i < p_me.i  and p_en.j <  p_me.j) return Position(p_me.i - 1, p_me.j - 1);
		if (p_en.i < p_me.i  and p_en.j == p_me.j) return Position(p_me.i - 1, p_me.j    );
		if (p_en.i < p_me.i  and p_en.j >  p_me.j) return Position(p_me.i - 1, p_me.j + 1);
		
		if (p_en.i == p_me.i and p_en.j <  p_me.j) return Position(p_me.i    , p_me.j - 1);
		if (p_en.i == p_me.i and p_en.j == p_me.j) return Position(p_me.i    , p_me.j    );
		if (p_en.i == p_me.i and p_en.j >  p_me.j) return Position(p_me.i    , p_me.j + 1);
		                                  
		if (p_en.i > p_me.i  and p_en.j <  p_me.j) return Position(p_me.i + 1, p_me.j - 1);
		if (p_en.i > p_me.i  and p_en.j == p_me.j) return Position(p_me.i + 1, p_me.j    );
		if (p_en.i > p_me.i  and p_en.j >  p_me.j) return Position(p_me.i + 1, p_me.j + 1);
		
		return PosErr;
	}
	
	/**
	*	getHeadPos()
	*/
	Position getHeadPos(int id)
	{
		Position p = data(id).pos;
		int ori = data(id).orientation;
		if (ori == SOUTH) 	return Position(p.i+2, p.j);
		if (ori == EAST) 	return Position(p.i, p.j+2);
		if (ori == NORTH) 	return Position(p.i-2, p.j);
		if (ori == WEST) 	return Position(p.i, p.j-2);
		return PosErr;
	}
	
	/**
	*	getNewHeadPos()
	*/
	Position getNewHeadPos(Position hpos, int ori, int instr)
	{
		Position newhpos = Position();
		if (instr == FORWARD1) {
			if (ori == SOUTH) 	newhpos = Position(hpos.i+1, hpos.j);
			if (ori == EAST) 	newhpos = Position(hpos.i, hpos.j+1);
			if (ori == NORTH) 	newhpos = Position(hpos.i-1, hpos.j);
			if (ori == WEST) 	newhpos = Position(hpos.i, hpos.j-1);
			return newhpos;
		}
		if (instr == FORWARD2) {
			if (ori == SOUTH) 	newhpos = Position(hpos.i+2, hpos.j);
			if (ori == EAST) 	newhpos = Position(hpos.i, hpos.j+2);
			if (ori == NORTH) 	newhpos = Position(hpos.i-2, hpos.j);
			if (ori == WEST) 	newhpos = Position(hpos.i, hpos.j-2);
			return newhpos;
		}
		if (instr == COUNTER_CLOCKWISE) {
			if (ori == SOUTH) newhpos = Position(hpos.i-2, hpos.j+2);
			if (ori == EAST)  newhpos = Position(hpos.i-2, hpos.j-2);
			if (ori == NORTH) newhpos = Position(hpos.i+2, hpos.j-2);
			if (ori == WEST)  newhpos = Position(hpos.i+2, hpos.j+2);
			return newhpos;
		}
		if (instr == CLOCKWISE) {
			if (ori == SOUTH) newhpos = Position(hpos.i-2, hpos.j-2);
			if (ori == EAST)  newhpos = Position(hpos.i+2, hpos.j-2);
			if (ori == NORTH) newhpos = Position(hpos.i+2, hpos.j+2);
			if (ori == WEST)  newhpos = Position(hpos.i-2, hpos.j+2);
			return newhpos;
		}
		return PosErr;
	}
		
	/**
	*	getInstructionHelicopter()
	*	returns the correct instruction to move (or turn towards) to position p_desired
	*/
	int getInstructionHelicopter(int id, Position hpos, const Position p_des)
	{
		if (not pos_ok(p_des) or not pos_ok(hpos)) { printError(0,"getInstructionHelicopter Error Pos not OK"); return -1; }
		Position cpos = data(id).pos;
		if (posIncludedInRange(p_des, Position(cpos.i-2,cpos.j-2), Position(cpos.i+2,cpos.j+2))) return 0; // already over p_des
		
		int ori = data(id).orientation;
		int olddist = ( abs(p_des.i - hpos.i) + abs(p_des.j - hpos.j) );
		
		int newdist;
		// try to move FORWARD2
		Position newhpos = hpos;
		
		if 		(ori == SOUTH) { newdist = ( abs(p_des.i - (hpos.i+2)) + abs(p_des.j - hpos.j) 	 ); newhpos.i = newhpos.i+2; }
		                         
		else if (ori == EAST)  { newdist = ( abs(p_des.i - hpos.i) 	+ abs(p_des.j - (hpos.j+2)) );	newhpos.j = newhpos.j+2; }
		                         
		else if (ori == NORTH) { newdist = ( abs(p_des.i - (hpos.i-2)) + abs(p_des.j - hpos.j) 	 );	newhpos.i = newhpos.i-2; }
		                         
		else if (ori == WEST)  { newdist = ( abs(p_des.i - hpos.i) 	+ abs(p_des.j - (hpos.j-2)) );	newhpos.j = newhpos.j-2; }

		if (newdist <= olddist and helicopterWillNotCrash(id, newhpos,FORWARD2)) return FORWARD2;
		else {
			return 3;
			/*
			if 		(ori == SOUTH) { newdist = ( abs(p_des.i - (hpos.i-2)) + abs(p_des.j - hpos.j+2) ); newhpos = Position(newhpos.i-2, newhpos.j+2); }
		                        
			else if (ori == EAST)  { newdist = ( abs(p_des.i - (hpos.i-2)) + abs(p_des.j - hpos.j-2) ); newhpos = Position(newhpos.i-2, newhpos.j-2); }
									
			else if (ori == NORTH) { newdist = ( abs(p_des.i - (hpos.i+2)) + abs(p_des.j - hpos.j-2) ); newhpos = Position(newhpos.i+2, newhpos.j-2); }
									
			else if (ori == WEST)  { newdist = ( abs(p_des.i - (hpos.i+2)) + abs(p_des.j - hpos.j+2) ); newhpos = Position(newhpos.i+2, newhpos.j+2); }
			
			printPos(newhpos,"newhpos_CC");
			cerr << "newdist_CC: " << newdist << endl << "CC_helicopterWillNotCrash(id, newhpos): " << helicopterWillNotCrash(id, newhpos) << endl;
			
			if (newdist <= olddist and helicopterWillNotCrash(id, newhpos)) return COUNTER_CLOCKWISE;
			else return CLOCKWISE;
			*/
		/*
			if (ori == SOUTH) {
				//if (p_des.i-2 >= p.i and p_des.j == p.j) return FORWARD2;
				//if (p_des.i-1 >= p.i and p_des.j == p.j) return FORWARD1;
				if (p_des.j   >= cpos.j and p_des.i == cpos.i) return COUNTER_CLOCKWISE;
				else return CLOCKWISE;
			}
			
			if (ori == EAST) 	{ if (p_des.i   < cpos.i) return COUNTER_CLOCKWISE; else return CLOCKWISE; }
			                                                                                                             
			if (ori == NORTH) 	{ if (p_des.j   <= cpos.j) return COUNTER_CLOCKWISE; else return CLOCKWISE; }
			                                             
			if (ori == WEST) 	{ if (p_des.i   >= cpos.i) return COUNTER_CLOCKWISE; else return CLOCKWISE; }
			*/
		}
		
		return 0;
	}
	
	/**
	*	helicopterWillNotCrash()
	*	This function returns true if a head_position of the helicopter does fit in the new spot when FORWARD2'ing
	*/
	bool helicopterWillNotCrash(int id, Position new_h_pos, int instr)
	{
		bool z = false;
		int ori = data(id).orientation;
		Position hpos = new_h_pos;
		
		if (instr == FORWARD2) {
			if (ori == SOUTH) {
				if ((not isAccessiblePos(Position(hpos.i-1,hpos.j),2)) or (not isAccessiblePos(Position(hpos.i-1,hpos.j-2),2)) or (not isAccessiblePos(Position(hpos.i-1,hpos.j+2),2)) or (not isAccessiblePos(Position(hpos.i,hpos.j),2))) return false;
						
				z = ((isPosEmptyOfThisUnitType(Position(hpos.i, hpos.j-2), HELICOPTER, id))
				and (isPosEmptyOfThisUnitType(Position(hpos.i, hpos.j+2), HELICOPTER, id)));
				return z;
			}
			if (ori == EAST) {
				if ((not isAccessiblePos(Position(hpos.i,hpos.j-1),2)) or (not isAccessiblePos(Position(hpos.i-2,hpos.j-1),2)) or (not isAccessiblePos(Position(hpos.i+2,hpos.j-1),2)) or (not isAccessiblePos(Position(hpos.i,hpos.j),2))) return false;
				
				z = ((isPosEmptyOfThisUnitType(Position(hpos.i-2, hpos.j), HELICOPTER, id))
				and (isPosEmptyOfThisUnitType(Position(hpos.i+2, hpos.j), HELICOPTER, id)));
				return z;
			}
			if (ori == NORTH) {
				if ((not isAccessiblePos(Position(hpos.i+1,hpos.j),2)) or (not isAccessiblePos(Position(hpos.i+1,hpos.j-2),2)) or (not isAccessiblePos(Position(hpos.i+1,hpos.j+2),2)) or (not isAccessiblePos(Position(hpos.i,hpos.j),2))) return false;
			
				z = ((isPosEmptyOfThisUnitType(Position(hpos.i, hpos.j-2), HELICOPTER, id))
				and (isPosEmptyOfThisUnitType(Position(hpos.i, hpos.j+2), HELICOPTER, id)));
				return z;
			}
			if (ori == WEST) {
				if ((not isAccessiblePos(Position(hpos.i,hpos.j+1),2)) or (not isAccessiblePos(Position(hpos.i-2,hpos.j+1),2)) or (not isAccessiblePos(Position(hpos.i+2,hpos.j+1),2)) or (not isAccessiblePos(Position(hpos.i,hpos.j),2))) return false;
				
				z = ((isPosEmptyOfThisUnitType(Position(hpos.i-2, hpos.j), HELICOPTER, id))
				and (isPosEmptyOfThisUnitType(Position(hpos.i+2, hpos.j), HELICOPTER, id)));
				return z;
			}
			return z;
		}
		if (instr == FORWARD1) {
			if (ori == SOUTH) {
				z = ((isAccessiblePos(Position(hpos.i,hpos.j),2))
				and (isPosEmptyOfThisUnitType(Position(hpos.i, hpos.j-2), HELICOPTER, id))
				and (isPosEmptyOfThisUnitType(Position(hpos.i, hpos.j+2), HELICOPTER, id)));
				return z;
			}
			if (ori == EAST) {
				z = ((isAccessiblePos(Position(hpos.i,hpos.j),2))
				and (isPosEmptyOfThisUnitType(Position(hpos.i-2, hpos.j), HELICOPTER, id))
				and (isPosEmptyOfThisUnitType(Position(hpos.i+2, hpos.j), HELICOPTER, id)));
				return z;
			}
			if (ori == NORTH) {
				z = ((isAccessiblePos(Position(hpos.i,hpos.j),2))
				and (isPosEmptyOfThisUnitType(Position(hpos.i, hpos.j-2), HELICOPTER, id))
				and (isPosEmptyOfThisUnitType(Position(hpos.i, hpos.j+2), HELICOPTER, id)));
				return z;
			}
			if (ori == WEST) {
				z = ((isAccessiblePos(Position(hpos.i,hpos.j),2))
				and (isPosEmptyOfThisUnitType(Position(hpos.i-2, hpos.j), HELICOPTER, id))
				and (isPosEmptyOfThisUnitType(Position(hpos.i+2, hpos.j), HELICOPTER, id)));
				return z;
			}
			return z;
		}
		return z;
		
	}
	
	/**
	*	posInHelicopterRange()
	*/
	bool posInHelicopterRange(Position pos_analyzed, int focus_on, int& aux)
	{
		if (not pos_ok(pos_analyzed)) { printError(0,"posInHelicopterRange --> ERROR!"); return false; }
		
		if (focus_on == MINE) {
			vector<int> mine_h = helicopters(me());
			for (unsigned int i = 0; i < mine_h.size(); ++i) {	// Analyze 1 helicopter
				Position ph = data(mine_h[i]).pos;				// Centered position of helicopter
				Position ph_ini = Position(ph.i-2, ph.j-2);
				Position ph_end = Position(ph.i+2, ph.j+2);
				// Is pos_analyzed inside helicopter 'i' ¿?
				if (posIncludedInRange(pos_analyzed, ph_ini, ph_end)) return true;
			}
			return false;
		}
		
		if (focus_on == ENEMY) {
			for (int pl = 0; pl < nb_players(); ++pl) {
				if (pl == me()) continue;							// Do NOT analyze my helicopters
				vector<int> h_pl = helicopters(pl);
				for (unsigned int i = 0; i < h_pl.size(); ++i) {	// Analyze 1 helicopter
					Position ph = data(h_pl[i]).pos;				// Centered position of helicopter
					Position ph_ini = Position(ph.i-2, ph.j-2);
					Position ph_end = Position(ph.i+2, ph.j+2);
					if (posIncludedInRange(pos_analyzed, ph_ini, ph_end)) {
						aux = data(h_pl[i]).napalm;
						// cerr << "aux " << aux << endl;
						return true;
					}
				}
			}
		}
		
		if (focus_on == OTHERS) {
			for (int pl = 0; pl < nb_players(); ++pl) {
				vector<int> h_pl = helicopters(pl);
				for (unsigned int i = 0; i < h_pl.size(); ++i) {	// Analyze 1 helicopter
					if (pl == me() and h_pl[i] == aux) continue;	// Do NOT analyze my helicopter with id = 'my_id'
					Position ph = data(h_pl[i]).pos;				// Centered position of helicopter
					Position ph_ini = Position(ph.i-2, ph.j-2);
					Position ph_end = Position(ph.i+2, ph.j+2);
					if (posIncludedInRange(pos_analyzed, ph_ini, ph_end)) return true;
				}
			}
		}
		
		else if (focus_on == ALL) {
			for (int pl = 0; pl < nb_players(); ++pl) {
				vector<int> h_pl = helicopters(pl);
				for (unsigned int i = 0; i < h_pl.size(); ++i) {	// Analyze 1 helicopter
					Position ph = data(h_pl[i]).pos;				// Centered position of helicopter
					Position ph_ini = Position(ph.i-2, ph.j-2);
					Position ph_end = Position(ph.i+2, ph.j+2);
					if (posIncludedInRange(pos_analyzed, ph_ini, ph_end)) return true;
				}
			}
		}
		
		return false;
	}
	
	/**
	*	throwParachuter()
	*/
	bool throwParachuter(int helicopter_id, Position p)
	{
		// cerr << "size: " << data(helicopter_id).parachuters.empty() << endl;
		if (data(helicopter_id).parachuters.empty()) return false; // no parachuters available to jump
		if (isEqualPos(PosErr, p) or not pos_ok(p)) {
			Position cpos = data(helicopter_id).pos;
			for (int i = -2; i <= 2; ++i) {
				for (int j = -2; j <= 2; ++j) {
					if(isPosEmptyOfThisUnitType(Position(cpos.i+i, cpos.j+j), SOLDIER,0) and not willProbablyBurnNextRound(Position(cpos.i+i, cpos.j+j))) {
						command_parachuter(cpos.i+i, cpos.j+j);
						return true;
					}
				}
			}
		}
		else {
			if (pos_ok(p) and isAccessiblePos(p, SOLDIER)) {
				command_parachuter(p.i, p.j);
				return true;
			}
		}
		return false;
	}
	
	/**
	*	removeFromVectorIDSoldiersInType()
	*/
	void removeFromVectorIDSoldiersInType(vector<int> &v, int type)
	{
		Position p = Position();
		for (unsigned int i = 0; i < v.size(); ++i) {
			if (data(v[i]).id <= (nb_players()*NUM_HELICOPTERS)) { v.erase(v.begin() + i); --i; }	// ID is not of a Soldier
			p = data(v[i]).pos;
			if (what(p.i, p.j) == type) { v.erase(v.begin() + i); --i; }
		}
	}
	
	/**
	*	weakestEnemyAround()
	*/
	int weakestEnemyAround(const vector<int> &id_enemies, int id)
	{
		if (not pos_ok(data(id).pos) or id_enemies.empty()) return -1;
		
		// Check which adj_position has a valid-id, lower-health and preferably-in-GRASS enemy
		vector<int> v = id_enemies;
			
		int first_soldier_id = (NUM_HELICOPTERS * nb_players()) + 1;	//  9
				
		for (unsigned int i = 0; i < v.size(); ++i) {
			if (v[i] < first_soldier_id) { v.erase(v.begin() + i); --i; }	// Erase and keep the order logic in the vector
		}// Helicopters 'id' erased (just in the strange case there were some)
		
		vector<int> aux1 = v;
		removeFromVectorIDSoldiersInType(v, FOREST);
		// enemy preferable in GRASS since, damage(@GRASS) > damage(@FOREST)

		if (v.empty()) v = aux1; 	// Al enemies are in FOREST (therefore 'v' was empty-ed)
		
		int chosen_enemy_id = 0, min_health = LIFE+1;
		
		// Pick enemy with the lowest health points
		for (unsigned int i = 0; i < v.size(); ++i) {
			if (data(v[i]).life <= min_health) {
				chosen_enemy_id = v[i];
				min_health = data(v[i]).life;
			}
		}
		
		if (chosen_enemy_id > 0) return chosen_enemy_id;
		else return -1;	// Error
	}
		
	/**
	*	strongestEnemyAround()
	*/
	int strongestEnemyAround(const vector<int> &id_enemies, Position p)
	{
		if (not pos_ok(p) or id_enemies.empty()) return -1;
		
		// Check which adj_position has a valid-id, lower-health and preferably-in-GRASS enemy
		vector<int> v = id_enemies;
			
		int first_soldier_id = (NUM_HELICOPTERS * nb_players()) + 1;	//  9
				
		for (unsigned int i = 0; i < v.size(); ++i) {
			if (v[i] < first_soldier_id) { v.erase(v.begin() + i); --i; }	// Erase and keep the order logic in the vector
		}// Helicopters 'id' erased (just in the strange case there were some)
		
		int chosen_enemy_id = 0, max_health = 0;
		
		// Pick enemy with the lowest health points
		for (unsigned int i = 0; i < v.size(); ++i) {
			if (data(v[i]).life >= max_health) {
				chosen_enemy_id = v[i];
				max_health = data(v[i]).life;
			}
		}
		
		if (chosen_enemy_id > 0) return chosen_enemy_id;
		else return -1;	// Error
	}
	
	/**
	*	isThereABuddySoldier()
	*/
	bool isThereABuddySoldier(Position p)
	{
		vector<int> soldiers = State::soldiers(me());
		for (unsigned int i = 0; i < soldiers.size(); ++i) {
			if (soldiers[i] == which_soldier(p.i, p.j)) return true;	// There is a Buddy Soldier at position 'p'
		}
		return false;
	}
	
	/**
	*	isThereAnEnemyAt()
	*/
	bool isThereAnEnemyAt(Position p, int unit_type)
	{
		if (not validUnitType(unit_type) or not pos_ok(p)) { printError(0,"isThereAnEnemyAt ---> ERROR"); return false; }
		
		if (unit_type == SOLDIER) {
			int id = which_soldier(p.i, p.j);
			if (id > 0 and data(id).player != me()) return true;
		}
		else if (unit_type == HELICOPTER) {
			int id = which_helicopter(p.i, p.j);
			if (id > 0) {
				// There's an helicopter centered at 'p'
				if (data(id).player != me()) return true; // and it's not mine
			}
			else {
				if (posInHelicopterRange(p, ENEMY, zero)) return true;
			}
		}
		return false;
	}

	/**
	*	getEnemiesIDsInNearArea()
	*/
	vector<int> getEnemiesIDsInNearArea(Position p, int unit_type)
	{
	/*
		vector<int> id_enemies;
		if (not pos_ok(p) or not validUnitType(unit_type)) { printError(1, ""); return id_enemies; }
		
		Position adj_pos = Position();
		int e_id = 0;
		for (int k = 0; k < 8; ++k) {
			adj_pos = sumPosPos(p, displ[k]);
			if (isAccessiblePos(adj_pos, unit_type) and isThereAnEnemyAt(adj_pos, unit_type)) {
				if (unit_type == SOLDIER) e_id = which_soldier(adj_pos.i, adj_pos.j);	// Store soldier _id of (adj_pos)
				if (unit_type == HELICOPTER) e_id = getIdOfHelicopterAt(adj_pos);
				// Store helicopter_id of the helicopter ranged in 5x(adj_pos)x5 only if I haven't stored yet (it can be adjacent to more than one position)

				if (not elemInVector(id_enemies,e_id)) id_enemies.push_back(e_id);
			}
		}
		return id_enemies;
		
		
	*/
		
		vector<int> id_enemies;
		if (not pos_ok(p) or not validUnitType(unit_type)) { printError(0, "getEnemiesIDsInNearArea --> POS NOT OK!!!!"); return id_enemies; }
		
		Position adj_pos = Position();
		int e_id = 0;
		for (int i = p.i - ENEMY_RANGE; i <= p.i + ENEMY_RANGE; ++i) {
		    for (int j = p.j - ENEMY_RANGE; j <= p.j + ENEMY_RANGE; ++j) {
		        Position p_analyzed = Position(i, j);
		        if (isAccessiblePos(p_analyzed, unit_type) and isThereAnEnemyAt(p_analyzed, unit_type)) {
		            if (unit_type == SOLDIER) e_id = which_soldier(p_analyzed.i, p_analyzed.j);	// Store soldier_id of (adj_pos)
		           	if (unit_type == HELICOPTER) e_id = getIdOfHelicopterAt(adj_pos);
				    // Store helicopter_id of the helicopter ranged in 5x(adj_pos)x5 only if I haven't stored yet (it can be adjacent to more than one position)

				    if (not elemInVector(id_enemies,e_id)) id_enemies.push_back(e_id);
		        }
		    }
		}
		return id_enemies;
		
	}
	
	/**
	*	canAttack()
	*/
	bool canAttack(int my_id, int en_id)
	{
		bool attack = false;
		attack = ((random(1,ATTACK_MAX_PROBABILITY_ALL) == 1) and (data(my_id).life >= ATTACK_MIN_HEALTH_ME_ALL) and (data(en_id).life <= ATTACK_MAX_HEALTH_ENEMY_ALL));
		
		Position my_p = data(my_id).pos;
		Position en_p = data(en_id).pos;
		if (what(my_p.i, my_p.j) == GRASS and what(en_p.i, en_p.j) == FOREST) attack = false;
		
		return attack;
		
		/*
		int e_id = which_soldier(p_attacked.i, p_attacked.j);
		if (e_id == -1) { printError(2,""); return false; }
		if (e_id == 0) { printError(5,""); return false; }
		int biome = what(data(e_id).pos.i, data(e_id).pos.j);
		
		if (biome == GRASS) {
			conditions_to_attack = (random(1,ATTACK_MAX_PROBABILITY_GRASS) == 1 and data(my_id).life > ATTACK_MIN_HEALTH_ME_GRASS and data(e_id).life < ATTACK_MAX_HEALTH_ENEMY_GRASS);
		}
		if (biome == FOREST) {
			conditions_to_attack = (random(1,ATTACK_MAX_PROBABILITY_FOREST) == 1 and data(my_id).life > ATTACK_MIN_HEALTH_ME_FOREST and data(e_id).life < ATTACK_MAX_HEALTH_ENEMY_FOREST);
		}
		*/
	}
	
	/**
	* napalmConditions()
	*/
	bool napalmConditions(int h_id)
	{
		if (data(h_id).napalm == 0)
		{
			Position p_ini = Position(data(h_id).pos.i-2, data(h_id).pos.j-2);
			Position p_end = Position(data(h_id).pos.i+2, data(h_id).pos.j+2);
			vector<int> sd = Player::soldiers(me());
			int buddy_counter = 0;
			for (int i = 0; i < (int)sd.size(); ++i) {
				if (posIncludedInRange(data(sd[i]).pos, p_ini, p_end)) {
					++buddy_counter;
					if (buddy_counter > MAX_BUDDIES_UNDER_NAPALM) return false; // I'm over buddy soldier
				}
			}
    
			int enemy_counter = 0;
			for (int pl = 0; pl < NUM_PLAYERS; ++pl) {
				if (pl == me()) continue;
				vector<int> sden = Player::soldiers(pl);
				for (int i = 0; i < (int)sden.size(); ++i) {
					if (posIncludedInRange(data(sden[i]).pos, p_ini, p_end)) {
						++enemy_counter; 	// another enemy under helicopter
						if (enemy_counter >= MIN_ENEMIES_UNDER_NAPALM) return true;
					}
				}
			}
		}
		return false;
	}
	
	/**
	*	noMmountainsNear()
	*/
	bool noMountainsNear(Position hpos, int ori)
	{
		// check if mountain in front of me
		int i = hpos.i;
		int j = hpos.j;
		
		bool z = false;
		if (ori == SOUTH) {
			z = ((isAccessiblePos(Position(i+1,j), 2))  and (isAccessiblePos(Position(i+1,j-1), 2)) and (isAccessiblePos(Position(i+1,j-2), 2)) and
				(isAccessiblePos(Position(i+1,j+1), 2)) and (isAccessiblePos(Position(i+1,j+2), 2))
				and
				(isAccessiblePos(Position(i+2,j), 2))   and (isAccessiblePos(Position(i+2,j-1), 2)) and (isAccessiblePos(Position(i+2,j-2), 2)) and
				(isAccessiblePos(Position(i+2,j+1), 2)) and (isAccessiblePos(Position(i+2,j+2), 2)));
			return z;
		}
		if (ori == EAST) {
			z = ((isAccessiblePos(Position(i,j+1), 2))  and (isAccessiblePos(Position(i-1,j+1), 2)) and (isAccessiblePos(Position(i-2,j+1), 2)) and
				(isAccessiblePos(Position(i+1,j+1), 2)) and (isAccessiblePos(Position(i+2,j+1), 2))
				and
				(isAccessiblePos(Position(i,j+2), 2))   and (isAccessiblePos(Position(i-1,j+2), 2)) and (isAccessiblePos(Position(i-2,j+2), 2)) and
				(isAccessiblePos(Position(i+1,j+2), 2)) and (isAccessiblePos(Position(i+2,j+2), 2)));
			return z;
		}
		if (ori == NORTH) {
			z = ((isAccessiblePos(Position(i-1,j), 2))  and (isAccessiblePos(Position(i-1,j-1), 2)) and (isAccessiblePos(Position(i-1,j-2), 2)) and
				(isAccessiblePos(Position(i-1,j+1), 2)) and (isAccessiblePos(Position(i-1,j+2), 2))
				and
				(isAccessiblePos(Position(i-2,j), 2))   and (isAccessiblePos(Position(i-2,j-1), 2)) and (isAccessiblePos(Position(i-2,j-2), 2)) and
				(isAccessiblePos(Position(i-2,j+1), 2)) and (isAccessiblePos(Position(i-2,j+2), 2)));
			return z;
		}
		if (ori == WEST) {
			z = ((isAccessiblePos(Position(i,j-1), 2))  and (isAccessiblePos(Position(i-1,j-1), 2)) and (isAccessiblePos(Position(i-2,j-1), 2)) and
				(isAccessiblePos(Position(i+1,j-1), 2)) and (isAccessiblePos(Position(i+2,j-1), 2))
				and
				(isAccessiblePos(Position(i,j-2), 2))   and (isAccessiblePos(Position(i-1,j-2), 2)) and (isAccessiblePos(Position(i-2,j-2), 2)) and
				(isAccessiblePos(Position(i+1,j-2), 2)) and (isAccessiblePos(Position(i+2,j-2), 2)));
			return z;
		}
		return z;
	}
	
	/**
	*	spaciousTurn()
	*	// check if a 4-ahead x5 space is available in front of hpos
	*/
	bool spaciousTurn(Position hpos, int ori)
	{
		int i = hpos.i;
		int j = hpos.j;
		
		bool z = false;
		if (ori == SOUTH) {
			z = ((isAccessiblePos(Position(i+1,j-2), 2)) and (isAccessiblePos(Position(i+1,j+2), 2))
			and  (isAccessiblePos(Position(i+2,j-2), 2)) and (isAccessiblePos(Position(i+2,j+2), 2))
			and  (isAccessiblePos(Position(i+3,j-2), 2)) and (isAccessiblePos(Position(i+3,j+2), 2))
			and  (isAccessiblePos(Position(i+4,j-2), 2)) and (isAccessiblePos(Position(i+4,j+2), 2))
			and  (isAccessiblePos(Position(i+4,j-1), 2)) and (isAccessiblePos(Position(i+4,j+1), 2)) and (isAccessiblePos(Position(i+4,j), 2)));
			return z;
		}
		if (ori == EAST) {
			z = ((isAccessiblePos(Position(i-2,j+1), 2)) and (isAccessiblePos(Position(i+2,j+1), 2))
			and  (isAccessiblePos(Position(i-2,j+2), 2)) and (isAccessiblePos(Position(i+2,j+2), 2))
			and  (isAccessiblePos(Position(i-2,j+3), 2)) and (isAccessiblePos(Position(i+2,j+3), 2))
			and  (isAccessiblePos(Position(i-2,j+4), 2)) and (isAccessiblePos(Position(i+2,j+4), 2))
			and  (isAccessiblePos(Position(i-1,j+4), 2)) and (isAccessiblePos(Position(i+1,j+4), 2)) and (isAccessiblePos(Position(i,j+4), 2)));
			return z;
		}
		if (ori == NORTH) {
			z = ((isAccessiblePos(Position(i-1,j-2), 2)) and (isAccessiblePos(Position(i-1,j+2), 2))
			and  (isAccessiblePos(Position(i-2,j-2), 2)) and (isAccessiblePos(Position(i-2,j+2), 2))
			and  (isAccessiblePos(Position(i-3,j-2), 2)) and (isAccessiblePos(Position(i-3,j+2), 2))
			and  (isAccessiblePos(Position(i-4,j-2), 2)) and (isAccessiblePos(Position(i-4,j+2), 2))
			and  (isAccessiblePos(Position(i-4,j-1), 2)) and (isAccessiblePos(Position(i-4,j+1), 2)) and (isAccessiblePos(Position(i-4,j), 2)));
			return z;
		}
		if (ori == WEST) {
			z = ((isAccessiblePos(Position(i-2,j-1), 2)) and (isAccessiblePos(Position(i+2,j-1), 2))
			and  (isAccessiblePos(Position(i-2,j-2), 2)) and (isAccessiblePos(Position(i+2,j-2), 2))
			and  (isAccessiblePos(Position(i-2,j-3), 2)) and (isAccessiblePos(Position(i+2,j-3), 2))
			and  (isAccessiblePos(Position(i-2,j-4), 2)) and (isAccessiblePos(Position(i+2,j-4), 2))
			and  (isAccessiblePos(Position(i-1,j-4), 2)) and (isAccessiblePos(Position(i+1,j-4), 2)) and (isAccessiblePos(Position(i,j-4), 2)));
			return z;
		}
		return z;
	}
	
	/**
	*	nearestEnemyPost()
	*/
	Position nearestEnemyPost(int id)
	{
		Position pos = data(id).pos;
		Position nEP = Position(-1,-1);
		vector<Post> posts = State::posts();
		int i, j, maxdist = MAX*MAX;
		for (int k = 0; k < (int)posts.size(); ++k) {
			if (/*posts[k].player < 0 or*/ posts[k].player == me()) continue;
			i = abs( posts[k].pos.i - pos.i );
			j = abs( posts[k].pos.j - pos.j );
			if ((i+j) < maxdist) {
				maxdist = (i+j);
				nEP = posts[k].pos;
			}
		}
		return nEP;
	}
	
	/**
	*	BFS_DIR_K_SOLDIER()
	*	This function complements the BFS. It iterates through the 8 possible positions in each BFS step
	*/
	bool bfs_dir_k_soldier(Mat_bool &visited, Position &new_pos, queue<Position> &Q, queue<int> &dQ, int &dist)
	{
		if (pos_ok(new_pos) and not visited[new_pos.i][new_pos.j] and isAccessiblePos(new_pos, SOLDIER) and fire_time(new_pos.i, new_pos.j) < dist) {
			if (isPost(new_pos) and  post_owner(new_pos.i, new_pos.j) != me()) return true; // check if I moved to a POST and I do NOT own it
			Q.push(new_pos);
			dQ.push(dist);
			visited[new_pos.i][new_pos.j] = true;
		}
		return false;
	}
	
	/**
	*
	*/
	bool bfs_dir_k_soldier_forest(Mat_bool &visited, Position &new_pos, queue<Position> &Q, queue<int> &dQ, int &dist)
	{
		if (pos_ok(new_pos) and not visited[new_pos.i][new_pos.j] and isAccessiblePos(new_pos, SOLDIER) and what(new_pos.i, new_pos.j) == FOREST and fire_time(new_pos.i, new_pos.j) < dist) {
			if (isPost(new_pos) and  post_owner(new_pos.i, new_pos.j) != me()) return true; // check if I moved to a POST and I do NOT own it
			Q.push(new_pos);
			dQ.push(dist);
			visited[new_pos.i][new_pos.j] = true;
		}
		return false;
	}
	
	/**
	*
	*/
	bool bfs_dir_k_soldier_low_post(Mat_bool &visited, Position &new_pos, queue<Position> &Q, queue<int> &dQ, int &dist)
	{
		if (pos_ok(new_pos) and not visited[new_pos.i][new_pos.j] and isAccessiblePos(new_pos, SOLDIER) and fire_time(new_pos.i, new_pos.j) < dist) {
			if (isPost(new_pos) and  post_owner(new_pos.i, new_pos.j) != me() and post_value(new_pos.i, new_pos.j) == LOW_VALUE) return true;
			Q.push(new_pos);
			dQ.push(dist);
			visited[new_pos.i][new_pos.j] = true;
		}
		return false;
	}
	
	/**
	*
	*/
	bool bfs_dir_k_soldier_high_post(Mat_bool &visited, Position &new_pos, queue<Position> &Q, queue<int> &dQ, int &dist)
	{
		if (pos_ok(new_pos) and not visited[new_pos.i][new_pos.j] and isAccessiblePos(new_pos, SOLDIER) and fire_time(new_pos.i, new_pos.j) < dist) {
			if (isPost(new_pos) and  post_owner(new_pos.i, new_pos.j) != me() and post_value(new_pos.i, new_pos.j) == HIGH_VALUE) return true;
			Q.push(new_pos);
			dQ.push(dist);
			visited[new_pos.i][new_pos.j] = true;
		}
		return false;
	}
	
	/**
	*	bfs_dir_k_soldier_enemy()
	*/
	bool bfs_dir_k_soldier_enemy(Mat_bool &visited, Position &new_pos, queue<Position> &Q, queue<int> &dQ, int &dist)
	{
		if (pos_ok(new_pos) and not visited[new_pos.i][new_pos.j] and isAccessiblePos(new_pos, SOLDIER) and fire_time(new_pos.i, new_pos.j) < dist) {
			if ((isThereAnEnemyAt(new_pos, SOLDIER)) and (data(which_soldier(new_pos.i, new_pos.j)).life <= ATTACK_MAX_HEALTH_ENEMY_ALL)) return true;
			Q.push(new_pos);
			dQ.push(dist);
			visited[new_pos.i][new_pos.j] = true;
		}
		return false;
	}
	
	/**
	*	bfs_dir_k_soldier_buddy()
	*/
	bool bfs_dir_k_soldier_buddy(Mat_bool &visited, Position &new_pos, queue<Position> &Q, queue<int> &dQ, int &dist)
	{
		if (pos_ok(new_pos) and not visited[new_pos.i][new_pos.j] and isAccessiblePos(new_pos, SOLDIER) and fire_time(new_pos.i, new_pos.j) < dist) {
			vector<int> S = soldiers(me());
			if (which_soldier(new_pos.i, new_pos.j) == 33) return true;
			Q.push(new_pos);
			dQ.push(dist);
			visited[new_pos.i][new_pos.j] = true;
		}
		return false;
	}
		
	/**
	*	bfs_dir_k_soldier_position()
	*/
	bool bfs_dir_k_soldier_position(Mat_bool &visited, Position &new_pos, queue<Position> &Q, queue<int> &dQ, int &dist, Position p_desired)
	{
		if (pos_ok(new_pos) and not visited[new_pos.i][new_pos.j] and isAccessiblePos(new_pos, SOLDIER) and fire_time(new_pos.i, new_pos.j) < dist) {
			if (isEqualPos(new_pos,p_desired)) return true;
			Q.push(new_pos);
			dQ.push(dist);
			visited[new_pos.i][new_pos.j] = true;
		}
		return false;
	}
	
	/**
	*	CHECK_ADJ_POS_SOLDIER()
	*	This function is the BFS main body. It calls bfs_dir_k() for each possible position	in each case
	*/
	int check_adj_pos_soldier(Position pos, const int mode, Position p_des)
	{
		if (mode == BFS_S_NORMAL and (isPost(pos)) and (post_owner(pos.i, pos.j) != me())) return 0;
		if (mode == BFS_S_FOREST and (isPost(pos)) and (post_owner(pos.i, pos.j) != me()) and (what(pos.i, pos.j) == FOREST)) return 0;
		if (mode == BFS_S_LOW_VALUE)    {};
		if (mode == BFS_S_HIGH_VALUE)	{};
		if (mode == BFS_S_ENEMIES)		{};
		if (mode == BFS_S_BUDDIES)		{};
		if (mode == BFS_S_POSITION and isEqualPos(pos, p_des)) return 0;
		
		Mat_bool visited(MAX, vector<bool>(MAX, false));	// matrix for visited positions
		queue<Position> Q;									// vertexes, here 'Positions'
		queue<int> dQ;										// distance queue
		
		Q.push(pos);
		dQ.push(0);
		visited[pos.i][pos.j] = true;
		
		Position new_pos = Position();			// new_pos <- pos + displ[k] {checked adjacent position}
		int dist;
		
		while (not Q.empty())
		{
			Position current_pos = Q.front();
			dist = dQ.front() + 1;
			
			for (int k = 0; k < 8; ++k) {		// check the eight adjacent positions respectively to the current analyzed position 'pos'
				new_pos = sumPosPos(current_pos,displ[k]);
				bool ret_dist = false;
            	if (mode == BFS_S_NORMAL)		ret_dist = bfs_dir_k_soldier			(visited,new_pos,Q,dQ,dist);            // calculate BFS for this direction
				if (mode == BFS_S_FOREST)		ret_dist = bfs_dir_k_soldier_forest 	(visited,new_pos,Q,dQ,dist);            // calculate BFS for this direction
				if (mode == BFS_S_LOW_VALUE)	ret_dist = bfs_dir_k_soldier_low_post 	(visited,new_pos,Q,dQ,dist);
				if (mode == BFS_S_HIGH_VALUE)	ret_dist = bfs_dir_k_soldier_high_post 	(visited,new_pos,Q,dQ,dist);
				if (mode == BFS_S_ENEMIES)		ret_dist = bfs_dir_k_soldier_enemy 		(visited,new_pos,Q,dQ,dist);
				if (mode == BFS_S_BUDDIES)		ret_dist = bfs_dir_k_soldier_buddy 		(visited,new_pos,Q,dQ,dist);
				if (mode == BFS_S_POSITION)		ret_dist = bfs_dir_k_soldier_position	(visited,new_pos,Q,dQ,dist, p_des);
				
				if (ret_dist) return dist;
			}
			
			Q.pop();
			dQ.pop();
		}
		
		return -1;	//NOT FOUND!
	}
	
	/**
	*	BFS_DIR_K_HELICOPTER()
	*	This function complements the BFS. It iterates through the 8 possible positions in each BFS step
	*/
	bool bfs_dir_k_helicopter(Mat_bool &visited, Position &new_pos, queue<Position> &Q, queue<int> &dQ, int &dist)
	{
		if (pos_ok(new_pos) and not visited[new_pos.i][new_pos.j] and isAccessiblePos(new_pos,HELICOPTER)) {
			if ((isPost(new_pos)) and (post_owner(new_pos.i, new_pos.j) != me())) return true; // check if I moved to a POST and I do NOT own i
			Q.push(new_pos);
			dQ.push(dist);
			visited[new_pos.i][new_pos.j] = true;
		}
		return false;
	}
	
	/**
	*	FIND_NEAREST_ENEMY_POST_HELICOPTER()
	*	This function is the BFS for helicopters each possible position	to move
	*	cpos is the centered position of an helicopter
	*/
	int find_nearest_enemy_post_helicopter(Position hpos, int ori)
	{
		Position cpos = getCPosFromHPos(hpos,ori);
		Position ppost = getPosOfPostUnderHelicopter(cpos);		// ppost is the Pos of the post under the helicopter centered at 'cpos'
		if (not isEqualPos(ppost,PosErr) and (post_owner(ppost.i, ppost.j)) != me()) return 0;
		
		Mat_bool visited(MAX, vector<bool>(MAX, false));	// matrix for visited positions
		queue<Position> Q;									// vertexes, here 'Positions'
		queue<int> dQ;										// distance queue
		
		Q.push(hpos);
		dQ.push(0);
		visited[hpos.i][hpos.j] = true;
		
		Position new_pos = Position();			// new_pos <- hpos + displ[k] {checked adjacent position}
		
		while (not Q.empty())
		{
			Position current_pos = Q.front();
			int dist = dQ.front() + 1;
			
			for (int k = 0; k < 8; k = k + 2) {		// check the four adjacent positions respectively to the current analyzed position 'pos'
				new_pos = sumPosPos(current_pos,displ[k]);
				bool ret_dist = bfs_dir_k_helicopter(visited,new_pos,Q,dQ,dist); 	// calculate BFS for this direction
				if (ret_dist) return dist;
			}
			Q.pop();
			dQ.pop();
		}
		
		return -1;	//NOT FOUND!
	}
	
	/**
	*	PLAY_SOLDIER
	*/
	void play_soldier(int id, bool second_half_of_soldiers)
	{
		Position pos = data(id).pos;   				// Current position of soldier 'id'
		Position adj_pos = Position();
		Position chosen_new_pos = Position(-1,-1);	// Position that the soldier 'id' will be commanded to
		
		vector<int> vec_id_enemies = getEnemiesIDsInNearArea(pos, SOLDIER);		// Get the adjacent enemies if I want to attack
		int strongest_enenmy_id = -1;
		int   weakest_enenmy_id = -1;
		
		cerr << "\nSoldier: " << id << " "; printPos(pos,"@Pos");
		printVec(vec_id_enemies,"vec_id_enemies");
		
		if (not vec_id_enemies.empty()) {
			strongest_enenmy_id = strongestEnemyAround(vec_id_enemies, pos);
			weakest_enenmy_id	= 	weakestEnemyAround(vec_id_enemies, id);
		}
		
		cerr << "sEID: " << strongest_enenmy_id << " wEID: " << weakest_enenmy_id << endl;
		
		bool attack = (weakest_enenmy_id > 0 and canAttack(id, weakest_enenmy_id));
		bool escape = (strongest_enenmy_id > 0 and data(strongest_enenmy_id).life >= data(id).life + 25);
			
		int min_dist = MAX*MAX, max_dist = -1;
		bool found_a_forest_pos = false;
		
		int k; if (second_half_of_soldiers) k = 7;	else k = 0;
			
		// check the 8 adjacent positions respectively to the current pos 'pos'. Check which adjacent position is closer to a post (BFS)
		while (k >= 0 and k <= 7)
		{
			adj_pos = sumPosPos(pos, displ[k]); // check that adj_pos is a valid and accessible one and there's not a Buddy Soldier
			int current_pos_dist = -1;
			int enemy_h_namapl = ROUNDS_NAPALM+1;	// if it is '31' no enemy heli
			bool enemy_heli = posInHelicopterRange(adj_pos, ENEMY, enemy_h_namapl);
			
			// cerr << "Iteration k = " << k << " "; printPos(adj_pos,"adj_pos");
							
			if (attack or escape)
			{
				if (attack and escape)
				{
					if (pos1FurtherThanPos2(adj_pos, data(weakest_enenmy_id).pos, data(strongest_enenmy_id).pos)) {	// Stronger closer -> escape
						if (isAccessiblePos(adj_pos, SOLDIER) and isPosEmptyOfThisUnitType(adj_pos, SOLDIER, 0) and not inFire(adj_pos) and not willProbablyBurnNextRound(adj_pos) and enemy_h_namapl > 4) {
							current_pos_dist = check_adj_pos_soldier(adj_pos, BFS_S_POSITION, data(strongest_enenmy_id).pos);
							if (current_pos_dist >= 0 and current_pos_dist > max_dist) { max_dist = current_pos_dist; chosen_new_pos = adj_pos; }
							// cerr << "IF escape -> current_pos_dist: " << current_pos_dist << " "; printPos(chosen_new_pos, "chosen_new_pos");
						}
					}
					else {																							// Weaker closer -> attack
						if (isAccessiblePos(adj_pos, SOLDIER) and isPosEmptyOfThisUnitType(adj_pos, SOLDIER, 0) and not inFire(adj_pos) and not willProbablyBurnNextRound(adj_pos) and enemy_h_namapl > 4) {
							current_pos_dist = check_adj_pos_soldier(adj_pos, BFS_S_POSITION, data(weakest_enenmy_id).pos);
							if (current_pos_dist >= 0 and current_pos_dist < min_dist) { min_dist = current_pos_dist; chosen_new_pos = adj_pos; }
							// cerr << "IF attack -> current_pos_dist: " << current_pos_dist << " "; printPos(chosen_new_pos, "chosen_new_pos");
						}
					}
				}
				else if (attack)
				{
					if (isAccessiblePos(adj_pos, SOLDIER) and isPosEmptyOfThisUnitType(adj_pos, SOLDIER, 0) and not inFire(adj_pos) and not willProbablyBurnNextRound(adj_pos) and enemy_h_namapl > 4) {
						current_pos_dist = check_adj_pos_soldier(adj_pos, BFS_S_POSITION, data(weakest_enenmy_id).pos);
						if (current_pos_dist >= 0 and current_pos_dist < min_dist) { min_dist = current_pos_dist; chosen_new_pos = adj_pos; }
						// cerr << "IF attack -> current_pos_dist: " << current_pos_dist << " "; printPos(chosen_new_pos, "chosen_new_pos");
					}
				}
				else if (escape)
				{
					if (isAccessiblePos(adj_pos, SOLDIER) and isPosEmptyOfThisUnitType(adj_pos, SOLDIER, 0) and not inFire(adj_pos) and not willProbablyBurnNextRound(adj_pos) and enemy_h_namapl > 4) {
						current_pos_dist = check_adj_pos_soldier(adj_pos, BFS_S_POSITION, data(strongest_enenmy_id).pos);
						if (current_pos_dist >= 0 and current_pos_dist > max_dist) { max_dist = current_pos_dist; chosen_new_pos = adj_pos; }
						// cerr << "IF escape -> current_pos_dist: " << current_pos_dist << " "; printPos(chosen_new_pos, "chosen_new_pos");
					}
				}
			}
			
			else if (isAccessiblePos(adj_pos, SOLDIER) and isPosEmptyOfThisUnitType(adj_pos, SOLDIER, 0) and not inFire(adj_pos) and not willProbablyBurnNextRound(adj_pos) and enemy_h_namapl > 4)
			{
				if (what(adj_pos.i, adj_pos.j) == FOREST) {
					current_pos_dist = check_adj_pos_soldier(adj_pos, BFS_S_FOREST, PosErr);
					if (current_pos_dist >= 0 and not found_a_forest_pos) { min_dist = current_pos_dist; chosen_new_pos = adj_pos; found_a_forest_pos = true; }
					if (current_pos_dist >= 0 and found_a_forest_pos and current_pos_dist < min_dist) { min_dist = current_pos_dist; chosen_new_pos = adj_pos; }
					
					// cerr << "IF alone->forest -> current_pos_dist: " << current_pos_dist << " "; printPos(chosen_new_pos, "chosen_new_pos");
				}
				else if (not found_a_forest_pos) {
					current_pos_dist = check_adj_pos_soldier(adj_pos, BFS_S_NORMAL, PosErr);
					if (current_pos_dist >= 0 and current_pos_dist < min_dist) { min_dist = current_pos_dist; chosen_new_pos = adj_pos; }
					// cerr << "IF alone->NO_forest -> current_pos_dist: " << current_pos_dist << " "; printPos(chosen_new_pos, "chosen_new_pos");
				}
			}
			
			// cerr << "After iteration k = " << k << " -> current_pos_dist: " << current_pos_dist << " "; printPos(chosen_new_pos, "chosen_new_pos");
			// cerr << endl;

			if (second_half_of_soldiers) { --k; } else { ++k; }

		}
			
		if (isPosErr(chosen_new_pos)) chosen_new_pos = pos;

		
		printPos(chosen_new_pos,"COMMAND->chosen_new_pos"); // cerr << endl;
		
		// Move Soldier to new position
		command_soldier(id, chosen_new_pos.i, chosen_new_pos.j);
	}
	
	/**
	*	PLAY_HELICOPTER
	*/
	int play_helicopter(int id)
	{
		Position ppost = getPosOfPostUnderHelicopter(data(id).pos);		// ppost is the Pos of the post under the helicopter centered at 'cpos'
		if (not isEqualPos(ppost,PosErr) and (post_owner(ppost.i, ppost.j)) != me()) {
			bool hj = throwParachuter(id, ppost); // throw parach. to under_post
			for (unsigned int i = 0; i < data(id).parachuters.size() and i < MAX_JUMP; ++i) throwParachuter(id, PosErr);
		}
			
		if (napalmConditions(id)) { return NAPALM; }
		else {
			// cerr << "NO NAPALM" << endl;
			Position hpos = getHeadPos(id);
			int ori = data(id).orientation;
			int olddist = find_nearest_enemy_post_helicopter(hpos, ori);
			
			// cerr << "ori: " << ori << endl;
			// printPos(hpos,"hpos");
			
			int FX_ori = ori;
			Position F1_newhpos = getNewHeadPos(hpos, FX_ori, FORWARD1);		// Calculate newhpos supposing FORWARD1
			Position F2_newhpos = getNewHeadPos(hpos, FX_ori, FORWARD2);		// Calculate newhpos supposing FORWARD2
			
			int CC_ori = ori + 1;	if (CC_ori == 4) CC_ori = SOUTH;			// Now ori has been COUNTER_CLOCKWISE'd
			Position CC_newhpos = getNewHeadPos(hpos, ori, COUNTER_CLOCKWISE); 	// Calculate newhpos supposing COUNTER_CLOCKWISE
			
			int C_ori  = ori - 1;	if (C_ori == -1) C_ori  = WEST;				// Now ori has been CLOCKWISE'd
			Position C_newhpos	= getNewHeadPos(hpos, ori, CLOCKWISE); 			// Calculate newhpos supposing CLOCKWISE
			
			// cerr << "olddist: " << olddist << endl;
									
			if (helicopterWillNotCrash(id, F2_newhpos, FORWARD2)) {	// try to move FORWARD2
				int F2_newdist = find_nearest_enemy_post_helicopter(F2_newhpos, FX_ori);
				// cerr << "will not crash F2\n";
				// cerr << "F2_newdist: " << F2_newdist << endl;
				if (olddist >= 0 and F2_newdist >= 0 and F2_newdist <= olddist) { return FORWARD2; }
			}
			
			if (helicopterWillNotCrash(id, F1_newhpos, FORWARD1)) {	// try to move FORWARD1
				int F1_newdist = find_nearest_enemy_post_helicopter(F1_newhpos, FX_ori);
				// cerr << "will not crash F1\n";
				// cerr << "F1_newdist: " << F1_newdist << endl;
				if (olddist >= 0 and F1_newdist >= 0 and F1_newdist <= olddist) { return FORWARD1; }
			}
				
			int CC_newdist = find_nearest_enemy_post_helicopter(CC_newhpos, CC_ori);
			int  C_newdist = find_nearest_enemy_post_helicopter( C_newhpos,  C_ori);
			
			if (olddist >= 0 and CC_newdist >= 0 and CC_newdist <= olddist and noMountainsNear(CC_newhpos, CC_ori)) {
				if ((C_newdist >= 0 and CC_newdist <= C_newdist) or (C_newdist < 0)) {
					// cerr << "will not crash CC\n";
					// cerr << "CC_newdist: " << CC_newdist << endl;
					return COUNTER_CLOCKWISE;
				}
			}
			
			if (olddist >= 0 and  C_newdist >= 0 and  C_newdist <= olddist and noMountainsNear(C_newhpos,  C_ori)) {
				// cerr << "will not crash C\n";
				// cerr << "C_newdist: " << C_newdist << endl;
				return 		 CLOCKWISE;
			}
			
			if (helicopterWillNotCrash(id, F1_newhpos, FORWARD1) and noMountainsNear(F1_newhpos, FX_ori)) return FORWARD1;
			
			// decide which turn has more free space to then forward2
			bool spacious_CC = spaciousTurn(CC_newhpos, CC_ori);
			bool spacious_C  = spaciousTurn( C_newhpos,  C_ori);
			
			// cerr << "spacious_CC: " << spacious_CC << endl << " spacious_C: " << spacious_C << endl;
			
			if (spacious_CC and not spacious_C) return COUNTER_CLOCKWISE;
			if (not spacious_CC and spacious_C) return CLOCKWISE;
			
			// cerr << "spacious_CC: " << spacious_CC << endl << "spacious_C: " << spacious_C << endl;
			
			return random(COUNTER_CLOCKWISE, CLOCKWISE);
			
		}
	}
	
	/**
	*   BFS
	*/
	int bfs(Position p)
	{
		if (isPost(p) and post_owner(p.i, p.j) != me()) return 0;
		
		Mat_bool visited(MAX, vector<bool>(MAX, false));	// matrix for visited positions
		queue<Position> Q;								// vertexes, here 'Positions'
		queue<int> dQ;										// distance queue
		
		Q.push(p);
		dQ.push(0);
		visited[p.i][p.j] = true;
		
		while (not Q.empty()) {
		  Position cp = Q.front();
		  int dist = dQ.front() + 1;
		  
      // NORTH (cp + (-1,0))
      Position pN = sumPosPos(cp, displ[0]);
      if (not visited[pN.i][pN.j] and isAccessiblePos(pN, SOLDIER)) {
      // and fire_time(new_pos.i, new_pos.j) < dist)
			    if (isPost(pN) and post_owner(pN.i, pN.j) != me()) return dist;
          Q.push(pN);
          dQ.push(dist);
          visited[pN.i][pN.j] = true;
      }
      		  
      // NORTH EAST (cp + (-1,+1))
      Position pNE = sumPosPos(cp, displ[1]);
      if (not visited[pNE.i][pNE.j] and isAccessiblePos(pNE, SOLDIER)) {
			    if (isPost(pNE) and post_owner(pNE.i, pNE.j) != me()) return dist;
          Q.push(pNE);
          dQ.push(dist);
          visited[pNE.i][pNE.j] = true;
      }
      		  
      // EAST (cp + (0,+1))
      Position pE = sumPosPos(cp, displ[2]);
      if (not visited[pE.i][pE.j] and isAccessiblePos(pE, SOLDIER)) {
			    if (isPost(pE) and post_owner(pE.i, pE.j) != me()) return dist;
          Q.push(pE);
          dQ.push(dist);
          visited[pE.i][pE.j] = true;
      }
      		  
      // SOUTH EAST (cp + (+1,+1))
      Position pSE = sumPosPos(cp, displ[3]);
      if (not visited[pSE.i][pSE.j] and isAccessiblePos(pSE, SOLDIER)) {
			    if (isPost(pSE) and post_owner(pSE.i, pSE.j) != me()) return dist;
          Q.push(pSE);
          dQ.push(dist);
          visited[pSE.i][pSE.j] = true;
      }
      		  
      // SOUTH (cp + (+1,0))
      Position pS = sumPosPos(cp, displ[4]);
      if (not visited[pS.i][pS.j] and isAccessiblePos(pS, SOLDIER)) {
			    if (isPost(pS) and post_owner(pS.i, pS.j) != me()) return dist;
          Q.push(pS);
          dQ.push(dist);
          visited[pS.i][pS.j] = true;
      }
      		  
      // SOUTH WEST (cp + (+1,-1))
      Position pSW = sumPosPos(cp, displ[5]);
      if (not visited[pSW.i][pSW.j] and isAccessiblePos(pSW, SOLDIER)) {
			    if (isPost(pSW) and post_owner(pSW.i, pSW.j) != me()) return dist;
          Q.push(pSW);
          dQ.push(dist);
          visited[pSW.i][pSW.j] = true;
      }
      
      // WEST (cp + (O,-1))
      Position pW = sumPosPos(cp, displ[6]);
      if (not visited[pW.i][pW.j] and isAccessiblePos(pW, SOLDIER)) {
			    if (isPost(pW) and post_owner(pW.i, pW.j) != me()) return dist;
          Q.push(pW);
          dQ.push(dist);
          visited[pW.i][pW.j] = true;
      }
  
      // NORTH WEST (cp + (-1,-1))
      Position pNW = sumPosPos(cp, displ[7]);
      if (not visited[pNW.i][pNW.j] and isAccessiblePos(pNW, SOLDIER)) {
			    if (isPost(pNW) and post_owner(pNW.i, pNW.j) != me()) return dist;
          Q.push(pNW);
          dQ.push(dist);
          visited[pNW.i][pNW.j] = true;
      }
    
		  Q.pop();
      dQ.pop();
		}
		return -1; // NOT FOUND
		
	}
	
	/**
	*   MOVE_SOLDIER
	*/
	void move_soldier(int id)
	{
		Position pos = data(id).pos;
		Position newpos = PosErr;         // chosen_new_pos
		
		// cerr << "Soldier " << id; printPos(pos, " at Pos");
		
		vector<int> vec_id_enemies = getEnemiesIDsInNearArea(pos, SOLDIER);
		// printVec(vec_id_enemies,"Vec_id_en");
		
		// If there's an enemy adjacent to me -> attack him
		if (not vec_id_enemies.empty() and vec_id_enemies.size() == 1) {
			int wEID = weakestEnemyAround(vec_id_enemies, id);
			newpos = data(wEID).pos;
			command_soldier(id, newpos.i, newpos.j);
			return;
			
			/*if (data(wEID).life <= data(id).life) { newpos = data(wEID).pos;	command_soldier(id, newpos.i, newpos.j);	return;	}*/
		}

		int mindist = MAX*MAX, curdist = -1;
		for(int k = 0; k < 8; ++k) {
			Position adjpos = sumPosPos(pos, displ[k]);
			// printPos(adjpos, "checking my adjacents");
			if (isPosEmptyOfThisUnitType(adjpos, SOLDIER, 0) and not willProbablyBurnNextRound(adjpos)) {      // and enemy_h_namapl > 4)
				curdist = bfs(adjpos);  // BFS to post
				// cerr << "curdist " << curdist << "  min " << mindist << endl;
				if (curdist >= 0 and curdist < mindist) {	mindist = curdist;	newpos = adjpos;	}
			}
		}
			
		// printPos(newpos, "chosen new pos");
		if (not isPosErr(newpos)) command_soldier(id, newpos.i, newpos.j);
	}
	
	/**
	*	MOVE_HELICOPTER
	*/
	void move_helicopter(int id)
	{
		int instr = 0;
		if (round() < 2) instr = FORWARD2;
		else if (round() == 2) {
			if (data(id).orientation % 2 != 0) instr = COUNTER_CLOCKWISE;
			else instr = CLOCKWISE;
		} 
		else if (napalmConditions(id)) instr = NAPALM;
		else if ((round() % 20) == 0) {
			if (data(id).orientation % 2 != 0) instr = COUNTER_CLOCKWISE;
			else instr = random(COUNTER_CLOCKWISE, CLOCKWISE);
		}
		else if ((round() % 33) == 0) {
			if (data(id).orientation % 2 != 0) instr = random(COUNTER_CLOCKWISE, CLOCKWISE);
			else instr = CLOCKWISE;
		}
		else
		{	
			int ori = data(id).orientation;
			Position hpos = getHeadPos(id);
			Position newhpos = getNewHeadPos(hpos, ori, FORWARD2);
			
			if (nhi_1.i == id) {
				if (nhi_1.j > 0) {		// There's an stored instruction for this round
					instr = nhi_1.j;
					nhi_1.j = 0;
				}
				else if (not helicopterWillNotCrash(id, newhpos, FORWARD2))  {	// heli will crash
					instr = COUNTER_CLOCKWISE;
					nhi_1.j = COUNTER_CLOCKWISE;
				}
				else instr = FORWARD2;
			}
			else if (nhi_2.i ==  id) {
				if (nhi_2.j > 0) {		// There's an stored instruction for this round
					instr = nhi_2.j;
					nhi_2.j = 0;
				}
				else if (not helicopterWillNotCrash(id, newhpos, FORWARD2))  {	// heli will crash
					instr = COUNTER_CLOCKWISE;
					nhi_2.j = COUNTER_CLOCKWISE;
				}
				else instr = FORWARD2;
			}
		}

		// cerr << "instr: " << instr << endl;
		if (instr > 0) command_helicopter(id, instr);
		  
	}
	
	/**
	* Play method, invoked once per each round.
	*/
	virtual void play()
	{
		int pid = me(); 	// My player id
		if (status(pid) > MAX_CPU_time_allowed) return;
		
		vector<int> helicopters = Player::helicopters(pid);	
		
		if (round() == 0) {		nhi_1 = Position(helicopters[0], 0); 	nhi_2 = Position(helicopters[1], 0);	}	
			
		for (int i = 0; i < (int)helicopters.size(); ++i)
		{
			// PARACHUTERS
			vector<int> parachuters = data(helicopters[i]).parachuters;
			for (int i = 0; i < (int)parachuters.size(); ++i) {
				if (parachuters[i] <= PARACHUTER_ALMOST_DEAD_JUMP) {	bool has_jumped = throwParachuter(helicopters[i], PosErr);	}	// Throw Parachuters about to die
			}
			
			// HELICOPTERS
			move_helicopter(helicopters[i]);
		}
		    
			// SOLDIERS
		vector<int> soldiers = Player::soldiers(pid);
		for (int i = 0; i < (int)soldiers.size(); ++i) {	move_soldier(soldiers[i]);	}
		
		/*
		char c;
		cerr << endl << endl << "NEXT ROUND? : ";
		cin >> c;
		*/
		
	}
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);