/**
 * @file cs1d.cpp
 * @brief 
 *
 */

#include "cs1d.h"
#include "knapsack.h"
#include <cmath>
#include <iostream>
#include <fstream>

const double ZERO_EPS = 1e-5; //tolerance for numeric issues

void Data::read(const char* filename)
{
	std::ifstream in(filename);
	// read data from file
	in >> m; // number of piece types
	in >> W; // stock length
	std::cout << "m = " << m << std::endl;
	std::cout << "W = " << W << std::endl;
	for (int i = 0; i < m; i++) { //for each piece type
		double l;
		double r;
		in >> l;	//length
		in >> r;	//demand
		L.push_back(l);
		R.push_back(r);
		std::cout << l << " " << r << std::endl;
	}
	in.close();
}

void CS1D::initMaster(const Data& data)
{
	// setup initial LP (Restricted Master Problem - RMP)
		
	// create all variables: RMP has data.m nonnegative continuos variables, the coefficient in the objective function is 1.0
	//TODO...
	std::vector<double> obj(data.m, 1.0);
	CHECKED_CPX_CALL( CPXnewcols, env, lp, data.m, &obj[0], NULL, NULL, NULL, NULL ); 
					//... NULL => default lb (0)  , ub (+inf), xctype (C), colname (no name)
	//...DONE
	
	// create all the constraints: variable with index i appears in constraint i with coefficient N_ii depending on data.L[i] and W
	//TODO...
	std::vector<int> idx(data.m);
	std::vector<double> coef(data.m);
	std::vector<char> sense(data.m, 'G');
	std::vector<int> matbeg(data.m);
	for (int i = 0; i < data.m; i++)
	{
		matbeg[i] = i;
		idx[i] = i;
		coef[i] = floor(data.W / data.L[i]);
	}
	CHECKED_CPX_CALL( CPXaddrows, env, lp, 0, data.m, data.m, &(data.R[0]), &sense[0], &matbeg[0], &idx[0], &coef[0], NULL, NULL );
	//...DONE
	
	// debug
	CHECKED_CPX_CALL( CPXwriteprob, env, lp, "initial.lp", NULL );
}

void CS1D::solveMasterLP(std::vector<double>& x, std::vector<double>& u, double& objval)
{
	//TODO
	// solve using CPX*lp*opt
	//...
	// get current LP obj value (reference objval)
	//...
	// get current RESTRICTED LP PRIMAL solution (reference x)
	//...
	// get current RESTRICTED LP DUAL solution using *CPXgetpi* (reference u)
	//...
	
	// solve using CPX*lp*opt
	CHECKED_CPX_CALL( CPXlpopt, env, lp );
	// get current LP obj value (reference objval)
	CHECKED_CPX_CALL( CPXgetobjval, env, lp, &objval );
	// get current RESTRICTED LP PRIMAL solution
	int n = CPXgetnumcols(env, lp);
	x.resize(n);
	CHECKED_CPX_CALL( CPXgetx, env, lp, &x[0], 0, n - 1 );
	// get current RESTRICTED LP DUAL solution
	int m = CPXgetnumrows(env, lp);
	u.resize(m);
	CHECKED_CPX_CALL( CPXgetpi, env, lp, &u[0], 0, m - 1 );
	//...DONE
}

bool CS1D::price(Env pricerEnv, const Data& data, const std::vector<double>& u)
{
	KPSolver kp(pricerEnv);
	std::vector<double> z;
	double value;
	
	
	//CALL kp.exec to solve the right knapsack problem and get 
	//	the related objective function into < value >
	//TODO...
	kp.exec(data.L, u, data.W, z, value);
	//...DONE
	
	//TODO...
	//"return false" if NO negative reduced cost variable exists
	//
	if (1 - value > -ZERO_EPS) return false;
	    // if the value is 1.000001, the reduced cost would be - 0.000001. Because 
	    //  of possible numerical issues, we consider this number as 0
	//...DONE
	
	//TODO...
	// prepare parameters for following *CPXaddcols*:
	//   - the vector idx of the indexes of the rows in which the variable appears with (nonzero) oefficient
	//   - the vector coef of the (nonzero) coefficients related to the row indexes above
	//   - the coefficient obj in the objective function
	//
	// add column
	std::vector<int> idx;
	std::vector<double> coef;
	int m = z.size();
	for(int i = 0; i < m; i++) 	{
		if (z[i] > ZERO_EPS) { // because of numerical troubles, we may have for z 
		                  //  values like  +/-1e-10 (that is 0)
			idx.push_back(i); 	//the variable will appear in the i-th constraint...
			coef.push_back(z[i]);	// ... with coefficient z[i]
		}
	}
	double obj = 1.0;
	int matbeg = 0;
	//...DONE
	
	// add the variable to the model
	CHECKED_CPX_CALL( CPXaddcols, env, lp, 1, idx.size(), &obj, &matbeg, &idx[0], &coef[0], NULL, NULL, NULL );
		//status = CPXaddcols (env, lp, ccnt, nzcnt, obj, cmatbeg, cmatind, cmatval, lb, ub, newcolname);
	return true;
}


void CS1D::branchAndBoundOnThePartialModel(std::vector<double>& x, double& objval)
// provides a heuristic (not necessarily optimal) integer solution
{
	// change to Integer Program
	int n = CPXgetnumcols(env, lp);
	std::vector<char> xtype(n, 'I');
	CHECKED_CPX_CALL( CPXcopyctype, env, lp, &xtype[0] );
	// solve
	CHECKED_CPX_CALL( CPXmipopt, env, lp );
	// get solution
	CHECKED_CPX_CALL( CPXgetobjval, env, lp, &objval );
	x.resize(n);
	CHECKED_CPX_CALL( CPXgetx, env, lp, &x[0], 0, n - 1 );
}










