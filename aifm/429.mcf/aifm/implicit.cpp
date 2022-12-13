/**************************************************************************
IMPLICIT.C of ZIB optimizer MCF, SPEC version

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
/*  LAST EDIT: Thu Feb 17 21:45:45 2005 by Andreas Loebel (boss.local.de)  */
/*  $Id: implicit.c,v 1.20 2005/02/17 21:43:12 bzfloebe Exp $  */



#include "implicit.h"



#ifdef _PROTO_
long resize_prob(arc_array_t& arcs, network_t *net )
#else
long resize_prob( net )
     network_t *net;
#endif
{
    uint64_t arc;
    // arc_t *arc;
    node_t *node, *stop, *root;
    size_t off;
            
    
    assert( net->max_new_m >= 3 );


    net->max_m += net->max_new_m;
    net->max_residual_new_m += net->max_new_m;
    

#if defined AT_HOME
    printf( "\nresize arcs to %4ld MB (%ld elements a %d B)\n\n",
            net->max_m * sizeof(arc_t) / 0x100000,
            net->max_m,
            sizeof(arc_t) );
    fflush( stdout );
#endif

    arc = 0;
    // arc = net->arcs;
    // yuanlai jiuyou: arc = (arc_t *) realloc( net->arcs, net->max_m * sizeof(arc_t) );
    if (is_nullptr(arc))
    // if( !arc )
    {
        printf( "network %s: not enough memory\n", net->inputfile );
        fflush( stdout );
        return -1;
    }
    
    off = (size_t)arc; // - (size_t)net->arcs;
        
    net->stop_arcs = net->m;
    // net->arcs = arc;
    // net->stop_arcs = arc + net->m;

    root = node = net->nodes;
    for( node++, stop = /*(void *)*/net->stop_nodes; node < stop; node++ )
        if( node->pred != root ) {
            node->basic_arc = node->basic_arc + off;
            // node->basic_arc = (arc_t *)((size_t)node->basic_arc + off);
        }
    return 0;
}







#ifdef _PROTO_
void insert_new_arc( /*arc_t *new,*/ arc_array_t& arcs, uint64_t _new, 
                    long newpos, node_t *tail, node_t *head,
                     cost_t cost, cost_t red_cost )
#else
void insert_new_arc( _new, newpos, tail, head, cost, red_cost )
     arc_t *_new;
     long newpos;
     node_t *tail;
     node_t *head;
     cost_t cost;
     cost_t red_cost;
#endif
{
    long pos;


    DerefScope scope;
    arc_t &newarc = arcs.at_mut(scope, _new + newpos);
    newarc.tail      = tail;
    newarc.head      = head;
    newarc.org_cost  = cost;
    newarc.cost      = cost;
    newarc.flow      = (flow_t)red_cost; 
    // new[newpos].tail      = tail;
    // new[newpos].head      = head;
    // new[newpos].org_cost  = cost;
    // new[newpos].cost      = cost;
    // new[newpos].flow      = (flow_t)red_cost; 
    
    pos = newpos+1;
    while( pos-1 && red_cost > (cost_t)arcs.at(scope, _new+pos/2-1).flow )
    {
        scope.renew();  
        arc_t &newarc1 = arcs.at_mut(scope, _new + pos - 1);
        const arc_t &newarc2 = arcs.at(scope, _new + pos/2 - 1);
        newarc1.tail     = newarc2.tail;
        newarc1.head     = newarc2.head;
        newarc1.cost     = newarc2.cost;
        newarc1.org_cost = newarc2.cost;
        newarc1.flow     = newarc2.flow;
        
        pos = pos/2;
        arc_t &newarc3 = arcs.at_mut(scope, _new + pos - 1);
        newarc3.tail     = tail;
        newarc3.head     = head;
        newarc3.cost     = cost;
        newarc3.org_cost = cost;
        newarc3.flow     = (flow_t)red_cost; 
    }

    // while( pos-1 && red_cost > (cost_t)new[pos/2-1].flow )
    // {
    //     new[pos-1].tail     = new[pos/2-1].tail;
    //     new[pos-1].head     = new[pos/2-1].head;
    //     new[pos-1].cost     = new[pos/2-1].cost;
    //     new[pos-1].org_cost = new[pos/2-1].cost;
    //     new[pos-1].flow     = new[pos/2-1].flow;
        
    //     pos = pos/2;
    //     new[pos-1].tail     = tail;
    //     new[pos-1].head     = head;
    //     new[pos-1].cost     = cost;
    //     new[pos-1].org_cost = cost;
    //     new[pos-1].flow     = (flow_t)red_cost; 
    // }
    
    return;
}   






