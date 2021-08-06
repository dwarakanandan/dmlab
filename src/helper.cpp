#include "helper.h"

using namespace std;

const std::string SEQUENTIAL = "SEQUENTIAL";
const std::string RANDOM = "RANDOM";

const std::string READ = "READ";
const std::string WRITE = "WRITE";

void printStats(const RuntimeArgs_t& args, const Result_t results) {
    std::stringstream stats;
    stats << "TID:" << args.thread_id
        << " offset:" << args.read_offset / _1GB << "GB"
        << " ops:" << results.op_count
        << " throughput:" << results.throughput << " GB/s" << endl << endl;
    cout << stats.str();
}

void printOpStats(const RuntimeArgs_t& args, const Result_t results) {
	std::stringstream msg;
	msg << "TID:" << args.thread_id
		<< " OP_SUBMIT: " << results.ops_submitted << " "
		<< " OP_RETURNED: " << results.ops_returned << " "
		<< " OP_FAILED: " << results.ops_failed << endl
        << " offset:" << args.read_offset / _1GB << "GB"
        << " ops:" << results.op_count
        << " throughput:" << results.throughput << " GB/s" << endl << endl;
	cout << msg.str();
}

const char* getErrorMessageWithTid(const RuntimeArgs_t& args, std::string error) {
	std::stringstream msg;
	msg << "TID:" << args.thread_id << " "<< error;
	return msg.str().c_str();
}

void fileOpen(RuntimeArgs_t *args) {
    args->fd = open(args->filename.c_str(), O_RDWR | O_CREAT | O_DIRECT, S_IRUSR | S_IWUSR);
    if (args->fd < 0) {
        perror("Open error");
        exit(-1);
    }
}

void runBenchmark(RuntimeArgs_t& userArgs, Result_t (*benchmarkFunction)(const RuntimeArgs_t& args)) {
    std::vector<std::future<Result_t>> threads;
    for (int i = 1; i <= userArgs.thread_count; ++i) {
        RuntimeArgs_t args;
        args.thread_id = i;
        args.blk_size = userArgs.blk_size;
        args.fd = userArgs.fd;
        args.debugInfo = userArgs.debugInfo;
        args.read_offset = (_100GB * i) % MAX_READ_OFFSET;
        args.operation = userArgs.operation;
        args.opmode = userArgs.opmode;
        args.oio = userArgs.oio;
        threads.push_back(std::async(benchmarkFunction, args));
    }

    double totalThroughput = 0;
    uint64_t totalOps = 0;
    for (auto& t : threads) {
        auto results = t.get();
        totalThroughput += results.throughput;
        totalOps += results.op_count;
    }

    int oioPrint = (userArgs.lib == SYNCIO) ? 1 : userArgs.oio;

    cout << std::fixed
        << userArgs.operation.substr(0,1) << " " <<  userArgs.opmode.substr(0,1) << " "
        << "BLK_SIZE_KB:" << userArgs.blk_size << " "
        << "OIO:" << oioPrint << " "
        << "OP_COUNT:" << totalOps << " "
        << "THROUGHPUT_GBPS:" << totalThroughput << endl;
}

double calculateThroughputGbps(uint64_t ops, size_t buffer_size) {
    return ((ops * buffer_size)/(1024.0*1024*1024 * RUN_TIME));
}

off_t getOffset(off_t initialOffset, size_t buffer_size, uint64_t iteration, bool isRand) {
    return isRand ?
        initialOffset + (rand() * buffer_size) % _100GB:
        initialOffset + (iteration * buffer_size) % _100GB;
}

Result_t return_error() {
	Result_t results;
	results.throughput = 0;
	results.op_count = 0;
	return results;
}