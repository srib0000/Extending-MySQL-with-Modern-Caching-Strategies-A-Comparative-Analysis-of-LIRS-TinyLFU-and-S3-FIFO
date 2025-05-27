Extending-MySQL-with-Modern-Caching-Strategies-A-Comparative-Analysis-of-LIRS-TinyLFU-and-S3-FIFO

Project Title
Extending MySQL with Modern Caching Strategies: A Comparative Analysis of LIRS, TinyLFU, and S3-FIFO for Query Performance Optimization

Course
CS 5513: Database Management Systems  
Spring 2025 — University of Oklahoma

Authors:
Sasank Sribhashyam — sasank.sribhashyam-1@ou.edu  
Venkat Tarun Adda — adda0003@ou.edu  
Chandana Sree Kamasani — chandana.sree.kamasani-1@ou.edu

Project Overview:

MySQL 8.0 removed its native Query Cache due to scalability and invalidation limitations. This project addresses that performance gap by designing and integrating a modular query result caching framework inside MySQL using three modern caching strategies:

- LIRS (Low Inter-reference Recency Set)
- TinyLFU (Tiny Least Frequently Used)
- S3-FIFO (Segmented Three-Queue FIFO)

Our system enhances MySQL’s performance for read-heavy and mixed workloads by caching normalized query results using pluggable strategies. This framework is lightweight, ACID-compliant, thread-safe, and extensible.

Objectives:

1. Design a pluggable query caching interface within MySQL  
2. Implement and evaluate LIRS, TinyLFU, and S3-FIFO 
3. Embed the caching layer between parsing and execution
4. Support query fingerprinting and normalization for semantic equivalence  
5. Measure performance via TPC-H and SysBench benchmarks

System Architecture:

1. Query Normalization
   SQL queries are transformed into normalized fingerprints by canonicalizing whitespace, literals, and structure.

2. Caching Framework 
   Positioned between MySQL’s parser and execution engine, intercepts queries, performs lookup, and serves cached results.

3. Pluggable Strategies  
   LIRS: Recency & reuse-distance aware  
   TinyLFU: Frequency-based probabilistic admission  
   S3-FIFO: Queue promotion-based adaptability

4. Thread-Safe Integration
   Fine-grained locking ensures cache consistency in multi-threaded environments.

5. Non-Intrusive Design
   No modifications to InnoDB or transactional components. Preserves MySQL’s ACID guarantees.

Experimental Setup

Benchmarks:
  - [TPC-H](https://www.tpc.org/tpch/) for analytical workloads  
  - [SysBench](https://github.com/akopytov/sysbench) for transactional loads

Metrics Evaluated:
  - Cache hit ratio  
  - Query execution latency  
  - Cache memory usage  
  - Cache pollution and promotion rates

Key Results

| Strategy  | Best Scenario          | Max Cache Hit | Avg. Latency ↓ | Notes                          |
|-----------|------------------------|---------------|----------------|--------------------------------|
| LIRS      | Repetitive OLAP Queries| 85%           | ~25%           | Best for long-term reuse       |
| TinyLFU   | Mixed workloads        | 80%           | ~22%           | Filters low-value entries      |
| S3-FIFO   | Bursty transactional   | 83%           | ~28%           | Concurrency & burst friendly   |

All strategies outperformed MySQL’s default (InnoDB-only) behavior by significantly reducing redundant execution.

Prerequisites:

- MySQL 8.0 source code  
- CMake 3.18+  
- GCC/G++ or Clang  
- Linux/macOS environment (tested on Ubuntu 22.04)

Installation Steps:

Clone MySQL source
git clone https://github.com/mysql/mysql-server.git
cd mysql-server

Copy cache layer code
cp -r /path/to/cache_layer ./sql/

Apply necessary hooks in sql_parse.cc and sql_executor.cc

Configure and build
cmake . -DDOWNLOAD_BOOST=1 -DWITH_BOOST=boost
make -j$(nproc)

Run MySQL with custom cache enabled
./bin/mysqld --enable-custom-cache --cache-strategy=tinylfu

-- Enable cache
SET GLOBAL enable_custom_cache = ON;

-- Set strategy: 'lirs', 'tinylfu', or 's3fifo'
SET GLOBAL cache_strategy = 'lirs';

-- Set cache size (in MB)
SET GLOBAL cache_size = 512;

-- Check cache performance
SHOW STATUS LIKE 'cache_%';

Limitations & Future Work:

- Current normalization does not fully handle subquery equivalences
- Limited to SELECT queries (DML/DDL caching unsupported)

Future enhancements:
- ML-guided cache admission
- Hybrid strategy combining LIRS + TinyLFU
- PostgreSQL or MariaDB support

License: This project is intended for academic use within the University of Oklahoma. For other use cases, please contact the authors.

References

- Jiang & Zhang, “LIRS: An Efficient Low Inter-reference Recency Set Replacement Policy”
- Einziger et al., “TinyLFU: A Highly Efficient Cache Admission Policy”
- Yang et al., “S3-FIFO: An Adaptively Segmented Cache Replacement Algorithm”


