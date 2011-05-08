
#ifndef SCHED_STEALER_HPP
#define SCHED_STEALER_HPP

#include <vector>
#include <tr1/unordered_set>

#include "sched/base.hpp"
#include "sched/node.hpp"
#include "sched/queue.hpp"

namespace sched
{

class stealer: public sched::base
{
private:
   
   utils::byte _pad_threads1[128];
   
   enum {
      PROCESS_ACTIVE,
      PROCESS_INACTIVE
   } process_state;
   
   utils::byte _pad_threads2[128];
   
   wqueue<work_unit> queue_work;
   boost::mutex mutex;
   
   utils::byte _pad_threads3[128];
   
   typedef std::tr1::unordered_set<db::node*, std::tr1::hash<db::node*>, std::equal_to<db::node*>, mem::allocator<db::node*> > node_set;
   
   node_set *nodes;
   boost::mutex *nodes_mutex;
   
   utils::byte _pad_threads4[128];
   
   typedef wqueue_free<work_unit> queue_work_free;
   std::vector<queue_work_free, mem::allocator<queue_work_free> > buffered_work;
   
   virtual void assert_end(void) const;
   virtual void assert_end_iteration(void) const;
   bool all_buffers_emptied(void) const;
   void make_active(void);
   void make_inactive(void);
   void flush_this_queue(wqueue_free<work_unit>&, stealer *);
   void generate_aggs(void);
   bool busy_wait(void);
   void flush_buffered(void);
   void add_node(db::node *);
   void remove_node(db::node *);
   stealer *select_steal_target(void) const;
   
public:
   
   virtual void init(const size_t);
   
   virtual void new_work(db::node *, const db::simple_tuple*, const bool is_agg = false);
   virtual void new_work_other(sched::base *, db::node *, const db::simple_tuple *);
   virtual void new_work_remote(process::remote *, const vm::process_id, process::message *);
   
   virtual bool get_work(work_unit&);
   virtual void finish_work(const work_unit&);
   virtual void end(void);
   virtual bool terminate_iteration(void);
   
   stealer *find_scheduler(const db::node::node_id);
   
   static db::node *create_node(const db::node::node_id id, const db::node::node_id trans)
   {
      return new thread_node(id, trans);
   }
   
   static std::vector<stealer*>& start(const size_t);
   
   explicit stealer(const vm::process_id);
   
   virtual ~stealer(void);
};

}

#endif