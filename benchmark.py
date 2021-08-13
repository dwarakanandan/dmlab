import os

base_command = './build/benchmark --file /dev/md127 '

libs = []
libs.append('syncio')
libs.append('libaio')
libs.append('iouring')

ops = []
ops.append('read')
ops.append('write')

threads = []
threads.append(1)
threads.append(8)

bsizes = [2, 4, 8, 16, 64, 128, 512, 1024, 2048, 102400]

oio_sizes = [2, 4, 8, 16, 64, 128, 256, 512, 1024]

outputs_global = []

def runBenchmarkAllOios(lib, threads, op, mode, bsize):
    print('Running lib:' + lib + ' threads:' + str(threads) + ' op:' + op + ' mode:' + mode + ' bsize:' + str(bsize))

    for oio in oio_sizes:
        tputs = []
        for i in range(0, 10):
            stream = os.popen(base_command + '--threads ' + str(threads) + ' --bsize ' + str(bsize) + ' --oio ' + str(oio) + ' --op ' + op + ' --mode ' + mode + ' --lib ' + lib)
            split = stream.readline().strip().split(' ')
            for s in split:
                if s.startswith('TPUT_GBPS'):
                    tputs.append(float(s.split(':')[1]))
        print(tputs)
        print(sum(tputs)/len(tputs))

    print()
        

def runBenchmarkAllBlks(lib, threads, op, mode):
    print('Running lib:' + lib + ' threads:' + str(threads) + ' op:' + op + ' mode:' + mode)
    outputs = []
    for bsize in bsizes:
        stream = os.popen(base_command + '--threads ' + str(threads) + ' --bsize ' + str(bsize) + ' --op ' + op + ' --mode ' + mode + ' --lib ' + lib)
        outputs.append(stream.readline().strip())
    outputs_global.append(outputs)
    for output in outputs:
        print(output)
    print()

def runBenchmark(lib, threads, op, mode, bsize):
    # print('Running lib:' + lib + ' threads:' + str(threads) + ' op:' + op + ' mode:' + mode + ' bsize:' + str(bsize))
    outputs = []
    stream = os.popen(base_command + '--threads ' + str(threads) + ' --bsize ' + str(bsize) + ' --op ' + op + ' --mode ' + mode + ' --lib ' + lib)
    outputs.append(stream.readline().strip())
    outputs_global.append(outputs)
    for output in outputs:
        print(output)

def mainBenchmark():
    for thread in threads:
        for lib in libs:
            for op in ops:
                runBenchmarkAllBlks(lib, thread, op, 'seq')
                runBenchmarkAllBlks(lib, thread, op, 'rand')

def basicSanityTest():
    for thread in [1, 8]:
        runBenchmark('syncio', thread, 'read', 'seq', 4)
        runBenchmark('libaio', thread, 'read', 'seq', 4)
        runBenchmark('iouring', thread, 'read', 'seq', 4)
        print()
        runBenchmark('syncio', thread, 'read', 'rand', 4)
        runBenchmark('libaio', thread, 'read', 'rand', 4)
        runBenchmark('iouring', thread, 'read', 'rand', 4)
        print()
        runBenchmark('syncio', thread, 'write', 'seq', 4)
        runBenchmark('libaio', thread, 'write', 'seq', 4)
        runBenchmark('iouring', thread, 'write', 'seq', 4)
        print()
        runBenchmark('syncio', thread, 'write', 'rand', 4)
        runBenchmark('libaio', thread, 'write', 'rand', 4)
        runBenchmark('iouring', thread, 'write', 'rand', 4)
        print()

# basicSanityTest()

runBenchmarkAllOios('libaio' , 1, 'read', 'rand', 4)
runBenchmarkAllOios('iouring' , 1, 'read', 'rand', 4)
# runBenchmarkAllOios('libaio' , 4, 'read', 'rand', 4)
# runBenchmarkAllOios('iouring' , 4, 'read', 'rand', 4)