/**************************************************************************
READMIN.C of ZIB optimizer MCF, SPEC version

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
/*  LAST EDIT: Thu Feb 17 20:44:29 2005 by Andreas Loebel (boss.local.de)  */
/*  $Id: readmin.c,v 1.16 2005/02/17 19:44:40 bzfloebe Exp $  */



#include "readmin.h"
#include "internal_utils.h"




#ifdef _PROTO_
long read_min(arc_array_t& arcs, network_t *net )
#else
long read_min( net )
     network_t *net;
#endif
{                                       
    char instring[201];
    long t, h, c;
    long i;
    // arc_t *arc;
    node_t *node;
    // FILE *in;

    // if(( in = fopen( net->inputfile, "r")) == NULL )
    //     return -1;
    if ( !init_file(net->inputfile) )
        return -1;

    // fgets( instring, 200, in );
    // if( sscanf( instring, "%ld %ld", &t, &h ) != 2 )
    //     return -1;
    if ( !read_l_l(&t, &h) )
        return -1;

    net->n_trips = t;
    net->m_org = h;
    net->n = (t+t+1); 
    net->m = (t+t+t+h);

    if( net->n_trips <= MAX_NB_TRIPS_FOR_SMALL_NET )
    {
        net->max_m = net->m;
        net->max_new_m = MAX_NEW_ARCS_SMALL_NET;
    }
    else
    {
#ifdef SPEC_STATIC
/*
      //net->max_m = 0x1c00000l;
*/
      net->max_m = 0x1a10000l;
#else
      net->max_m = MAX( net->m + MAX_NEW_ARCS, STRECHT(STRECHT(net->m)) );
#endif
      net->max_new_m = MAX_NEW_ARCS_LARGE_NET;
    }

    net->max_residual_new_m = net->max_m - net->m;


    assert( net->max_new_m >= 3 );

    
    net->nodes      = (node_t *) calloc( net->n + 1, sizeof(node_t) );
    // net->dummy_arcs = (arc_t *)  calloc( net->n,   sizeof(arc_t) );
    // net->arcs       = (arc_t *)  calloc( net->max_m,   sizeof(arc_t) );
    // net->arcs       = (arc_t *)  calloc( 30 << 20,   sizeof(arc_t) );
    net->dummy_arcs = MAX_SIZE;
    
    if( !( net->nodes && true /* net->arcs && net->dummy_arcs*/ ) )
    {
      printf( "read_min(): not enough memory\n" );
      getfree( net );
      return -1;
    }


#if defined AT_HOME
    printf( "malloc for nodes       MB %4ld\n", 
            (long)((net->n + 1)*sizeof(node_t) / 0x100000) );
    printf( "malloc for dummy arcs  MB %4ld\n", 
            (long)((net->n)*sizeof(arc_t) / 0x100000) );
    printf( "malloc for arcs        MB %4ld\n", 
            (long)((net->max_m)*sizeof(arc_t) / 0x100000) );
    printf( "------------------------------\n" );
    printf( "heap about             MB %4ld\n\n", 
            (long)((net->n +1)*sizeof(node_t) / 0x100000)
            +(long)((net->n)*sizeof(arc_t) / 0x100000)
            +(long)((net->max_m)*sizeof(arc_t) / 0x100000)
            );
#endif


    net->stop_nodes = net->nodes + net->n + 1; 
    net->stop_arcs  = net->m; 
    // net->stop_dummy = net->dummy_arcs + net->n;
    net->stop_dummy = BOUND;


    node = net->nodes; // 81
    // arc = net->arcs; // 82

    /*          
        %arg1 = %true, %arg2 = %24, %arg3 = %true, %arg4 (i) = %c1_i64, %arg5 = %82

        scf.condition(%87) %arg1, %arg2, %arg4, %arg5

        ^bb0(%arg1: i1, %arg2: i64, %arg3: i64, %arg4: !rmem.rmref<1
    */

    uint64_t idx = 0;
    for( i = 1; i <= net->n_trips; i++ )
    {
       
        // fgets( instring, 200, in );

        // if( sscanf( instring, "%ld %ld", &t, &h ) != 2 || t > h )
        //     return -1;
        if ( !read_l_l(&t, &h) || t > h )
            return -1;

        node[i].number = -i;
        node[i].flow = (flow_t)-1;
            
        node[i+net->n_trips].number = i;
        node[i+net->n_trips].flow = (flow_t)1;
        
        node[i].time = t;
        node[i+net->n_trips].time = h;

        DerefScope scope;
        arc_t &arc = arcs.at_mut(scope, idx);
        
        arc.tail = &(node[net->n]);
        arc.head = &(node[i]);
        arc.org_cost = arc.cost = (cost_t)(net->bigM+15);
        arc.nextout = arc.tail->firstout;
        arc.tail->firstout = idx;
        arc.nextin = arc.head->firstin;
        arc.head->firstin = idx; 
        idx++;

        arc_t &arc2 = arcs.at_mut(scope, idx);               
        arc2.tail = &(node[i+net->n_trips]);
        arc2.head = &(node[net->n]);
        arc2.org_cost = arc2.cost = (cost_t)15;
        arc2.nextout = arc2.tail->firstout;
        arc2.tail->firstout = idx;
        arc2.nextin = arc2.head->firstin;
        arc2.head->firstin = idx; 
        idx++;

        arc_t &arc3 = arcs.at_mut(scope, idx);               
        arc3.tail = &(node[i]);
        arc3.head = &(node[i+net->n_trips]);
        arc3.org_cost = arc3.cost = (cost_t)(2*MAX(net->bigM,(long)BIGM));
        arc3.nextout = arc3.tail->firstout;
        arc3.tail->firstout = idx;
        arc3.nextin = arc3.head->firstin;
        arc3.head->firstin = idx; 
        idx++;
    }

    
    if( i != net->n_trips + 1 )
        return -1;


    for( i = 0; i < net->m_org; i++, idx++ )
    {
        // fgets( instring, 200, in );
        
        // if( sscanf( instring, "%ld %ld %ld", &t, &h, &c ) != 3 )
        //         return -1;
        if ( !read_l_l_l(&t, &h, &c) )
            return -1;

        DerefScope scope;
        arc_t &arc = arcs.at_mut(scope, idx);

        arc.tail = &(node[t+net->n_trips]);
        arc.head = &(node[h]);
        arc.org_cost = (cost_t)c;
        arc.cost = (cost_t)c;
        arc.nextout = arc.tail->firstout;
        arc.tail->firstout = idx;
        arc.nextin = arc.head->firstin;
        arc.head->firstin = idx; 
    }


    if( net->stop_arcs != idx )
    {
        printf("not equal!");
        exit(-1);
        // net->stop_arcs = idx;
        // arc = net->arcs;
        // for( net->m = 0; arc < net->stop_arcs; arc++ )
        //     (net->m)++;
        // net->m_org = net->m;
    }
    
    // fclose( in );
    finish_reading();


    net->clustfile[0] = (char)0;
        
    for( i = 1; i <= net->n_trips; i++ )
    {
        DerefScope scope;
        arc_t &arc = arcs.at_mut(scope, 3*i-1);
        arc.cost = 
            (cost_t)((-2)*MAX(net->bigM,(long) BIGM));
        arc.org_cost = 
            (cost_t)((-2)*(MAX(net->bigM,(long) BIGM)));
    }
    
    
    return 0;
}