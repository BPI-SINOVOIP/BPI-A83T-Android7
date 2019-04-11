#include "atw_math.h"

static void  solveProjectionMatrixWithCompactCoeffs(const double in[], const double out[], double P[]);

void GetBlkCoef(const double ouBlk[], const double inBlk[], double coef[])
{
    solveProjectionMatrixWithCompactCoeffs(ouBlk, inBlk, coef);
}

static void  solveProjectionMatrixWithCompactCoeffs(const double in[], const double out[], double P[])
{
	//This version cut out unnecessary elements in coefficient matrix, which makes
	// the 8 * 9 matrix to 8 * 5.

	//Matrix form:
	//        M ==
	//                  0     1         2         3       4    

	//        0  	[	x1,   y1,    -x1*u1,   -y1*u1,    u1; ...

	//        1  	 	x2,   y2,    -x2*u2,   -y2*u2,    u2; ...

	//        2  	 	x3,   y3,    -x3*u3,   -y3*u3,    u3; ...

	//        3  	 	x1,   y1,    -x1*v1,   -y1*v1,    v1; ...

	//        4  	 	x2,   y2,    -x2*v2,   -y2*v2,    v2; ...

	//        5  	 	x3,   y3,    -x3*v3,   -y3*v3,    v3; ...

	//        6  	 	x4,   y4,    -x4*v4,   -y4*v4,    v4; ...

	//        7  	 	x4,   y4,    -x4*u4,   -y4*u4,    u4; ...
	//          	];

	double x1 = in[0], y1 = in[1],
		x2 = in[2], y2 = in[3],
		x3 = in[4], y3 = in[5],
		x4 = in[6], y4 = in[7];
	//output coordinates
	double u1 = out[0], v1 = out[1],
		u2 = out[2], v2 = out[3],
		u3 = out[4], v3 = out[5],
		u4 = out[6], v4 = out[7];

	double M[40] = {
	x1,    y1,    -x1 * u1,    -y1 * u1,    u1,
	x2,    y2,    -x2 * u2,    -y2 * u2,    u2,
	x3,    y3,    -x3 * u3,    -y3 * u3,    u3,
	x1,    y1,    -x1 * v1,    -y1 * v1,    v1,
	x2,    y2,    -x2 * v2,    -y2 * v2,    v2,
	x3,    y3,    -x3 * v3,    -y3 * v3,    v3,
	x4,    y4,    -x4 * v4,    -y4 * v4,    v4,
	x4,    y4,    -x4 * u4,    -y4 * u4,    u4
	};

	//forward elimination: column 0
	// M( 1, : ) -= M( 0, : )
	// M( 2, : ) -= M( 0, : )
	// M( 7, : ) -= M( 0, : )
    // affected columns: 0, 1, 2, 6, 7, 8
	//M[1][1] -= M[0][1]; M[1][2] -= M[0][2]; M[1][6] -= M[0][6]; M[1][7] -= M[0][7]; 
	//M[1][8] -= M[0][8];
	M[5] -= M[0]; 
	M[6] -= M[1]; 
	M[7] -= M[2]; 
	M[8] -= M[3]; 
	M[9] -= M[4];

	//M[2][1] -= M[0][1]; M[2][2] -= M[0][2]; M[2][6] -= M[0][6]; M[2][7] -= M[0][7]; 
	//M[2][8] -= M[0][8];
	M[10] -= M[0]; 
	M[11] -= M[1]; 
	M[12] -= M[2]; 
	M[13] -= M[3]; 
	M[14] -= M[4];

	//M[7][1] -= M[0][1]; M[7][2] -= M[0][2]; M[7][6] -= M[0][6]; M[7][7] -= M[0][7]; 
	//M[7][8] -= M[0][8];
	M[35] -= M[0]; 
	M[36] -= M[1]; 
	M[37] -= M[2]; 
	M[38] -= M[3]; 
	M[39] -= M[4];


	//forward elimination: column 1
	// M( 1, : ) /= M( 1, 1 )
	// M( 2, : ) -= M( 1, : ) * M( 2, 1 )
	// M( 7, : ) -= M( 1, : ) * M( 7, 1 )
    // affected columns: 1, 2, 6, 7, 8
	//double factor = 1. / M[1][1];
	//M[1][2] *= factor;           M[1][6] *= factor;           M[1][7] *= factor;           
	//M[1][8] *= factor;
	double factor = 1. / M[5];
	M[6] *= factor;           
	M[7] *= factor;           
	M[8] *= factor;           
	M[9] *= factor;

	//factor = M[2][1];
	//M[2][2] -= M[1][2] * factor; M[2][6] -= M[1][6] * factor; M[2][7] -= M[1][7] * factor; 
	//M[2][8] -= M[1][8] * factor;
	factor = M[10];
	M[11] -= M[6] * factor; 
	M[12] -= M[7] * factor; 
	M[13] -= M[8] * factor; 
	M[14] -= M[9] * factor;

	//factor = M[7][1];
	//M[7][2] -= M[1][2] * factor; M[7][6] -= M[1][6] * factor; M[7][7] -= M[1][7] * factor; 
	//M[7][8] -= M[1][8] * factor;
	factor = M[35];
	M[36] -= M[6] * factor; 
	M[37] -= M[7] * factor; 
	M[38] -= M[8] * factor; 
	M[39] -= M[9] * factor;


	//forward elimination : column 2
	// M(2, :) /= M(2, 2);
	// M(7, :) -= M(2, :) .* M(7, 2);
    // affected columns: 2, 6, 7, 8
	//factor = 1. / M[2][2];
	//M[2][6] *= factor; M[2][7] *= factor; 
	//M[2][8] *= factor;
	factor = 1. / M[11];
	M[12] *= factor; 
	M[13] *= factor; 
	M[14] *= factor;

	//factor = M[7][2];
	//M[7][6] -= M[2][6] * factor; M[7][7] -= M[2][7] * factor; 
	//M[7][8] -= M[2][8] * factor;
	factor = M[36];
	M[37] -= M[12] * factor; 
	M[38] -= M[13] * factor; 
	M[39] -= M[14] * factor;

	//forward elimination : column 3
	// M(4, :) -= M(3, :);
	// M(5, :) -= M(3, :);
	// M(6, :) -= M(3, :);
    // affected columns: 3, 4, 5, 6, 7, 8
	//M[4][4] -= M[3][4]; M[4][5] -= M[3][5]; M[4][6] -= M[3][6]; M[4][7] -= M[3][7]; 
	//M[4][8] -= M[3][8];
	M[20] -= M[15]; 
	M[21] -= M[16]; 
	M[22] -= M[17]; 
	M[23] -= M[18]; 
	M[24] -= M[19];

	//M[5][4] -= M[3][4]; M[5][5] -= M[3][5]; M[5][6] -= M[3][6]; M[5][7] -= M[3][7]; 
	//M[5][8] -= M[3][8];
	M[25] -= M[15]; 
	M[26] -= M[16]; 
	M[27] -= M[17]; 
	M[28] -= M[18]; 
	M[29] -= M[19];

	//M[6][4] -= M[3][4]; M[6][5] -= M[3][5]; M[6][6] -= M[3][6]; M[6][7] -= M[3][7]; 
	//M[6][8] -= M[3][8];
	M[30] -= M[15]; 
	M[31] -= M[16]; 
	M[32] -= M[17]; 
	M[33] -= M[18]; 
	M[34] -= M[19];

	//forward elimination : column 4
	// M(4, :) /= M(4, 4);
	// M(5, :) -= M(4, :).*M(5, 4);
	// M(6, :) -= M(4, :).*M(6, 4);
    // affected columns: 4, 5, 6, 7, 8
	//factor = 1. / M[4][4];
	//M[4][5] *= factor;           M[4][6] *= factor;           M[4][7] *= factor;           
	//M[4][8] *= factor;
	factor = 1. / M[20];
	M[21] *= factor;           
	M[22] *= factor;           
	M[23] *= factor;           
	M[24] *= factor;

    //factor = M[5][4];
	//M[5][5] -= M[4][5] * factor; M[5][6] -= M[4][6] * factor; M[5][7] -= M[4][7] * factor; 
	//M[5][8] -= M[4][8] * factor;
    factor = M[25];
	M[26] -= M[21] * factor; 
	M[27] -= M[22] * factor; 
	M[28] -= M[23] * factor; 
	M[29] -= M[24] * factor;

    //factor = M[6][4];
	//M[6][5] -= M[4][5] * factor; M[6][6] -= M[4][6] * factor; M[6][7] -= M[4][7] * factor; 
	//M[6][8] -= M[4][8] * factor;
    factor = M[30];
	M[31] -= M[21] * factor; 
	M[32] -= M[22] * factor; 
	M[33] -= M[23] * factor; 
	M[34] -= M[24] * factor;

	//%forward elimination : column 5
	// M(5, :) /= M(5, 5);
	// M(6, :) -= M(5, :).*M(6, 5);
    // affected columns: 5, 6, 7, 8
	//factor = 1. / M[50];
	//M[51] *= factor;           M[52] *= factor;           M[53] *= factor;
	factor = 1. / M[26];
	M[27] *= factor;           
	M[28] *= factor;           
	M[29] *= factor;
    
    //factor = M[59];
	//M[60] -= M[51] * factor; M[61] -= M[52] * factor; M[62] -= M[53] * factor;
    factor = M[31];
	M[32] -= M[27] * factor; 
	M[33] -= M[28] * factor; 
	M[34] -= M[29] * factor;

	//%forward elimination : column 6
	// M(6, :) /= M(6, 6);
	// M(7, :) -= M(6, :).*M(7, 6);
    // affected columns: 6, 7, 8
	//factor = 1. / M[6][6];
	//M[6][7] *= factor;           
	//M[6][8] *= factor;
	factor = 1. / M[32];
	M[33] *= factor;           
	M[34] *= factor;
    
    
    //factor = M[7][6];
	//M[7][7] -= M[6][7] * factor; 
	//M[7][8] -= M[6][8] * factor;
    factor = M[37];
	M[38] -= M[33] * factor; 
	M[39] -= M[34] * factor;

	//%forward elimination : column 7 
	// M(7, :) /= M(7, 7);
    // affected columns: 7, 8
	//factor = 1. / M[7][7];
	//M[7][8] *= factor;
	M[39] /= M[38];

	//%backward elimination : column 7 
	//  M(6, :) -= M(7, :) * M(6, 7);
	//  M(5, :) -= M(7, :) * M(5, 7);
	//  M(4, :) -= M(7, :) * M(4, 7);
	//  M(3, :) -= M(7, :) * M(3, 7);
	//  M(2, :) -= M(7, :) * M(2, 7);
	//  M(1, :) -= M(7, :) * M(1, 7);
	//  M(0, :) -= M(7, :) * M(0, 7);
    // affected columns: 7, 8
    //factor = M[7][8];
    //M[6][8] -= M[6][7] * factor;    
    //M[5][8] -= M[5][7] * factor;    
    //M[4][8] -= M[4][7] * factor;    
    //M[3][8] -= M[3][7] * factor;    
    //M[2][8] -= M[2][7] * factor;    
    //M[1][8] -= M[1][7] * factor;    
    //M[0][8] -= M[0][7] * factor;    
    factor = M[39];
    M[34] -= M[33] * factor;    
    M[29] -= M[28] * factor;    
    M[24] -= M[23] * factor;    
    M[19] -= M[18] * factor;    
    M[14] -= M[13] * factor;    
    M[9]  -= M[8] * factor;    
    M[4]  -= M[3] * factor;    

	//%backward elimination : column 6 
	//  M(5, :) -= M(6, :) * M(5, 6);
	//  M(4, :) -= M(6, :) * M(4, 6);
	//  M(3, :) -= M(6, :) * M(3, 6);
	//  M(2, :) -= M(6, :) * M(2, 6);
	//  M(1, :) -= M(6, :) * M(1, 6);
	//  M(0, :) -= M(6, :) * M(0, 6);
    // affected columns: 6, 8
    //factor = M[6][8];
    //M[5][8] -= M[5][6] * factor;    
    //M[4][8] -= M[4][6] * factor;    
    //M[3][8] -= M[3][6] * factor;    
    //M[2][8] -= M[2][6] * factor;    
    //M[1][8] -= M[1][6] * factor;    
    //M[0][8] -= M[0][6] * factor;    
    factor = M[34];
    M[29] -= M[27] * factor;    
    M[24] -= M[22] * factor;    
    M[19] -= M[17] * factor;    
    M[14] -= M[12] * factor;    
    M[9]  -= M[7] * factor;    
    M[4]  -= M[2] * factor;    

	//%backward elimination : column 5 
	//  M(4, :) -= M(5, :) * M(4, 5);
	//  M(3, :) -= M(5, :) * M(3, 5);
    // affected columns: 5, 8
    //factor = M[5][8];
    //M[4][8] -= M[4][5] * factor;    
    //M[3][8] -= M[3][5] * factor;    
    M[24] -= M[21] * M[29];    
    M[19] -= M[16] * M[29];    

	//%backward elimination : column 4 
	//	M(3, :) -= M(4, :) * M(3, 4);
    // affected columns: 4, 8
    //factor = M[4][8];
    //M[3][8] -= M[3][4] * factor;    
    M[19] -= M[15] * M[24];    


	//%backward elimination : column 3 
	//	% already done

	//%backward elimination : column 2
	//	M(1, :) -= M(2, :) * M(1, 2);
	//  M(0, :) -= M(2, :) * M(0, 2);
    // affected columns: 2, 8
    //factor = M[2][8];
    //M[1][8] -= M[1][2] * factor;    
    //M[0][8] -= M[0][2] * factor;    
    M[9] -= M[6] * M[14];    
    M[4] -= M[1] * M[14];    

	//%backward elimination : column 1
	//	M(0, :) -= M(1, :) * M(0, 1);
    // affected columns: 1, 8
    //factor = M[1][8];
    //M[0][8] -= M[0][1] * factor;   
    M[4] -= M[0] * M[9];   

	//%backward elimination : column 0 
	//	% already done

	P[0] = M[9];
	P[1] = M[14];
	P[2] = M[4];
	P[3] = M[24];
	P[4] = M[29];
	P[5] = M[19];
	P[6] = M[34];
	P[7] = M[39];

	return;

}