#ifdef _PROTO_
void replace_weaker_arc( network_t *net, /*arc_t *new,*/
                         arc_array_t& arcs, uint64_t _new,
                         node_t *tail, node_t *head,
                         cost_t cost, cost_t red_cost )
#else
void replace_weaker_arc( net, _new, tail, head, cost, red_cost )
     network *net;
     arc_t *_new;
     node_t *tail;
     node_t *head;
     cost_t cost;
     cost_t red_cost;
#endif
{
    long pos;
    long cmp;

    DerefScope scope;
    arc_t &newarc = arcs.at_mut(scope, _new);
    newarc.tail     = tail;
    newarc.head     = head;
    newarc.org_cost = cost;
    newarc.cost     = cost;
    newarc.flow     = (flow_t)red_cost; 

    // new[0].tail     = tail;
    // new[0].head     = head;
    // new[0].org_cost = cost;
    // new[0].cost     = cost;
    // new[0].flow     = (flow_t)red_cost; 
                    
    pos = 1;
    cmp = (arcs.at(scope, _new+1).flow > arcs.at(scope, _new+2).flow) ? 2 : 3;
    // cmp = (new[1].flow > new[2].flow) ? 2 : 3;

    while( cmp <= net->max_residual_new_m && red_cost < arcs.at(scope, _new+cmp-1).flow )
    {
        scope.renew();
        arc_t &newarc1 = arcs.at_mut(scope, _new+pos-1);
        arc_t &newarc2 = arcs.at_mut(scope, _new+cmp-1);
        newarc1.tail = newarc2.tail;
        newarc1.head = newarc2.head;
        newarc1.cost = newarc2.cost;
        newarc1.org_cost = newarc2.cost;
        newarc1.flow = newarc2.flow;
        
        newarc2.tail = tail;
        newarc2.head = head;
        newarc2.cost = cost;
        newarc2.org_cost = cost;
        newarc2.flow = (flow_t)red_cost; 
        pos = cmp;
        cmp *= 2;
        if( cmp + 1 <= net->max_residual_new_m )
            if( arcs.at(scope, _new+cmp-1).flow < arcs.at(scope, _new+cmp).flow )
                cmp++;
    }

    // while( cmp <= net->max_residual_new_m && red_cost < new[cmp-1].flow )
    // {
    //     new[pos-1].tail = new[cmp-1].tail;
    //     new[pos-1].head = new[cmp-1].head;
    //     new[pos-1].cost = new[cmp-1].cost;
    //     new[pos-1].org_cost = new[cmp-1].cost;
    //     new[pos-1].flow = new[cmp-1].flow;
        
    //     new[cmp-1].tail = tail;
    //     new[cmp-1].head = head;
    //     new[cmp-1].cost = cost;
    //     new[cmp-1].org_cost = cost;
    //     new[cmp-1].flow = (flow_t)red_cost; 
    //     pos = cmp;
    //     cmp *= 2;
    //     if( cmp + 1 <= net->max_residual_new_m )
    //         if( new[cmp-1].flow < new[cmp].flow )
    //             cmp++;
    // }
    
    return;
}   




#if defined AT_HOME
#include <sys/time.h>
double Get_Time( void  ) 
{
    struct timeval tp;
    struct timezone tzp;
    if( gettimeofday( &tp, &tzp ) == 0 )
        return (double)(tp.tv_sec) + (double)(tp.tv_usec)/1.0e6;
    else
        return 0.0;
}
static double wall_time = 0; 
#endif


#ifdef _PROTO_
long price_out_impl(arc_array_t& arcs, network_t *net )
#else
long price_out_impl( net )
     network_t *net;
