/**************************************************************************
MCFUTIL.C of ZIB optimizer MCF, SPEC version

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
/*  LAST EDIT: Thu Feb 17 21:46:06 2005 by Andreas Loebel (boss.local.de)  */
/*  $Id: mcfutil.c,v 1.11 2005/02/17 21:43:12 bzfloebe Exp $  */



#include "mcfutil.h"



#ifdef _PROTO_
void refresh_neighbour_lists(arc_array_t& arcs, network_t *net )
#else
void refresh_neighbour_lists( net )
    network_t *net;
#endif
{
    node_t *node;
    // arc_t *arc;
    uint64_t idx;
    // void *stop;
    
    node = net->nodes;
    for(void* stop = (void *)net->stop_nodes; node < (node_t *)stop; node++ )
    {
        node->firstin = 0; // (arc_t *)NULL;
        node->firstout = 0; // (arc_t *)NULL;
    }
    idx = 0;
    // arc = net->arcs;
    for(uint64_t stop = /*(void *)*/net->stop_arcs; idx < /*(arc_t *)*/stop; idx++ )
    {
        DerefScope scope;
        arc_t &arc = arcs.at_mut(scope, idx);
        arc.nextout = arc.tail->firstout;
        arc.tail->firstout = idx;
        arc.nextin = arc.head->firstin;
        arc.head->firstin = idx;

        // arc->nextout = arc->tail->firstout;
        // arc->tail->firstout = arc;
        // arc->nextin = arc->head->firstin;
        // arc->head->firstin = arc;
    }
    
    return;
}










#ifdef _PROTO_
long refresh_potential(arc_array_t& arcs, network_t *net )
#else
long refresh_potential( net )
    network_t *net;
#endif
{
    node_t *node, *tmp;
    node_t *root = net->nodes;
    long checksum = 0;
    

    root->potential = (cost_t) -MAX_ART_COST;
    tmp = node = root->child;
    while( node != root )
    {
        while( node )
        {
            if( node->orientation == UP ) {
                DerefScope scope;
                const arc_t &arc = arcs.at(scope, node->basic_arc);
                node->potential = arc.cost + node->pred->potential;    
                // node->potential = node->basic_arc->cost + node->pred->potential;
            }
            else /* == DOWN */
            {
                DerefScope scope;
                const arc_t &arc = arcs.at(scope, node->basic_arc);
                node->potential = node->pred->potential - arc.cost;
                // node->potential = node->pred->potential - node->basic_arc->cost;
                checksum++;
            }

            tmp = node;
            node = node->child;
        }
        
        node = tmp;

        while( node->pred )
        {
            tmp = node->sibling;
            if( tmp )
            {
                node = tmp;
                break;
            }
            else
                node = node->pred;
        }
    }
    return checksum;
}







#ifdef _PROTO_
double flow_cost(arc_array_t& arcs, network_t *net )
#else
double flow_cost( net )
    network_t *net;
#endif
{
    uint64_t idx;
    // arc_t *arc;
    node_t *node;
    void *stop;
    uint64_t stop_arc;
    long fleet = 0;
    cost_t operational_cost = 0;
    

    stop_arc = net->stop_arcs;
    for(idx = 0; idx != stop_arc; idx++ )
    {
        DerefScope scope;
        arc_t &arc = arcs.at_mut(scope, idx);
        if( arc.ident == AT_UPPER )
            arc.flow = (flow_t)1;
        else
            arc.flow = (flow_t)0;
    }
    // stop = (void *)net->stop_arcs;
    // for( arc = net->arcs; arc != (arc_t *)stop; arc++ )
    // {
    //     if( arc->ident == AT_UPPER )
    //         arc->flow = (flow_t)1;
    //     else
    //         arc->flow = (flow_t)0;
    // }
    

    stop = net->stop_nodes;
    for( node = net->nodes, node++; node != (node_t *)stop; node++ ) {
        DerefScope scope;
        arc_t &arc = arcs.at_mut(scope, node->basic_arc);
        arc.flow = node->flow;
        // node->basic_arc->flow = node->flow;
    }


    stop_arc = net->stop_arcs;
    for( idx = 0; idx != stop_arc; idx++ )
    {
        DerefScope scope;
        const arc_t &arc = arcs.at(scope, idx);
        if( arc.flow )
        {
            if( !(arc.tail->number < 0 && arc.head->number > 0) )
            {
                if( !arc.tail->number )
                {
                    operational_cost += (arc.cost - net->bigM);
                    fleet++;
                }
                else
                    operational_cost += arc.cost;
            }
        }

    } 
    // stop = (void *)net->stop_arcs;
    // for( arc = net->arcs; arc != (arc_t *)stop; arc++ )
    // {
    //     if( arc->flow )
    //     {
    //         if( !(arc->tail->number < 0 && arc->head->number > 0) )
    //         {
    //             if( !arc->tail->number )
    //             {
    //                 operational_cost += (arc->cost - net->bigM);
    //                 fleet++;
    //             }
    //             else
    //                 operational_cost += arc->cost;
    //         }
    //     }

    // }
    
    return (double)fleet * (double)net->bigM + (double)operational_cost;
}










