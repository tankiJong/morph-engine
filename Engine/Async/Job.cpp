#include "Job.hpp"
#include <vector>
#include <mutex>
#include "Thread.hpp"
using namespace Job;
class Worker;

class JobQueue {
public:
  void enqueue(S<Decl> decl);
protected:
  std::vector<Decl> mDecls;
  std::mutex mLock;
};

class Worker {
public:
  void start();
  void join();
  void detach();
  bool isRunning();
  static void workerEntryPoint(Worker* worker);
protected:
  bool mIsRunning = false;
  Thread* mThread = nullptr;
};

class JobCenter {
public:
  void startup();
  void issueJob(Decl& decl);
  void claimJob(Worker& worker);
  void shutdown();
protected:
  std::vector<Worker> mWorkers;
  std::vector<JobQueue> mQueues;
};


static JobCenter gJobCenter;

void freeWorker(Worker* worker) {
  delete worker;
}

void JobCenter::startup() {
  for(uint i = 0; i < Job::kMaxWorker; i++) {
    Worker& wk = mWorkers.emplace_back();
    wk.start();
  }
}

void JobCenter::shutdown() {
  
}

void Worker::start() {
  mIsRunning = true;
  mThread = new Thread(workerEntryPoint, this);
}

void Worker::join() {
  mIsRunning = false;
}

void Worker::workerEntryPoint(Worker* worker) {
  while(worker->isRunning()) {
    
  }
}

namespace Job {
  std::atomic<counter_id_t> Counter::sNextId = 0;
};



void Job::startup() {
  gJobCenter.startup();
}

void Job::shutdown() {
  gJobCenter.shutdown();
}

//W<Job::Counter> Job::dispatch(Decl& decl) {
//
//}
