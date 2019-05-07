#include "Job.hpp"
#include <mutex>
#include "Thread.hpp"
#include "Engine/Memory/Pool.hpp"
#include <queue>
#include "Engine/Core/Time/Clock.hpp"
using namespace Job;

class JobQueue {
public:
  JobQueue() = default;
  JobQueue(const JobQueue& q)
	  : mCategory(q.mCategory) {}
  S<Counter> enqueue(const S<Counter>& counter);
  S<Counter> dequeue();
  void category(category_t cat) { mCategory = cat; }

  auto clear();

protected:
  category_t mCategory;
  std::queue<S<Counter>> mCounters;
  std::mutex mLock;
};

class JobCenter {
public:

  ~JobCenter();
  S<Counter> claimJob(category_t category);
  S<Counter> createJob(const S<Decl>& decl, category_t cat);
  S<Counter> issueJob(const S<Counter>& counter);
  S<Counter> issueJob(const S<Decl>& decl, category_t cat);
  void shutdown();
  void startup(uint categoryCount);
  bool opening() const { return mIsOpening; }

protected:
  std::vector<JobQueue> mQueues;
  std::vector<Thread> mSystemJobThreads;
  static void systemThreadEntry(category_t category);
  bool mIsOpening = false;
};


static JobCenter gJobCenter;
thread_local Counter* gCurrentJob = nullptr;










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
  mSystemJobThreads.emplace_back("Job Generic", systemThreadEntry, CAT_GENERIC);
  mSystemJobThreads.emplace_back("Job GenericSlow", systemThreadEntry, CAT_GENERIC_SLOW);
  mSystemJobThreads.emplace_back("Job GenericSlow", systemThreadEntry, CAT_GENERIC_SLOW);
  mSystemJobThreads.emplace_back("Job IO", systemThreadEntry, CAT_IO);
}


void JobCenter::systemThreadEntry(category_t category) {
  Consumer consumer;
  category_t categories[1] = { category };
  consumer.init(categories);
  while(running()) {
    consumer.consumeAll();
    CurrentThread::yield();
  }
}

S<Counter> JobQueue::enqueue(const S<Counter>& counter) {
  std::scoped_lock lock(mLock);
  mCounters.push(counter);
  return counter;
}

S<Counter> JobQueue::dequeue() {
  std::scoped_lock lock(mLock);
  if(mCounters.size() == 0) return nullptr;
  S<Counter> counter = mCounters.front();
  mCounters.pop();
  return counter;
}

auto JobQueue::clear() {
  std::scoped_lock lock(mLock);
  while(!mCounters.empty()) {
    S<Counter>& counter = mCounters.front();
    counter->terminate();
    mCounters.pop();
  }
}

JobCenter::~JobCenter() {
  for(Thread& t: mSystemJobThreads) {
    t.join();
  }
}

S<Counter> JobCenter::issueJob(const S<Decl>& decl, category_t cat) {
  S<Counter> counter = createJob(decl, cat);
  counter->decrementCounter();
  EXPECTS(counter->counter() == 0);
  JobQueue& q = mQueues[cat];
  return q.enqueue(counter);
}

S<Counter> JobCenter::claimJob(category_t category) {
  JobQueue& q = mQueues[category];
  return q.dequeue();
}

S<Counter> JobCenter::createJob(const S<Decl>& decl, category_t cat) {
  S<Counter> counter = std::make_shared<Counter>(decl, cat);
  return counter;
}

S<Counter> JobCenter::issueJob(const S<Counter>& counter) {
  JobQueue& q = mQueues[counter->category()];
  return q.enqueue(counter);
}

void JobCenter::shutdown() {
  mIsOpening = false;
  for(JobQueue& queue: mQueues) {
    queue.clear();
  }
}


namespace Job {
  std::atomic<counter_id_t> Counter::sNextId = 0;
};

void Counter::dispatchBlockees() const {
  for(uint i = 0; i < mBlockeeCount; i++) {
    S<Counter> blockee = mBlockees[i];
    dispatch(blockee);
  }
}

Counter::~Counter() {
  for(uint i = 0; i < mBlockeeCount; i++) {
    mBlockees[i]->terminate();
  }
}

void Counter::invoke() {
  mDecl->execute();
  dispatchBlockees();
  mIsDone = true;
}

void Counter::reset() {
  mDecl = nullptr;
  mDispatchCounter = 1;
  mBlockees.fill(nullptr);
  mBlockeeCount = 0;
}

void Counter::addBlockee(const S<Counter>& counter) {
  mBlockees[mBlockeeCount++] = counter;
  counter->mDispatchCounter++;
}

void Consumer::init(span<category_t> categories) {
  mCategories.clear();
  mCategories.insert(mCategories.begin(), categories.begin(), categories.end());
}

bool Consumer::consume() {
  for(category_t category: mCategories) {
    S<Counter> counter = gJobCenter.claimJob(category);
    if(counter != nullptr) {
      gCurrentJob = counter.get();
      counter->invoke();
      gCurrentJob = nullptr;
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

void Consumer::consumeFor(uint ms) {
  uint start = GetMainClock().total.millisecond;

  while(consume()) {
    if(GetMainClock().total.millisecond >= start + ms) break;
  }
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

Counter* Job::currentJob() {
  return gCurrentJob;
}

S<Counter> Job::create(Decl& decl, category_t cat) {
  return create(std::move(decl), cat);
}

S<Counter> Job::create(Decl&& decl, category_t cat) {
  S<Decl> realDecl = S<Decl>(new Decl(decl));
  return gJobCenter.createJob(realDecl, cat);
}

void Job::dispatch(const S<Counter>& counter) {
  counter->decrementCounter();
  if(counter->counter() == 0) {
    gJobCenter.issueJob(counter);
  }
}

W<Counter> Job::dispatch(Decl& decl,  category_t cat) {
  return dispatch(std::move(decl), cat);
}

W<Counter> Job::dispatch(Decl&& decl, category_t cat) {
  S<Decl> realDecl = std::make_shared<Decl>(decl);
  return gJobCenter.issueJob(realDecl, cat);
}

void Job::chain(const S<Counter>& prerequisite, const S<Counter>& afterFinish) {
  EXPECTS(afterFinish != nullptr);
  prerequisite->addBlockee(afterFinish);
}

bool Job::ready() {
  return gJobCenter.opening();
}


void Job::wait(W<Counter> counter, float maxTimeSecond) {
  float start = (float)GetMainClock().total.second;
  // see whether someone is holding it
  S<Counter> c = counter.lock();
  if(c == nullptr) return;

  while(!c->done() && (float)GetMainClock().total.second < start + maxTimeSecond) {
    CurrentThread::yield();
  };
}
