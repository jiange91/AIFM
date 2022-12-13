/**************************************************************************
DEFINES.H of ZIB optimizer MCF, SPEC version

This software was developed at ZIB Berlin. Maintenance and revisions 
solely on responsibility of Andreas Loebel

Dr. Andreas Loebel
Ortlerweg 29b, 12207 Berlin

Konrad-Zuse-Zentrum fuer Informationstechnik Berlin (ZIB)
Scientific Computing - Optimization
Takustr. 7, 14195 Berlin-Dahlem

Copyright (c) 1998-2000 ZIB.           
Copyright (c) 2000-2002 ZIB & Loebel.  
Copyright (c) 2003-2005 Andreas Loebel.
**************************************************************************/
/*  LAST EDIT: Thu Feb 17 21:46:48 2005 by Andreas Loebel (boss.local.de)  */
/*  $Id: defines.h,v 1.13 2005/02/17 21:43:12 bzfloebe Exp $  */



#ifndef _DEFINES_H
#define _DEFINES_H

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>

#ifdef INTERNAL_TIMING
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>
#endif


extern "C" {
#include <runtime/runtime.h>
}

#include "array.hpp"
#include "device.hpp"
#include "helpers.hpp"
#include "manager.hpp"

#include "prototyp.h"


#define UNBOUNDED        1000000000
#define ZERO             0
#define MAX_ART_COST     (long)(100000000L)
#define ARITHMETIC_TYPE "I"



#define FIXED           -1
#define BASIC            0
#define AT_LOWER         1
#define AT_UPPER         2
/* #define AT_ZERO           3  NOT ALLOWED FOR THE SPEC VERSION */
#undef AT_ZERO


#define UP    1
#define DOWN  0



typedef long flow_t;
typedef long cost_t;




#ifndef NULL
#define NULL 0
#endif


#ifndef ABS
#define ABS( x ) ( ((x) >= 0) ? ( x ) : -( x ) )
#endif


#ifndef MAX
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#endif


#ifndef SET_ZERO
#define SET_ZERO( vec, n ) if( vec ) memset( (void *)vec, 0, (size_t)n )
#endif


#ifndef FREE
#define FREE( vec ) if( vec ) free( (void *)vec )
#endif


typedef struct node_t *node_p;

typedef struct arc_t *arc_p;

using namespace far_memory;

#define MAX_SIZE 6204416
#define BOUND (MAX_SIZE + 26451)
#define NULLPTR (30 << 21)
#define is_nullptr(x) (x < 0 || x >= BOUND)

typedef Array<arc_t, BOUND> arc_array_t; 


struct node_t
{
  cost_t potential; 
  int orientation;
  node_p child;
  node_p pred;
  node_p sibling;
  node_p sibling_prev;     
  uint64_t basic_arc; 
  uint64_t firstout, firstin;
  uint64_t arc_tmp;
  flow_t flow;
  long depth; 
  int number;
  int time;
};



struct arc_t
{
  cost_t cost;
  node_p tail, head;
  int ident;
  uint64_t nextout, nextin;
  flow_t flow;
  cost_t org_cost;
};



typedef struct network
{
  char inputfile[200];
  char clustfile[200];
  long n, n_trips;
  long max_m, m, m_org, m_impl;
  long max_residual_new_m, max_new_m;
  
  long primal_unbounded;
  long dual_unbounded;
  long perturbed;
  long feasible;
  long eps;
  long opt_tol;
  long feas_tol;
  long pert_val;
  long bigM;
  double optcost;  
  cost_t ignore_impl;
  node_p nodes, stop_nodes;
  uint64_t stop_arcs;
  uint64_t dummy_arcs, stop_dummy; 
  long iterations;
  long bound_exchanges;
  long checksum;
} network_t;



#endif