#ifdef _PROTO_
double flow_org_cost(arc_array_t& arcs, network_t *net )
#else
double flow_org_cost( net )
    network_t *net;
#endif
{
    // arc_t *arc;
    uint64_t idx;
    node_t *node;
    void *stop;
    uint64_t stop_arc;
    
    long fleet = 0;
    cost_t operational_cost = 0;
    

    stop_arc = net->stop_arcs;
    for( idx = 0; idx != stop_arc; idx++ )
    { 
        DerefScope scope;
        arc_t &arc = arcs.at_mut(scope, idx);
        if( arc.ident == AT_UPPER )
            arc.flow = (flow_t)1;
        else
            arc.flow = (flow_t)0;
    }

    // stop = (void *)net->stop_arcs;
    // for( arc = net->arcs; arc != (arc_t *)stop; arc++ )
    // {
    //     if( arc->ident == AT_UPPER )
    //         arc->flow = (flow_t)1;
    //     else
    //         arc->flow = (flow_t)0;
    // }

    stop = (void *)net->stop_nodes;
    for( node = net->nodes, node++; node != (node_t *)stop; node++ ) {
        DerefScope scope;
        arc_t &arc = arcs.at_mut(scope, node->basic_arc);
        arc.flow = node->flow;
    }
    // stop = (void *)net->stop_nodes;
    // for( node = net->nodes, node++; node != (node_t *)stop; node++ )
    //     node->basic_arc->flow = node->flow;
    
    stop_arc = net->stop_arcs;
    for( idx = 0; idx != stop_arc; idx++ )
    {
        DerefScope scope;
        const arc_t &arc = arcs.at(scope, idx);
        if( arc.flow )
        {
            if( !(arc.tail->number < 0 && arc.head->number > 0) )
            {
                if( !arc.tail->number )
                {
                    operational_cost += (arc.org_cost - net->bigM);
                    fleet++;
                }
                else
                    operational_cost += arc.org_cost;
            }
        }
    }
    // stop = (void *)net->stop_arcs;
    // for( arc = net->arcs; arc != (arc_t *)stop; arc++ )
    // {
    //     if( arc->flow )
    //     {
    //         if( !(arc->tail->number < 0 && arc->head->number > 0) )
    //         {
    //             if( !arc->tail->number )
    //             {
    //                 operational_cost += (arc->org_cost - net->bigM);
    //                 fleet++;
    //             }
    //             else
    //                 operational_cost += arc->org_cost;
    //         }
    //     }
    // }
    
    return (double)fleet * (double)net->bigM + (double)operational_cost;
}










#ifdef _PROTO_
long primal_feasible(arc_array_t& arcs, network_t *net )
#else
long primal_feasible( net )
    network_t *net;
