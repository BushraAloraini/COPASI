
/**
 *  File name: CGA.h
 *
 *  Programmer: Yongqun He 
 *  Contact email: yohe@vt.edu
 *  Purpose: This is the interface (.h file) of the CGA class. 
 *           It is to implement the genetic algorithm for COPASI optimization
 */


#ifndef CGA_H
#define CGA_H


#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "math.h"
#include "float.h"
#include "..\cdatum.h"
#include "..\optparam.h"

class COptParam;
extern "C" double dr250( void );
extern "C" void r250_init(int seed);
extern "C" unsigned int r250n(unsigned n);
extern "C" double rnormal01( void );

// global variables
int debug;				// 1 if output is to be written
unsigned int gener;			// number of generations
unsigned int popsize;		        // size of the population
int ncross;				// number of crossover points
double mutvar;				// variance for mutations
double mutprob;				// probability of mutations
unsigned int best;			// index of the best individual
int nparam;				// number of parameters
int nconstr;				// number of constraints
double tau1;				// parameter for updating variances
double tau2;				// parameter for updating variances
COptParam **parameter;		        // array of parameters being searched
COptParam **constraint;		        // array of constraints
double **indv;				// for array of individuals w/ candidate values for the parameters
double *candx;				// array of values of objective function f/ individuals
unsigned int *crp;			// indexes of the crossover points
unsigned int *midx;			// indexes for shuffling the population
unsigned int *wins;			// number of wins of each individual in the tournament
double (*f) (void);			// pointer to function evaluations
void (*callback) (double);	        // pointer to callback function


extern "C" __declspec( dllexport ) void OptDLLName( char *name );
extern "C" __declspec( dllexport ) int OptDLLInit();
extern "C" __declspec( dllexport ) char * OptDLLParameterName( int p, char *name );
extern "C" __declspec( dllexport ) int OptDLLParameterNumber( void );
extern "C" __declspec( dllexport ) void OptDLLSetMethodParameter( int n, double p );
extern "C" __declspec( dllexport ) int OptDLLVersion( void );
extern "C" __declspec( dllexport ) int OptDLLIsConstrained( void );
extern "C" __declspec( dllexport ) int OptDLLIsBounded( void );
extern "C" __declspec( dllexport ) int OptDLLSolveLsq( double (*feval) (double *) );
extern "C" __declspec( dllexport ) void OptDLLSetCallback( void (*cb) (double) );
extern "C" __declspec( dllexport ) void OptDLLClearMemory( void );
extern "C" __declspec( dllexport ) int OptDLLRoutines( void );
extern "C" __declspec( dllexport ) int OptDLLCreateArrays( int p, int c, int r );
extern "C" __declspec( dllexport ) void OptDLLSetOptParam( int i, COptParam *pr );
extern "C" __declspec( dllexport ) void OptDLLSetConstr( int i, COptParam *pr );
extern "C" __declspec( dllexport ) double OptDLLGetOptParameter( int i );
extern "C" __declspec( dllexport ) double OptDLLGetObjF( void );
extern "C" __declspec( dllexport ) int OptDLLOptimise( double (*feval) (void) );


// evaluate the fitness of one individual
double evaluate( unsigned int i );

// copy individual o to position d
void copy( unsigned int o, unsigned int d );

// swap individuals o and d
void swap( unsigned int o, unsigned int d );

//mutate one individual
void mutate( unsigned int i );

void crossover( unsigned int p1, unsigned int p2, unsigned int c1, unsigned int c2 );

void shuffle( void );

// replicate the individuals w/ crossover
void replicate( void );

// select popsize individuals
void select( int method );

// check the best individual at this generation
unsigned int fittest( void );

// initialise the population
void creation( unsigned int l, unsigned int u );

// routines used for debugging
void dump_data_init( void );

void dump_data( unsigned int i );

//listed above
//extern "C" __declspec( dllexport ) int OptDLLOptimise( double (*feval) (void) );



#endif 





