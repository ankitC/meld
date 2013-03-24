
#ifndef SCHED_NODES_SIM_HPP
#define SCHED_NODES_SIM_HPP

#include "mem/base.hpp"
#include "db/tuple.hpp"
#include "utils/spinlock.hpp"
#include "sched/base.hpp"
#include "queue/intrusive.hpp"
#include "queue/safe_simple_pqueue.hpp"
#include "queue/safe_linear_queue.hpp"

namespace sched
{ 

class sim_node: public db::node
{
public:
	
   std::map<int, vm::node_val> faces;
	size_t timestamp;
	queue::heap_queue<db::simple_tuple*> tuple_pqueue; // for deterministic computation
	queue::push_safe_linear_queue<db::simple_tuple*> pending; // for fast computation

   inline int get_face(const vm::node_val node) const
   {
      for(std::map<int, vm::node_val>::const_iterator it(faces.begin()), end(faces.end()); it != end; it++) {
         if(it->second == node)
            return it->first;
      }

      return -1;
   }

   inline vm::node_val get_node_at_face(const int face) const
   {
      assert(faces.find(face) != faces.end());
      return faces.find(face)->second;
   }
   
	inline bool has_work(void) const { return !tuple_pqueue.empty(); }

   virtual void assert_end(void) const
   {
      assert(!has_work());
   }

   virtual void assert_end_iteration(void) const
   {
      assert(!has_work());
   }

   explicit sim_node(const db::node::node_id _id, const db::node::node_id _trans, vm::all *all):
		db::node(_id, _trans, all), timestamp(0)
   {
		tuple_pqueue.set_type(HEAP_INT_ASC);
	}

   virtual ~sim_node(void) { }
};
   
}

#endif