#include <iostream>

#include "process/machine.hpp"
#include "vm/program.hpp"
#include "vm/state.hpp"
#include "process/process.hpp"
#include "runtime/list.hpp"
#include "process/message.hpp"
#include "mem/thread.hpp"

using namespace process;
using namespace db;
using namespace std;
using namespace vm;
using namespace boost;

namespace process
{
   
bool
machine::same_place(const node::node_id id1, const node::node_id id2) const
{
   if(id1 == id2)
      return true;
   
   remote *rem1(rout.find_remote(id1));
   remote *rem2(rout.find_remote(id2));
   
   if(rem1 != rem2)
      return false;
   
   return rem1->find_proc_owner(id1) == rem1->find_proc_owner(id2);
}

void
machine::route(process *caller, const node::node_id id, const simple_tuple* stuple)
{  
   remote* rem(rout.find_remote(id));
   sched::base *sched_caller(caller->get_scheduler());
   
   if(rem == remote::self) {
      // on this machine
      node *node(state::DATABASE->find_node(id));
      process *proc(process_list[remote::self->find_proc_owner(id)]);
      if(caller == proc)
         sched_caller->new_work(node, stuple);
      else
         sched_caller->new_work_other(proc->get_scheduler(), node, stuple);
   }
#ifdef COMPILE_MPI
   else {
      // remote, mpi machine
      
      assert(rout.use_mpi());
      
      message *msg(new message(id, stuple));
      
      sched_caller->new_work_remote(rem, rem->find_proc_owner(id), msg);
   }
#endif
}

void
machine::distribute_nodes(database *db, vector<sched::sstatic*>& schedulers)
{
   const size_t total(remote::self->get_total_nodes());
   const size_t num_procs(process_list.size());
   
   if(total < num_procs)
      throw process_exec_error("Number of nodes is less than the number of threads");
   
   size_t nodes_per_proc = remote::self->get_nodes_per_proc();
   size_t num_nodes = nodes_per_proc;
   size_t cur_proc = 0;
   database::map_nodes::const_iterator it(db->nodes_begin());
	database::map_nodes::const_iterator end(db->nodes_end());
   
   for(; it != end && cur_proc < num_procs;
         ++it)
   {
      schedulers[cur_proc]->add_node(it->second);
   
      --num_nodes;
   
      if(num_nodes == 0) {
         num_nodes = nodes_per_proc;
         ++cur_proc;
      }
   }
   
   if(it != end) {
      // put remaining nodes on last process
      --cur_proc;
      
      for(; it != end; ++it)
         schedulers[cur_proc]->add_node(it->second);
   }
}

void
machine::start(void)
{
   for(size_t i(1); i < num_threads; ++i)
      process_list[i]->start();
   process_list[0]->start();
   
   for(size_t i(1); i < num_threads; ++i)
      process_list[i]->join();

   if(will_show_database)
      state::DATABASE->print_db(cout);
   if(will_dump_database)
      state::DATABASE->dump_db(cout);
}

machine::machine(const string& file, router& _rout, const size_t th, const scheduler_type _sched_type):
   filename(file),
   num_threads(th),
   sched_type(_sched_type),
   will_show_database(false),
   will_dump_database(false),
   rout(_rout)
{  
   state::PROGRAM = new program(filename, &rout);
   state::DATABASE = state::PROGRAM->get_database();
   state::MACHINE = this;
   state::NUM_THREADS = num_threads;
   
   mem::init(num_threads);
   
   switch(sched_type) {
      case SCHED_THREADS_STATIC: {
            vector<sched::threads_static*>& schedulers(sched::threads_static::start(num_threads));
            vector<sched::sstatic*> transformed;
            
            transformed.resize(num_threads);
            process_list.resize(num_threads);
      
            for(process_id i(0); i < num_threads; ++i) {
               process_list[i] = new process(i, schedulers[i]);
               transformed[i] = (sched::sstatic*)schedulers[i];
            }
      
            distribute_nodes(state::DATABASE, transformed);
         }
         break;
      case SCHED_MPI_UNI_STATIC: {
#ifdef COMPILE_MPI
            sched::mpi_static *scheduler(sched::mpi_static::start());
            vector<sched::sstatic*> procs;
            
            process_list.resize(1);
            procs.resize(1);
            
            process_list[0] = new process(0, scheduler);
            procs[0] = (sched::sstatic*)scheduler;
            
            distribute_nodes(state::DATABASE, procs);
#endif
         }
         break;
      case SCHED_UNKNOWN:
         assert(0);
         break;
   }
}

machine::~machine(void)
{
   delete state::PROGRAM;
   
   for(process_id i(0); i != num_threads; ++i)
      delete process_list[i];
      
   mem::cleanup(num_threads);
}

}
