#define "defs.h"


U64 GeneratePosKey(const S_BOARD *pos){
	
	int sq = 0;
	U64 finalKey = 0;
	int piece = EMPTY;
	
	//PIECE 
	for(sq=0; sq<BRD_SQ_NUM; ++sq){
		piece = pos->pieces[sq];
		if (piece!=NO_SQ && piece!=EMPTY){
			finalKey ^= PieceKeys[piece][sq];
			
			
		}
	}
	if(pos->side == WHITE){
		finalKey ^= SideKey;
	}
	if(pos->enPas != NO_SQ){
		finalKey ^= PieceKeys[EMPTY][pos->enPas];
	}
	
	finalKey ^= CastleKeys[pos->castlePerm];
	return finalKey;
	}
