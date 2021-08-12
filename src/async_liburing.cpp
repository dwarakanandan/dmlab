#include "async_liburing.h"

using namespace std;

Result_t async_liburing(const RuntimeArgs_t& args) {
    size_t buffer_size = 1024 * args.blk_size;
    uint64_t ops_submitted = 0, ops_returned = 0, ops_failed = 0;
    bool isRead = (args.operation == READ);
	bool isRand = (args.opmode == RANDOM);

    struct io_uring ring;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    struct iovec *iovecs;
    __kernel_timespec timeout;
    char* buffer[args.oio];
    int ret;

    iovecs = (iovec*) calloc(args.oio, sizeof(struct iovec));
    for (int i = 0; i < args.oio; i++)
    {
        buffer[i] = (char *) aligned_alloc(1024, buffer_size);
	    memset(buffer[i], '0', buffer_size);
        iovecs[i].iov_base = buffer[i];
        iovecs[i].iov_len = buffer_size;
    }

    /* Initialize io_uring */
    ret = io_uring_queue_init(2, &ring, 0);
    cout << "iouring init " << ret <<endl;
    if (ret < 0) {
        perror(getErrorMessageWithTid(args, "io_uring_queue_init"));
        return return_error();
    }

	double start = getTime();
	while (getTime() - start < RUN_TIME) {
        /* Get a Submission Queue Entry */
        sqe = io_uring_get_sqe(&ring);
        if (sqe == NULL) {
            fprintf(stderr, "io_uring_get_sqe failed\n");
            return return_error();
        }
        off_t offset = getOffset(args.read_offset, buffer_size, ops_submitted, isRand);
        isRead? io_uring_prep_readv(sqe, args.fd, iovecs, args.oio, offset):
                io_uring_prep_writev(sqe, args.fd, iovecs, args.oio, offset);

        /* Submit the request with args.oio operations */
        ret = io_uring_submit(&ring);
        ops_submitted+= args.oio;

        /* Wait for IO request to complete */
        timeout.tv_sec = 1;
        ret = io_uring_wait_cqe_timeout(&ring, &cqe, &timeout);
        if (ret < 0) {
            perror(getErrorMessageWithTid(args, "io_uring_wait_cqe"));
            return return_error();
        }

        /* Check completion event result code */
        if (cqe->res < 0) {
            ops_failed+= args.oio;
        }
        io_uring_cqe_seen(&ring, cqe);

        ops_returned+= args.oio;
    }

    /* Cleanup io_uring */
    io_uring_queue_exit(&ring);

    Result_t results;
    results.throughput = calculateThroughputGbps(ops_returned-ops_failed, buffer_size);
    results.op_count = ops_returned-ops_failed;
	results.ops_submitted = ops_submitted;
	results.ops_returned = ops_returned;
	results.ops_failed = ops_failed;

	if (args.debugInfo) printOpStats(args, results);
    return results;
}