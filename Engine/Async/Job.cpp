#include "Job.hpp"
#include <vector>
#include <mutex>
#include "Thread.hpp"
#include "Engine/Memory/Pool.hpp"
using namespace Job;

class JobQueue {
public:
  JobQueue() = default;
  JobQueue(const JobQueue& q)
	  : mCategory(q.mCategory) {}
  S<Counter> enqueue(const S<Decl>& decl);
  S<Counter> dequeue();
  void category(category_t cat) { mCategory = cat; }
protected:
  category_t mCategory;
  std::vector<S<Counter>> mCounters;
  std::mutex mLock;
};

class JobCenter {
public:

  ~JobCenter();
  S<Counter> issueJob(const S<Decl>& decl, category_t cat);
  S<Counter> claimJob(category_t category);
  void shutdown();
  void startup(uint categoryCount);
  bool opening() const { return mIsOpening; }

protected:
  std::vector<JobQueue> mQueues;
  std::vector<Thread> mSystemJobThreads;
  static void systemThreadEntry(uint category);
  bool mIsOpening = false;
};


static JobCenter gJobCenter;

void JobCenter::startup(uint categoryCount) {
  mIsOpening = true;

  mQueues.resize(categoryCount);
  for(uint i = 0; i < categoryCount; i++) {
    // create job queues
    JobQueue& q = mQueues[i];
    q.category((category_t)i);
  }

  // create job threads
  mSystemJobThreads.emplace_back("Job Generic", systemThreadEntry, CAT_GENERIC);
  mSystemJobThreads.emplace_back("Job Generic", systemThreadEntry, CAT_GENERIC);
  mSystemJobThreads.emplace_back("Job GenericSlow", systemThreadEntry, CAT_GENERIC_SLOW);
  mSystemJobThreads.emplace_back("Job IO", systemThreadEntry, CAT_IO);
}

void JobCenter::systemThreadEntry(uint category) {
  Consumer consumer;
  category_t categories[1] = { category };
  consumer.init(categories);
  while(running()) {
    consumer.consumeAll();
    CurrentThread::yield();
  }
}

S<Counter> JobQueue::enqueue(const S<Decl>& decl) {
  std::scoped_lock lock(mLock);
  S<Counter> counter = std::make_shared<Counter>(decl);
  mCounters.push_back(counter);
  counter->decrementCounter();
  return counter;
}

S<Counter> JobQueue::dequeue() {
  std::scoped_lock lock(mLock);
  for(uint i = 0; i < mCounters.size(); i++) {
    S<Counter>& counter = mCounters[i];
    if(counter->counter() == 0) {
      std::swap(counter, mCounters.back());
      S<Counter> c = mCounters.back();
      mCounters.pop_back();
      return c;
    }
  }
  return nullptr;
}

JobCenter::~JobCenter() {
  for(Thread& t: mSystemJobThreads) {
    t.join();
  }
}

S<Counter> JobCenter::issueJob(const S<Decl>& decl, category_t cat) {
  JobQueue& q = mQueues[cat];
  return q.enqueue(decl);
}

S<Counter> JobCenter::claimJob(category_t category) {
  JobQueue& q = mQueues[category];
  return q.dequeue();
}

void JobCenter::shutdown() {
  mIsOpening = false;
}


namespace Job {
  std::atomic<counter_id_t> Counter::sNextId = 0;
};

void Counter::invoke() {
  mDecl->execute();
  for(uint i = 0; i < mBlockeeCount; i++) {
    Counter* blockee = mBlockees[i];
    blockee->decrementCounter();
  }
}

void Counter::reset() {
  mDecl = nullptr;
  mDispatchCounter = 1;
  mBlockees.fill(nullptr);
  mBlockeeCount = 0;
}

void Consumer::init(span<category_t> categories) {
  mCategories.clear();
  mCategories.insert(mCategories.begin(), categories.begin(), categories.end());
}

bool Consumer::consume() {
  for(category_t category: mCategories) {
    S<Counter> counter = gJobCenter.claimJob(category);
    if(counter != nullptr) {
      counter->invoke();
      return true;
    }
  }

  return false;
}

bool Consumer::consumeAll() {
  bool consumed = false;
  do {
    consumed = consume();
  } while(consumed);

  return consumed;
}

void Job::startup(uint categoryCount) {
  gJobCenter.startup(categoryCount);
}

void Job::shutdown() {
  gJobCenter.shutdown();
}

bool Job::running() {
  return gJobCenter.opening();
}

W<Counter> Job::dispatch(Decl& decl,  category_t cat) {
  S<Decl> realDecl = std::make_shared<Decl>(std::move(decl));
  return gJobCenter.issueJob(realDecl, cat);
}

