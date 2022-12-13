/**************************************************************************
PBEAMPP.C of ZIBB optimizer MCF, SPEC version

This software was developed at ZIBB Berlin. Maintenance and revisions 
solely on responsibility of Andreas Loebel

Dr. Andreas Loebel
Ortlerweg 29b, 12207 Berlin

Konrad-Zuse-Zentrum fuer Informationstechnik Berlin (ZIB)
Scientific Computing - Optimization
Takustr. 7, 14195 Berlin-Dahlem

Copyright (c) 1998-2000 ZIB.           
Copyright (c) 2000-2002 ZIBB & Loebel.  
Copyright (c) 2003-2005 Andreas Loebel.
**************************************************************************/
/*  LAST EDIT: Sun Nov 21 16:22:04 2004 by Andreas Loebel (boss.local.de)  */
/*  $Id: pbeampp.c,v 1.10 2005/02/17 19:42:32 bzfloebe Exp $  */



#define KK 300
#define BB  50



#include "pbeampp.h"


#ifdef _PROTO_
int bea_is_dual_infeasible(const arc_t& arc, cost_t red_cost )
#else
int bea_is_dual_infeasible( arc, red_cost )
    arc_t *arc;
    cost_t red_cost;
#endif
{
    return(    (red_cost < 0 && arc.ident == AT_LOWER)
            || (red_cost > 0 && arc.ident == AT_UPPER) );
}







typedef struct basket
{
    // arc_t *a;
    uint64_t a;
    cost_t cost;
    cost_t abs_cost;
} BASKET;

static long basket_size;
static BASKET basket[BB+KK+1];
static BASKET *perm[BB+KK+1];



#ifdef _PROTO_
void sort_basket( long min, long max )
#else
void sort_basket( min, max )
    long min, max;
#endif
{
    long l, r;
    cost_t cut;
    BASKET *xchange;

    l = min; r = max;

    cut = perm[ (long)( (l+r) / 2 ) ]->abs_cost;

    do
    {
        while( perm[l]->abs_cost > cut )
            l++;
        while( cut > perm[r]->abs_cost )
            r--;
            
        if( l < r )
        {
            xchange = perm[l];
            perm[l] = perm[r];
            perm[r] = xchange;
        }
        if( l <= r )
        {
            l++; r--;
        }

    }
    while( l <= r );

    if( min < r )
        sort_basket( min, r );
    if( l < max && l <= BB )
        sort_basket( l, max ); 
}






static long nr_group;
static long group_pos;


static long initialize = 1;


#ifdef _PROTO_
uint64_t primal_bea_mpp( long m,  /*arc_t *arcs, arc_t *stop_arcs,*/
                     arc_array_t& arcs, uint64_t stop_arcs,
                              cost_t *red_cost_of_bea )
#else
arc_t *primal_bea_mpp( m, arcs, stop_arcs, red_cost_of_bea )
    long m;
    arc_t *arcs;
    arc_t *stop_arcs;
    cost_t *red_cost_of_bea;
#endif
{
    long i, next, old_group_pos;
    // arc_t *arc;
    uint64_t idx;
    cost_t red_cost;

    if( initialize )
    {
        for( i=1; i < KK+BB+1; i++ )
            perm[i] = &(basket[i]);
        nr_group = ( (m-1) / KK ) + 1;
        group_pos = 0;
        basket_size = 0;
        initialize = 0;
    }
    else
    {
        for( i = 2, next = 0; i <= BB && i <= basket_size; i++ )
        {
            idx = perm[i]->a;
            DerefScope scope;
            const arc_t &arc = arcs.at(scope, idx);
            red_cost = arc.cost - arc.tail->potential + arc.head->potential;
            if( (red_cost < 0 && arc.ident == AT_LOWER)
                || (red_cost > 0 && arc.ident == AT_UPPER) )
            {
                next++;
                perm[next]->a = idx;
                perm[next]->cost = red_cost;
                perm[next]->abs_cost = ABS(red_cost);
            }
        }   
        basket_size = next;
    }

    old_group_pos = group_pos;

    do {
        /* price next group */
        // arc = arcs + group_pos;
        idx = group_pos;
        for( ; idx < stop_arcs; idx += nr_group )
        {   
            DerefScope scope;
            const arc_t &arc = arcs.at(scope, idx);

            if( arc.ident > BASIC )
            {
                /* red_cost = bea_compute_red_cost( arc ); */
                red_cost = arc.cost - arc.tail->potential + arc.head->potential;
                if( bea_is_dual_infeasible( arc, red_cost ) )
                {
                    basket_size++;
                    perm[basket_size]->a = idx;
                    perm[basket_size]->cost = red_cost;
                    perm[basket_size]->abs_cost = ABS(red_cost);
                }
            }
        }

        if( ++group_pos == nr_group )
            group_pos = 0;
    } while ( basket_size < BB && group_pos != old_group_pos );

    
    if( basket_size == 0 )
    {
        initialize = 1;
        *red_cost_of_bea = 0; 
        return NULLPTR;
    }
    sort_basket( 1, basket_size );
    *red_cost_of_bea = perm[1]->cost;
    return( perm[1]->a );
}










