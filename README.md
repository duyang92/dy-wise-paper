# DyWiSE-paper
# introduction
Estimating weighted set similarity is a fundamental
task in many applications, including real-time document clus-
tering, recommendation systems, and online advertising. Exist-
ing methods (e.g., MinHash and its weighted variants) usually
assume static data with all elements and their weights known
in advance, making them unsuitable for insertion-only streams
where sets are continuously updated with new elements and
evolving weights, as in recommendation systems where users
constantly interact with new items and their interests shift over
time. To bridge the gap between prior research and practical
needs, we identify three core challenges: (i) summarizing each
set using a small memory footprint, (ii) efficiently handling weight
updates for streaming items, and (iii) supporting low-latency
online queries. To tackle these challenges, we propose DyWiSE
(Dynamic Weighted Set Similarity Estimator), the first similarity
estimator tailored to dynamic weighted sets in insertion-only
streams. Guided by the heavy-tailed nature of real-world data,
where a small fraction of heavy-weight elements dominates simi-
larity computation, DyWiSE introduces a compact heavy-element
buffer that occupies only a small portion of memory to retain
these influential elements. To accurately identify them, DyWiSE
employs a lightweight yet effective weight estimator capable of
incrementally updating and estimating the element weights with
minimal overhead. This design can provide accurate similarity
estimation while minimizing memory footprints and enabling
queries with ultra-low latency. Extensive experiments on real-
world datasets demonstrate that DyWiSE reduces memory usage
by up to 1000× compared with state-of-the-art methods while
achieving competitive accuracy. Moreover, DyWiSE supports
online queries with up to 100× lower latency and achieves an
increase in F1 score by up to 39%, significantly improving the
end-to-end performance of document deduplication and video
recommendation.
# Evaluations
DSSE is implemented using C++ in DSS2.hpp. To reproduct the experiment results in the paper, run the following command:

`/bin/bash runtest.sh
`