#endif
{
    void *stop;
    node_t *node;
    uint64_t dummy = net->dummy_arcs;
    uint64_t stop_dummy = net->stop_dummy;
   // arc_t *dummy = net->dummy_arcs;
    // arc_t *stop_dummy = net->stop_dummy;
    // arc_t *arc;
    uint64_t idx;
    flow_t flow;
    

    node = net->nodes;
    stop = (void *)net->stop_nodes;

    for( node++; node < (node_t *)stop; node++ )
    {
        // arc = node->basic_arc;
        idx = node->basic_arc;
        flow = node->flow;
        if( /*arc*/ idx >= dummy && /*arc*/ idx < stop_dummy )
        {
            if( ABS(flow) > (flow_t)net->feas_tol )
            {
                printf( "PRIMAL NETWORK SIMPLEX: " );
                printf( "artificial arc with nonzero flow, node %d (%ld)\n",
                        node->number, flow );
            }
        }
        else
        {
            if( flow < (flow_t)(-net->feas_tol)
               || flow - (flow_t)1 > (flow_t)net->feas_tol )
            {
                printf( "PRIMAL NETWORK SIMPLEX: " );
                printf( "basis primal infeasible (%ld)\n", flow );
                net->feasible = 0;
                return 1;
            }
        }
    }
    
    net->feasible = 1;
    
    return 0;
}








#ifdef _PROTO_
long dual_feasible(arc_array_t& arcs,  network_t *net )
#else
long dual_feasible(  net )
    network_t *net;
#endif
{
    // arc_t         *arc;
    uint64_t          idx;
    // arc_t         *stop     = net->stop_arcs;
    uint64_t         stop     = net->stop_arcs;
    cost_t        red_cost;
    
    

    for (idx = 0; idx < stop; idx++)
    // for( arc = net->arcs; arc < stop; arc++ )
    {
        DerefScope scope;
        const arc_t &arc = arcs.at(scope, idx);
        red_cost = arc.cost - arc.tail->potential 
            + arc.head->potential;
        
        // red_cost = arc->cost - arc->tail->potential 
        //     + arc->head->potential;
        
        switch( arc.ident )
        // switch( arc->ident )
        {
        case BASIC:
#ifdef AT_ZERO
        case AT_ZERO:
            if( ABS(red_cost) > (cost_t)net->feas_tol )
#ifdef DEBUG
            printf("no debug!\n");
              //  printf("%d %d %d %ld\n", arc->tail->number, arc->head->number,
                       arc->ident, red_cost );
#else
                // goto DUAL_INFEAS;
                fprintf( stderr, "DUAL NETWORK SIMPLEX: " );
                fprintf( stderr, "basis dual infeasible\n" );
                return 1;
#endif
            
            break;
#endif
        case AT_LOWER:
            if( red_cost < (cost_t)-net->feas_tol )
#ifdef DEBUG
                printf("%d %d %d %ld\n", arc->tail->number, arc->head->number,
                       arc->ident, red_cost );
#else
                // goto DUAL_INFEAS;
                fprintf( stderr, "DUAL NETWORK SIMPLEX: " );
                fprintf( stderr, "basis dual infeasible\n" );
                return 1;
#endif

            break;
        case AT_UPPER:
            if( red_cost > (cost_t)net->feas_tol )
#ifdef DEBUG
                printf("%d %d %d %ld\n", arc->tail->number, arc->head->number,
                       arc->ident, red_cost );
#else
                // goto DUAL_INFEAS;
                fprintf( stderr, "DUAL NETWORK SIMPLEX: " );
                fprintf( stderr, "basis dual infeasible\n" );
                return 1;
#endif

            break;
        case FIXED:
        default:
            break;
        }
    }
    
    return 0;
    
DUAL_INFEAS:
    fprintf( stderr, "DUAL NETWORK SIMPLEX: " );
    fprintf( stderr, "basis dual infeasible\n" );
    return 1;
}







#ifdef _PROTO_
long getfree( 
            network_t *net
            )
#else
long getfree( net )
     network_t *net;
#endif
{  
    FREE( net->nodes );
    // FREE( net->arcs );
    FREE( net->dummy_arcs );
    net->nodes = net->stop_nodes = NULL;
    net->dummy_arcs = net->stop_dummy = NULLPTR;
    // net->arcs = net->stop_arcs = NULL;
    // net->dummy_arcs = net->stop_dummy = NULL;

    return 0;
}



