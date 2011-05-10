
#ifndef SCHED_THREADED_HPP
#define SCHED_THREADED_HPP

#include <boost/thread/mutex.hpp>
#include <boost/thread/barrier.hpp>
#include <vector>

#include "utils/types.hpp"
#include "sched/base.hpp"
#include "sched/termination_barrier.hpp"

namespace sched
{

class threaded
{
private:
   
   static boost::barrier *thread_barrier;
   static termination_barrier *term_barrier;
   
   mutable enum {
      THREAD_ACTIVE,
      THREAD_INACTIVE
   } state;
   
   utils::byte _pad_threaded[128];
   
protected:
   
   static std::vector<sched::base*> ALL_THREADS;
   
   static inline void add_thread(sched::base *add)
   {
      ALL_THREADS[add->get_id()] = add;
   }
   
   static inline void threads_synchronize(void)
   {
      thread_barrier->wait();
   }
   static void init_barriers(const size_t);
   
   boost::mutex mutex;

   inline void set_active(void)
   {
      assert(state == THREAD_INACTIVE);
      state = THREAD_ACTIVE;
      term_barrier->is_active();
   }
   
   inline void set_inactive(void)
   {
      assert(state == THREAD_ACTIVE);
      state = THREAD_INACTIVE;
      term_barrier->is_inactive();
   }
   
   inline const bool is_inactive(void) const { return state == THREAD_INACTIVE; }
   inline const bool is_active(void) const { return state == THREAD_ACTIVE; }
   
   inline const bool all_threads_finished(void) const
   {
      return term_barrier->all_finished();
   }
   
public:
   
   explicit threaded(void): state(THREAD_ACTIVE)
   {
   }
   
   virtual ~threaded(void)
   {
      assert(state == THREAD_INACTIVE);
   }
};

}

#endif
