/**************************************************************************
PSIMPLEX.C of ZIB optimizer MCF, SPEC version

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
/*  LAST EDIT: Sun Nov 21 16:22:44 2004 by Andreas Loebel (boss.local.de)  */
/*  $Id: psimplex.c,v 1.9 2005/02/17 19:44:56 bzfloebe Exp $  */



#undef DEBUG

#include "psimplex.h"

#ifdef _PROTO_
long primal_net_simplex(arc_array_t& arcs, network_t *net )
#else
long primal_net_simplex(  net )
    network_t *net;
#endif
{
    flow_t        delta;
    flow_t        new_flow;
    long          opt = 0;
    long          xchange;
    long          new_orientation;
    node_t        *iplus;
    node_t        *jplus; 
    node_t        *iminus;
    node_t        *jminus;
    node_t        *w; 
    // arc_t         *bea;
    uint64_t      bea;
    uint64_t      bla;
    // arc_t         *bla;
    // arc_t         *arcs          = net->arcs;
    // arc_t         *stop_arcs     = net->stop_arcs;
    uint64_t      stop_arcs         = net->stop_arcs;
    node_t        *temp;
    long          m = net->m;
    long          new_set;
    cost_t        red_cost_of_bea;
    long          *iterations = &(net->iterations);
    long          *bound_exchanges = &(net->bound_exchanges);
    long          *checksum = &(net->checksum);

    while( !opt )
    {   //9365
        //7495
        // if( (bea = primal_bea_mpp( m, arcs, stop_arcs, &red_cost_of_bea )) )
        bea = primal_bea_mpp( m, arcs, stop_arcs, &red_cost_of_bea );
        if(!is_nullptr(bea))
        {
            (*iterations)++;

#ifdef DEBUG
        {
        DerefScope scope;
        const arc_t &arc = arcs.at(scope, bea);
        printf( "it %ld: bea = (%ld,%ld), red_cost = %ld\n", 
                *iterations, arc.tail->number, arc.head->number,
                red_cost_of_bea );
        }
#endif

            if( red_cost_of_bea > ZERO ) 
            {
                DerefScope scope;
                const arc_t &arc = arcs.at(scope, bea);
                iplus = arc.head;
                jplus = arc.tail;
                // iplus = bea->head;
                // jplus = bea->tail;
            }
            else 
            {
                DerefScope scope;
                const arc_t &arc = arcs.at(scope, bea);
                iplus = arc.tail;
                jplus = arc.head;
                // iplus = bea->tail;
                // jplus = bea->head;
            }

            delta = (flow_t)1;
            iminus = primal_iminus( &delta, &xchange, iplus, 
                    jplus, &w );
            if( !iminus )
            {
                (*bound_exchanges)++;
                
                DerefScope scope;
                arc_t &arc = arcs.at_mut(scope, bea);
                if( arc.ident == AT_UPPER)
                    arc.ident = AT_LOWER;
                else
                    arc.ident = AT_UPPER;

                // if( bea->ident == AT_UPPER)
                //     bea->ident = AT_LOWER;
                // else
                //     bea->ident = AT_UPPER;
                
                if( delta )
                    primal_update_flow( iplus, jplus, w );
            }
            else 
            {
                if( xchange )
                {
                    temp = jplus;
                    jplus = iplus;
                    iplus = temp;
                }
                jminus = iminus->pred;

                bla = iminus->basic_arc;
                // bla = iminus->basic_arc;
                 
                if( xchange != iminus->orientation )
                    new_set = AT_LOWER;
                else
                    new_set = AT_UPPER;

                if( red_cost_of_bea > 0 )
                    new_flow = (flow_t)1 - delta;
                else
                    new_flow = delta;

                DerefScope scope;
                arc_t &arc = arcs.at_mut(scope, bea);
                if( arc.tail == iplus )
                    new_orientation = UP;
                // if( bea->tail == iplus )
                //     new_orientation = UP;
                else
                    new_orientation = DOWN;
                update_tree( !xchange, new_orientation,
                            delta, new_flow, iplus, jplus, iminus, 
                            jminus, w, arcs, bea, /*bea, */ red_cost_of_bea,
                            (flow_t)net->feas_tol);
                arc.ident = BASIC; 
                arcs.at_mut(scope, bla).ident = new_set;
               // bea->ident = BASIC; 
                // bla->ident = new_set;
               
                if( !((*iterations-1) % 200) )
                {
                    *checksum += refresh_potential(arcs, net );
#if defined AT_HOME
                    if( *checksum > 2000000000l )
                    {
                        printf( "%ld\n", *checksum );
                        fflush(stdout);
                    }
#endif
                }   
            }
        }
        else
            opt = 1;
    }


    *checksum += refresh_potential(arcs, net );
     //printf("checksum: %ld\n", *checksum);
    primal_feasible(arcs, net );
    dual_feasible(arcs, net );
    
    return 0;
}



