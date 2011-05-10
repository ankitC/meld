
#ifndef SCHED_STATIC_LOCAL_HPP
#define SCHED_STATIC_LOCAL_HPP

#include <boost/thread/barrier.hpp>
#include <vector>

#include "sched/base.hpp"
#include "sched/node.hpp"
#include "sched/queue/safe_queue.hpp"
#include "sched/termination_barrier.hpp"

namespace sched
{

class static_local: public sched::base
{
protected:
   
   static boost::barrier *thread_barrier;
   static termination_barrier *term_barrier;
   static void threads_synchronize(void);
   static void init_barriers(const size_t);
   
   utils::byte _pad_threads1[128];
   
   enum {
      PROCESS_ACTIVE,
      PROCESS_INACTIVE
   } process_state;
   
   boost::mutex mutex;
   
   utils::byte _pad_threads2[128];
   
   safe_queue<thread_node*> queue_nodes;
   
   utils::byte _pad_threads3[128];
   
   thread_node *current_node;
   
   virtual void assert_end(void) const;
   virtual void assert_end_iteration(void) const;
   bool set_next_node(void);
   bool check_if_current_useless();
   void make_active(void);
   void make_inactive(void);
   virtual void generate_aggs(void);
   virtual bool busy_wait(void);
   
   inline void add_to_queue(thread_node *node)
   {
      queue_nodes.push(node);
   }
   
public:
   
   virtual void init(const size_t);
   
   virtual void new_work(db::node *, db::node *, const db::simple_tuple*, const bool is_agg = false);
   virtual void new_work_other(sched::base *, db::node *, const db::simple_tuple *);
   virtual void new_work_remote(process::remote *, const vm::process_id, process::message *);
   
   virtual bool get_work(work_unit&);
   virtual void finish_work(const work_unit&);
   virtual void end(void);
   virtual bool terminate_iteration(void);
   
   static_local *find_scheduler(const db::node::node_id);
   
   static db::node *create_node(const db::node::node_id id, const db::node::node_id trans)
   {
      return new thread_node(id, trans);
   }
   
   static std::vector<static_local*>& start(const size_t);
   
   explicit static_local(const vm::process_id);
   
   virtual ~static_local(void);
};

}

#endif
