/**************************************************************************
MCF.H of ZIB optimizer MCF, SPEC version

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
/*  LAST EDIT: Thu Feb 17 22:10:51 2005 by Andreas Loebel (boss.local.de)  */
/*  $Id: mcf.c,v 1.15 2005/02/17 21:43:12 bzfloebe Exp $  */



#include "mcf.h"
#include <time.h>
#include <stdint.h>

constexpr static uint64_t kCacheMBs = 552;
constexpr static uint64_t kCacheSize = (kCacheMBs << 20);
constexpr static uint64_t kFarMemSize = (690ULL << 20); // (1ULL << 30);
constexpr static uint32_t kNumGCThreads = 40;
constexpr static uint32_t kNumConnections = 300;

static inline uint64_t _microtime() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    uint64_t t = ts.tv_sec*1000*1000 + ts.tv_nsec/1000;
    return t;
}

#define REPORT

extern long min_impl_duration;
network_t net;

#ifdef _PROTO_
long global_opt(arc_array_t &arcs)
#else
long global_opt( )
#endif
{
    long new_arcs;
    long residual_nb_it;
    

    new_arcs = -1;
    residual_nb_it = net.n_trips <= MAX_NB_TRIPS_FOR_SMALL_NET ?
        MAX_NB_ITERATIONS_SMALL_NET : MAX_NB_ITERATIONS_LARGE_NET;

    while( new_arcs )
    {
#ifdef REPORT
        printf( "active arcs                : %ld\n", net.m );
#endif

        primal_net_simplex(arcs, &net );


#ifdef REPORT
        printf( "simplex iterations         : %ld\n", net.iterations );
        printf( "objective value            : %0.0f\n", flow_cost(arcs, &net) );
#endif


#if defined AT_HOME
        printf( "%ld residual iterations\n", residual_nb_it );
#endif

        if( !residual_nb_it )
            break;


        if( net.m_impl )
        {
          new_arcs = suspend_impl(arcs, &net, (cost_t)-1, 0 );

#ifdef REPORT
          if( new_arcs )
            printf( "erased arcs                : %ld\n", new_arcs );
#endif
        }


        new_arcs = price_out_impl(arcs, &net );

#ifdef REPORT
        if( new_arcs )
            printf( "new implicit arcs          : %ld\n", new_arcs );
#endif
        
        if( new_arcs < 0 )
        {
#ifdef REPORT
            printf( "not enough memory, exit(-1)\n" );
#endif

            exit(-1);
        }

#ifndef REPORT
        printf( "\n" );
#endif


        residual_nb_it--;
    }

    printf( "checksum                   : %ld\n", net.checksum );

    return 0;
}




#ifdef _PROTO_
void do_work(FarMemManager *manager)
#else
int main( argc, argv )
    int argc;
    char *argv[];
#endif
{
    printf( "\nMCF SPEC CPU2006 version 1.10\n" );
    printf( "Copyright (c) 1998-2000 Zuse Institut Berlin (ZIB)\n" );
    printf( "Copyright (c) 2000-2002 Andreas Loebel & ZIB\n" );
    printf( "Copyright (c) 2003-2005 Andreas Loebel\n" );
    printf( "\n" );



    memset( (void *)(&net), 0, (size_t)sizeof(network_t) );
    net.bigM = (long)BIGM;

    char path[100] = "/mnt/AIFM/aifm/429.mcf/data/train/input/inp.in";

    strcpy( net.inputfile, path);
    
    auto net_arcs = manager->allocate_array<arc_t, BOUND>();
  
    if( read_min(net_arcs, &net ) )
    {
        printf( "read error, exit\n" );
        getfree( &net );
        return;
    }


#ifdef REPORT
    printf( "nodes                      : %ld\n", net.n_trips );
#endif

    uint64_t start = _microtime();

    primal_start_artificial(net_arcs, &net);
    global_opt(net_arcs);

    uint64_t end  = _microtime();
    printf("Time: %lu\n", end - start);

    return;

#ifdef REPORT
    printf( "done\n" );
#endif

    
    if( write_circulations( "mcf.out", net_arcs, &net ) )
    {
        getfree( &net );
        return;    
    }

    getfree( &net );
    return;
}



int argc;
void _main(void *arg) {
  char **argv = static_cast<char **>(arg);
  std::string ip_addr_port(argv[1]);
  auto raddr = helpers::str_to_netaddr(ip_addr_port);
  std::unique_ptr<FarMemManager> manager =
      std::unique_ptr<FarMemManager>(FarMemManagerFactory::build(
          kCacheSize, kNumGCThreads,
          new TCPDevice(raddr, kNumConnections, kFarMemSize)));
  do_work(manager.get());
}

int main(int _argc, char *argv[]) {
  std::cout << "sizeof: " << sizeof(arc_t) << ' ' << sizeof(node_t) << std::endl;
  
  int ret;

  if (_argc < 3) {
    std::cerr << "usage: [cfg_file] [ip_addr:port]" << std::endl;
    return -EINVAL;
  }

  char conf_path[strlen(argv[1]) + 1];
  strcpy(conf_path, argv[1]);
  for (int i = 2; i < _argc; i++) {
    argv[i - 1] = argv[i];
  }
  argc = _argc - 1;

  ret = runtime_init(conf_path, _main, argv);
  if (ret) {
    std::cerr << "failed to start runtime" << std::endl;
    return ret;
  }

  return 0;
}