#endif
{
    long i;
    long trips;
    long new_arcs = 0;
    long resized = 0;
    long latest;
    long min_impl_duration = 15;

    cost_t bigM = net->bigM;
    cost_t head_potential;
    cost_t arc_cost = 30;
    cost_t red_cost;
    cost_t bigM_minus_min_impl_duration;

    uint64_t arcout_idx, arcin_idx, arcnew_idx, stop;
    uint64_t first_of_sparse_list_idx;    
    // register arc_t *arcout, *arcin, *arcnew, *stop;
    // register arc_t *first_of_sparse_list;
    node_t *tail, *head;


#if defined AT_HOME
    wall_time -= Get_Time();
#endif

    
    bigM_minus_min_impl_duration = (cost_t)bigM - min_impl_duration;
    

    
    if( net->n_trips <= MAX_NB_TRIPS_FOR_SMALL_NET )
    {
      if( net->m + net->max_new_m > net->max_m 
          &&
          (net->n_trips*net->n_trips)/2 + net->m > net->max_m
          )
      {
        // printf("Not modified!\n");
        // exit(-1);
        resized = 1;
        if( resize_prob(arcs, net ) ) {
          return -1;
          }
        
        refresh_neighbour_lists(arcs, net );
      }
    }
#if !defined SPEC_STATIC
    else
    {
      if( net->m + net->max_new_m > net->max_m 
          &&
          (net->n_trips*net->n_trips)/2 + net->m > net->max_m
          )
      {
        resized = 1;
        if( resize_prob( net ) )
          return -1;
        
        refresh_neighbour_lists( net );
      }
    }
#endif

        
    arcnew_idx = net->stop_arcs;
    // arcnew = net->stop_arcs;
    trips = net->n_trips;

    arcout_idx = 0;
    // arcout = net->arcs;
    
    {
        DerefScope scope;
        for( i = 0; i < trips && arcs.at(scope, arcout_idx+1).ident == FIXED; i++, arcout_idx += 3 ) {scope.renew();};
        first_of_sparse_list_idx = NULLPTR;
    }

    // for( i = 0; i < trips && arcout[1].ident == FIXED; i++, arcout += 3 );
    // first_of_sparse_list = (arc_t *)NULL;
    
    for( ; i < trips; i++, /*arcout += 3*/ arcout_idx += 3 )
    {
        DerefScope scope;
        arc_t &arcout = arcs.at_mut(scope, arcout_idx);
        if( arcs.at(scope, arcout_idx+1).ident != FIXED )
        {
            arcs.at_mut(scope, arcout.head->firstout).head->arc_tmp = first_of_sparse_list_idx;
            first_of_sparse_list_idx = arcout_idx + 1;
        }
        
        // if( arcout[1].ident != FIXED )
        // {
        //     arcout->head->firstout->head->arc_tmp = first_of_sparse_list;
        //     first_of_sparse_list = arcout + 1;
        // }
        
        if( arcout.ident == FIXED )
            continue;

        // if( arcout->ident == FIXED )
        //     continue;

        head = arcout.head;
        latest = head->time - arcout.org_cost 
            + (long)bigM_minus_min_impl_duration;
        // head = arcout->head;
        // latest = head->time - arcout->org_cost 
        //     + (long)bigM_minus_min_impl_duration;
                
        head_potential = head->potential;

        arcin_idx = arcs.at_mut(scope, first_of_sparse_list_idx).tail->arc_tmp;
        
        while( !is_nullptr(arcin_idx) )
        { 
            scope.renew();
            arc_t &arcin = arcs.at_mut(scope, arcin_idx);

            tail = arcin.tail;

            if( tail->time + arcin.org_cost > latest )
            {
                arcin_idx = tail->arc_tmp;
                continue;
            }
            
            red_cost = arc_cost - tail->potential + head->potential;
            
            if( red_cost < 0 )
            {
                if( new_arcs < net->max_residual_new_m )
                {
                    insert_new_arc( arcs, arcnew_idx, new_arcs, tail, head, 
                                    arc_cost, red_cost );
                    new_arcs++;                 
                }
                else if( (cost_t)arcs.at_mut(scope, arcnew_idx).flow > red_cost )
                    replace_weaker_arc( net, arcs, arcnew_idx, tail, head, 
                                        arc_cost, red_cost );
            }

            arcin_idx = tail->arc_tmp;
        }

        // arcin = first_of_sparse_list->tail->arc_tmp;
        // while( arcin )
        // {
        //     tail = arcin->tail;

        //     if( tail->time + arcin->org_cost > latest )
        //     {
        //         arcin = tail->arc_tmp;
        //         continue;
        //     }
            
        //     red_cost = arc_cost - tail->potential + head->potential;
            
        //     if( red_cost < 0 )
        //     {
        //         if( new_arcs < net->max_residual_new_m )
        //         {
        //             insert_new_arc( arcnew, new_arcs, tail, head, 
        //                             arc_cost, red_cost );
        //             new_arcs++;                 
        //         }
        //         else if( (cost_t)arcnew[0].flow > red_cost )
        //             replace_weaker_arc( net, arcnew, tail, head, 
        //                                 arc_cost, red_cost );
        //     }

        //     arcin = tail->arc_tmp;
        // }
    }

    if( new_arcs )
    {
        arcnew_idx = net->stop_arcs;
        net->stop_arcs += new_arcs;
        stop = net->stop_arcs;
        if( resized )
        {
            for( ; arcnew_idx != stop; arcnew_idx++ )
            {
                DerefScope scope;
                arc_t &arcnew = arcs.at_mut(scope, arcnew_idx);
                arcnew.flow = (flow_t)0;
                arcnew.ident = AT_LOWER;
            }
        }
        else
        {
            for( ; arcnew_idx != stop; arcnew_idx++ )
            {
                DerefScope scope;
                arc_t &arcnew = arcs.at_mut(scope, arcnew_idx);
                arcnew.flow = (flow_t)0;
                arcnew.ident = AT_LOWER;
                arcnew.nextout = arcnew.tail->firstout;
                arcnew.tail->firstout = arcnew_idx;
                arcnew.nextin = arcnew.head->firstin;
                arcnew.head->firstin = arcnew_idx;
            }
        }
        
        net->m += new_arcs;
        net->m_impl += new_arcs;
        net->max_residual_new_m -= new_arcs;

        // arcnew = net->stop_arcs;
        // net->stop_arcs += new_arcs;
        // stop = (void *)net->stop_arcs;
        // if( resized )
        // {
        //     for( ; arcnew != stop; arcnew++ )
        //     {
        //         arcnew->flow = (flow_t)0;
        //         arcnew->ident = AT_LOWER;
        //     }
        // }
        // else
        // {
        //     for( ; arcnew != stop; arcnew++ )
        //     {
        //         arcnew->flow = (flow_t)0;
        //         arcnew->ident = AT_LOWER;
        //         arcnew->nextout = arcnew->tail->firstout;
        //         arcnew->tail->firstout = arcnew;
        //         arcnew->nextin = arcnew->head->firstin;
        //         arcnew->head->firstin = arcnew;
        //     }
        // }
        
        // net->m += new_arcs;
        // net->m_impl += new_arcs;
        // net->max_residual_new_m -= new_arcs;
    }
    
#if defined AT_HOME
    wall_time += Get_Time();
    printf( "total time price_out_impl(): %0.0f\n", wall_time );
#endif


    return new_arcs;
}   






