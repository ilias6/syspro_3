all: worker diseaseAggregator whoServer whoClient

whoClient: client.o clientFun.o list.o record.o heap.o hash.o avl.o functions.o
	gcc -o whoClient client.o clientFun.o list.o record.o heap.o hash.o avl.o functions.o -pthread
client.o: client.c
	gcc -c client.c
clientFun.o: clientFun.c
	gcc -c clientFun.c

whoServer: whoServer.o whoServerFun.o list.o record.o heap.o hash.o avl.o functions.o stack.o
	gcc -o whoServer whoServer.o whoServerFun.o list.o record.o heap.o hash.o avl.o functions.o stack.o -g3 -pthread
whoServer.o: whoServer.c
	gcc -c whoServer.c -g3
whoServerFun.o: whoServerFun.c
	gcc -c whoServerFun.c -g3
stack.o: stack.c
	gcc -c stack.c -g3

diseaseAggregator: diseaseAggregator.o param.o aggregator_fun.o record.o list.o functions.o heap.o hash.o avl.o
	gcc -o diseaseAggregator diseaseAggregator.o param.o aggregator_fun.o record.o list.o functions.o heap.o hash.o avl.o -g3 -pthread
diseaseAggregator.o: diseaseAggregator.c
	gcc -c diseaseAggregator.c
param.o: param.c
	gcc -c param.c
aggregator_fun.o: aggregator_fun.c
	gcc -c aggregator_fun.c

worker: list.o record.o avl.o worker.o heap.o hash.o functions.o worker_fun.o
	gcc -o worker list.o record.o avl.o worker.o heap.o hash.o functions.o worker_fun.o -g3 -pthread
worker.o: worker.c
	gcc -c worker.c
functions.o: functions.c
	gcc -c functions.c
list.o: list.c
	gcc -c list.c
record.o: record.c
	gcc -c record.c
avl.o: avl.c
	gcc -c avl.c
heap.o: heap.c
	gcc -c heap.c
hash.o: hash.c
	gcc -c hash.c

cleanO:
		rm *.o
cleanLog:
		rm ./log_files/*
cleanAll:
		rm worker diseaseAggregator *.o
