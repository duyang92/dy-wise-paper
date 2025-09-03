all: test_icws_HTWE test_fast_icws_HTWE test_bagminhash_HTWE test_dartminhash_HTWE test_dss2_SWE

test_icws_HTWE: main.cpp algs/icws.hpp datareader.cpp Makefile
	g++ -std=c++11 -lstdc++fs -DICWS_SKETCH -DQUERY_SIZE=1000 -DHTWE -O3 -o test_icws_HTWE main.cpp

test_fast_icws_HTWE: main.cpp algs/icws.hpp datareader.cpp Makefile
	g++ -std=c++11 -lstdc++fs -DFAST_ICWS_SKETCH -DQUERY_SIZE=1000 -DHTWE -O3 -o test_fast_icws_HTWE main.cpp

test_bagminhash_HTWE: main.cpp algs/bagminhash_wrappers.hpp datareader.cpp Makefile
	g++ -std=c++14  -DBAGMINHASH_SKETCH -DHTWE -DQUERY_SIZE=1000 -DXXH_INLINE_ALL -O3 -o test_bagminhash_HTWE main.cpp algs/bagminhash/xxhash/libxxhash.a

test_dartminhash_HTWE: main.cpp algs/dartminhash.hpp datareader.cpp algs/HashTabWeightEst.h Makefile
	g++ -std=c++11 -lstdc++fs -DDARTMINHASH_SKETCH -DQUERY_SIZE=1000 -DHTWE -O3 -o test_dartminhash_HTWE main.cpp

test_dss2_SWE_LAZY: main.cpp algs/vHLL.h datareader.cpp algs/DSS2.hpp algs/SketchWeightEst.h Makefile
	g++ -std=c++14 -lstdc++fs -DDSS2 -DLAZY -DSWE -DQUERY_SIZE=1000 -O3 -o test_dss2_SWE_LAZY main.cpp
test_dss2_SWE: main.cpp algs/vHLL.h datareader.cpp algs/DSS2.hpp algs/SketchWeightEst.h Makefile
	g++ -std=c++14 -lstdc++fs -DDSS2 -DSWE -DQUERY_SIZE=1000 -O3 -o test_dss2_SWE main.cpp
test_icws_SWE: main.cpp algs/icws.hpp datareader.cpp Makefile
	g++ -std=c++11 -lstdc++fs -DICWS_SKETCH -DSWE -DQUERY_SIZE=1000 -O3 -o test_icws_SWE main.cpp
test_fast_icws_SWE: main.cpp algs/icws.hpp datareader.cpp Makefile
	g++ -std=c++11 -lstdc++fs -DFAST_ICWS_SKETCH -DSWE -DQUERY_SIZE=1000 -O3 -o test_fast_icws_SWE main.cpp
test_bagminhash_SWE: main.cpp algs/bagminhash_wrappers.hpp datareader.cpp Makefile
	g++ -std=c++14  -DBAGMINHASH_SKETCH -DSWE -DQUERY_SIZE=1000 -DXXH_INLINE_ALL -O3 -o test_bagminhash_SWE main.cpp algs/bagminhash/xxhash/libxxhash.a
test_dartminhash_SWE: main.cpp algs/dartminhash.hpp datareader.cpp algs/HashTabWeightEst.h Makefile
	g++ -std=c++11 -lstdc++fs -DDARTMINHASH_SKETCH -DSWE -DQUERY_SIZE=1000 -O3 -o test_dartminhash_SWE main.cpp
	
clear:
	# 删除所有 test 开头的可执行文件
	rm -f test_*