#ifdef _PROTO_
long suspend_impl(arc_array_t& arcs, network_t *net, cost_t threshold, long all )
#else
long suspend_impl( net, threshold, all )
     network_t *net;
     cost_t threshold;
     long all;
#endif
{
    long susp;
    
    cost_t red_cost;
    uint64_t idx, new_idx;
    uint64_t stop;
    // arc_t *new_arc, *arc;
    // void *stop;

    if( all )
        susp = net->m_impl;
    else
    {
        stop = net->stop_arcs;
        new_idx = net->m - net->m_impl;
        for( susp = 0, idx = new_idx; idx < stop; idx++ )
        {   
            DerefScope scope;
            const arc_t &arc = arcs.at(scope, idx);
            if( arc.ident == AT_LOWER )
                red_cost = arc.cost - arc.tail->potential 
                        + arc.head->potential;
            else
            {
                red_cost = (cost_t)-2;
                
                if( arc.ident == BASIC )
                {
                    if( arc.tail->basic_arc == idx)
                        arc.tail->basic_arc = new_idx;
                    else
                        arc.head->basic_arc = new_idx;
                }
            }
            
            if( red_cost > threshold )
                susp++;
            else
            {
                DerefScope new_scope;
                arcs.at_mut(new_scope, new_idx) = arc;
                new_idx++;
            }
        }

        // stop = (void *)net->stop_arcs;
        // new_arc = &(net->arcs[net->m - net->m_impl]);
        // for( susp = 0, arc = new_arc; arc < (arc_t *)stop; arc++ )
        // {
        //     if( arc->ident == AT_LOWER )
        //         red_cost = arc->cost - arc->tail->potential 
        //                 + arc->head->potential;
        //     else
        //     {
        //         red_cost = (cost_t)-2;
                
        //         if( arc->ident == BASIC )
        //         {
        //             if( arc->tail->basic_arc == arc )
        //                 arc->tail->basic_arc = new_arc;
        //             else
        //                 arc->head->basic_arc = new_arc;
        //         }
        //     }
            
        //     if( red_cost > threshold )
        //         susp++;
        //     else
        //     {
        //         *new_arc = *arc;
        //         new_arc++;
        //     }
        // }
    }
    
   
        
#if defined AT_HOME
    printf( "\nremove %ld arcs\n\n", susp );
    fflush( stdout );
#endif

    if( susp )
    {
        net->m -= susp;
        net->m_impl -= susp;
        net->stop_arcs -= susp;
        net->max_residual_new_m += susp;
        
        refresh_neighbour_lists(arcs, net );
    }

    return susp;
}



