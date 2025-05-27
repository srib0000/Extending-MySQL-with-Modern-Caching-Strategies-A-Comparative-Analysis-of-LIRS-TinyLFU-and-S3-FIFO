#include <iostream>
#include <string>
#include <unordered_map>
#include <deque>
#include <map>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cstdlib>
#include <climits>
#include <vector>

// Query Processing Components

class QueryParser
{
public:
    std::string parse(const std::string &query)
    {
        // Return the query in lowercase for uniformity.
        std::string parsed = query;
        std::transform(parsed.begin(), parsed.end(), parsed.begin(), ::tolower);
        return parsed;
    }
};

class QueryOptimizer
{
public:
    std::string optimize(const std::string &parsed_query)
    {
        // Simulate generating an optimized execution plan.
        return "OptimizedPlan(" + parsed_query + ")";
    }
};

class ExecutionEngine
{
public:
    std::string execute(const std::string &plan)
    {
        // Simulate query execution delay.
        std::this_thread::sleep_for(std::chrono::milliseconds(150 + std::rand() % 150));
        return "Result for " + plan;
    }
};

class TransactionManager
{
public:
    void begin()
    {
        std::cout << "Transaction started.\n";
    }
    void commit()
    {
        std::cout << "Transaction committed.\n";
    }
    void rollback()
    {
        std::cout << "Transaction rolled back.\n";
    }
};

class LockManager
{
public:
    void acquire(const std::string &resource)
    {
        std::cout << "Lock acquired on " << resource << ".\n";
    }
    void release(const std::string &resource)
    {
        std::cout << "Lock released on " << resource << ".\n";
    }
};

// Base Cache Strategy

class CacheStrategy
{
public:
    int capacity;
    std::unordered_map<std::string, std::string> cache;
    int cache_hits;
    int cache_misses;

    CacheStrategy(int cap) : capacity(cap), cache_hits(0), cache_misses(0) {}
    virtual ~CacheStrategy() {}

    virtual std::string get(const std::string &query)
    {
        if (cache.find(query) != cache.end())
        {
            cache_hits++;
            update(query);
            return cache[query];
        }
        else
        {
            cache_misses++;
            return "";
        }
    }

    virtual void put(const std::string &query, const std::string &result)
    {
        if (cache.find(query) != cache.end())
        {
            update(query);
        }
        else
        {
            if (cache.size() >= (size_t)capacity)
            {
                evict();
            }
            cache[query] = result;
            admit(query);
        }
    }

    virtual void admit(const std::string &query) = 0;
    virtual void update(const std::string &query) = 0;
    virtual void evict() = 0;

    virtual void stats()
    {
        std::cout << "Cache Hits: " << cache_hits << "\n";
        std::cout << "Cache Misses: " << cache_misses << "\n";
        std::cout << "Current Cache Size: " << cache.size() << "\n";
        std::cout << "Cached Queries:\n";
        for (const auto &entry : cache)
        {
            std::cout << " - " << entry.first << "\n";
        }
    }
};

// LIRS Cache Implementation (Low Inter-reference Recency)

class LIRSCache : public CacheStrategy
{
    std::deque<std::string> high_interference_list; // High reuse queries
    std::deque<std::string> low_interference_list;  // Low reuse queries
    std::unordered_map<std::string, bool> in_high;

public:
    LIRSCache(int cap) : CacheStrategy(cap) {}

    void admit(const std::string &query) override
    {
        high_interference_list.push_back(query);
        in_high[query] = true;
    }

    void update(const std::string &query) override
    {
        if (in_high[query])
        {
            // Move to front to make it the most recently used in high-interference list.
            high_interference_list.erase(std::remove(high_interference_list.begin(), high_interference_list.end(), query), high_interference_list.end());
            high_interference_list.push_back(query);
        }
        else
        {
            // Promote to high-interference list.
            low_interference_list.erase(std::remove(low_interference_list.begin(), low_interference_list.end(), query), low_interference_list.end());
            high_interference_list.push_back(query);
            in_high[query] = true;
        }
    }

    void evict() override
    {
        std::string victim;
        if (!high_interference_list.empty())
        {
            victim = high_interference_list.front();
            high_interference_list.pop_front();
        }
        else if (!low_interference_list.empty())
        {
            victim = low_interference_list.front();
            low_interference_list.pop_front();
        }
        else if (!cache.empty())
        {
            victim = cache.begin()->first;
        }
        if (!victim.empty())
        {
            cache.erase(victim);
            in_high.erase(victim);
            std::cout << "LIRS Evicted: " << victim << "\n";
        }
    }
};

// TinyFLU Cache Implementation (Tiny First Look Up)

class TinyFLUCache : public CacheStrategy
{
    std::deque<std::string> query_queue;

public:
    TinyFLUCache(int cap) : CacheStrategy(cap) {}

    void admit(const std::string &query) override
    {
        query_queue.push_back(query);
    }

    void update(const std::string &query) override
    {
        // Move query to the end of the deque, indicating it was recently used.
        query_queue.erase(std::remove(query_queue.begin(), query_queue.end(), query), query_queue.end());
        query_queue.push_back(query);
    }

    void evict() override
    {
        if (!query_queue.empty())
        {
            std::string victim = query_queue.front();
            query_queue.pop_front();
            cache.erase(victim);
            std::cout << "TinyFLU Evicted: " << victim << "\n";
        }
    }
};

/// S3-FIFO Cache Implementation
class S3FIFOCache : public CacheStrategy
{
    std::deque<std::string> short_term;
    std::deque<std::string> medium_term;
    std::deque<std::string> long_term;

public:
    S3FIFOCache(int cap) : CacheStrategy(cap) {}

    void admit(const std::string &query) override
    {
        // New queries go to short-term queue.
        short_term.push_back(query);
    }

    void update(const std::string &query) override
    {
        // Promote queries across queues.
        auto it = std::find(short_term.begin(), short_term.end(), query);
        if (it != short_term.end())
        {
            short_term.erase(it);
            medium_term.push_back(query);
            return;
        }
        it = std::find(medium_term.begin(), medium_term.end(), query);
        if (it != medium_term.end())
        {
            medium_term.erase(it);
            long_term.push_back(query);
            return;
        }
        it = std::find(long_term.begin(), long_term.end(), query);
        if (it != long_term.end())
        {
            long_term.erase(it);
            long_term.push_back(query); // Refresh position.
        }
    }

    void evict() override
    {
        std::string victim;
        if (!short_term.empty())
        {
            victim = short_term.front();
            short_term.pop_front();
        }
        else if (!medium_term.empty())
        {
            victim = medium_term.front();
            medium_term.pop_front();
        }
        else if (!long_term.empty())
        {
            victim = long_term.front();
            long_term.pop_front();
        }
        else if (!cache.empty())
        {
            victim = cache.begin()->first;
        }
        if (!victim.empty())
        {
            cache.erase(victim);
            std::cout << "S3-FIFO Evicted: " << victim << "\n";
        }
    }
};

// Database System Simulation with Extended Cache Strategies

class DatabaseSystem
{
    QueryParser parser;
    QueryOptimizer optimizer;
    ExecutionEngine engine;
    TransactionManager tx_manager;
    LockManager lock_manager;
    CacheStrategy *cache_strategy;

public:
    DatabaseSystem() : cache_strategy(new LIRSCache(5)) {}

    ~DatabaseSystem()
    {
        delete cache_strategy;
    }

    std::string trim(const std::string &str)
    {
        size_t first = str.find_first_not_of(' ');
        size_t last = str.find_last_not_of(' ');
        if (first == std::string::npos || last == std::string::npos)
            return "";
        return str.substr(first, (last - first + 1));
    }

    void print_raw_input_info(const std::string &input)
    {
        std::cout << "DEBUG: Raw input: '" << input << "'\n";
        std::cout << "DEBUG: Input length: " << input.length() << "\n";

        // Display the ASCII values of each character in the input string
        std::cout << "DEBUG: ASCII values of input characters: ";
        for (char c : input)
        {
            std::cout << static_cast<int>(c) << " ";
        }
        std::cout << "\n";
    }

    void set_cache_strategy(const std::string &strategy)
    {
        delete cache_strategy; // Remove current strategy

        std::string strat = strategy;

        // Print raw input before any transformation for detailed inspection
        print_raw_input_info(strat);

        // Trim leading/trailing spaces and convert to lowercase for case-insensitive comparison
        strat = trim(strat);
        std::transform(strat.begin(), strat.end(), strat.begin(), ::tolower);

        std::cout << "DEBUG: Processed strategy (after trim and lowercase): '" << strat << "'\n";

        // Modify comparison to accept both s3fifo and s3-fifo
        if (strat == "lirs")
        {
            cache_strategy = new LIRSCache(5);
            std::cout << "Caching strategy set to LIRS.\n";
        }
        else if (strat == "tinyflu")
        {
            cache_strategy = new TinyFLUCache(5);
            std::cout << "Caching strategy set to TinyFLU.\n";
        }
        else if (strat == "s3fifo" || strat == "s3-fifo") // Accept both formats
        {
            cache_strategy = new S3FIFOCache(5);
            std::cout << "Caching strategy set to S3-FIFO.\n";
        }
        else
        {
            std::cout << "Invalid caching strategy selected. Defaulting to LIRS.\n";
            delete cache_strategy;
            cache_strategy = new LIRSCache(5);
        }
    }
    // Process the query and return the result.
    std::string process_query(const std::string &query)
    {
        std::string parsed_query = parser.parse(query);
        std::string plan = optimizer.optimize(parsed_query);

        // Check cache first.
        std::string cached_result = cache_strategy->get(query);
        if (!cached_result.empty())
        {
            std::cout << "Cache hit!\n";
            return cached_result;
        }
        else
        {
            std::cout << "Cache miss! Executing query...\n";
            lock_manager.acquire("table");
            std::cout << "Lock acquired on table.\n";
            tx_manager.begin();
            std::string result = engine.execute(plan);
            tx_manager.commit();
            lock_manager.release("table");
            cache_strategy->put(query, result);
            return result;
        }
    }

    void show_cache_stats()
    {
        cache_strategy->stats();
    }

    void run_benchmark()
    {
        std::string queries[] = {
            "SELECT * FROM employees",
            "SELECT * FROM orders WHERE order_id = 100",
            "SELECT name FROM customers WHERE city = 'New York'",
            "SELECT * FROM orders",
            "SELECT COUNT(*) FROM sales",
            "SELECT * FROM employees",                  // duplicate to test cache hit
            "SELECT * FROM orders WHERE order_id = 100" // duplicate
        };

        std::cout << "Running benchmark...\n";
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto &query : queries)
        {
            process_query(query);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Benchmark completed in " << duration.count() << " seconds.\n";
    }
};

// Menu Driven Application

void print_menu()
{
    std::cout << "\n====== Database Management System Simulation ======\n";
    std::cout << "1. Set Caching Strategy (LIRS, TinyFLU, S3-FIFO)\n";
    std::cout << "2. Enter and Process SQL Query\n";
    std::cout << "3. Run Benchmark Simulation\n";
    std::cout << "4. Show Cache Statistics\n";
    std::cout << "5. Exit\n";
    std::cout << "=====================================================\n";
}

int main()
{
    DatabaseSystem db_system;
    std::string choice;

    while (true)
    {
        print_menu();
        std::cout << "Enter your choice: ";
        std::getline(std::cin, choice);

        if (choice == "1")
        {
            std::cout << "Enter caching strategy (LIRS/TinyFLU/S3-FIFO): ";
            std::string strat;
            std::getline(std::cin, strat);
            db_system.set_cache_strategy(strat);
        }
        else if (choice == "2")
        {
            std::cout << "Enter SQL query: ";
            // query examples:
            // "SELECT * FROM users", "UPDATE users SET age = 30 WHERE id = 1"
            // "DELETE FROM users WHERE id = 2", "INSERT INTO users (name, age) VALUES ('Alice', 25)"
            // "SELECT name FROM users WHERE age > 20", "SELECT * FROM orders WHERE amount > 1000"
            // "UPDATE orders SET status = 'shipped' WHERE id = 3", "DELETE FROM orders WHERE id = 4"
            // "INSERT INTO orders (product, amount) VALUES ('Laptop', 1500)", "SELECT product FROM orders WHERE amount < 500"
            // "SELECT * FROM products WHERE price > 100", "UPDATE products SET stock = 50 WHERE id = 5"
            // "DELETE FROM products WHERE id = 6", "INSERT INTO products (name, price) VALUES ('Phone', 700)"
            // "SELECT name FROM products WHERE price < 300", "SELECT * FROM customers WHERE city = 'New York'"
            // "UPDATE customers SET status = 'active' WHERE id = 7", "DELETE FROM customers WHERE id = 8"
            // "INSERT INTO customers (name, city) VALUES ('Bob', 'Los Angeles')", "SELECT name FROM customers WHERE city = 'Chicago'"
            // "SELECT * FROM transactions WHERE amount > 1000", "UPDATE transactions SET status = 'completed' WHERE id = 9"
            // "DELETE FROM transactions WHERE id = 10", "INSERT INTO transactions (user_id, amount) VALUES (1, 2000)"
            // "SELECT user_id FROM transactions WHERE amount < 500", "SELECT * FROM reviews WHERE rating > 4"
            // "UPDATE reviews SET status = 'approved' WHERE id = 11", "DELETE FROM reviews WHERE id = 12"
            // "INSERT INTO reviews (product_id, rating) VALUES (1, 5)", "SELECT product_id FROM reviews WHERE rating < 3"
            // "SELECT * FROM feedback WHERE score > 4", "UPDATE feedback SET status = 'resolved' WHERE id = 13"
            // "DELETE FROM feedback WHERE id = 14", "INSERT INTO feedback (user_id, score) VALUES (2, 5)"
            // "SELECT user_id FROM feedback WHERE score < 3", "SELECT * FROM logs WHERE level = 'error'"
            // "UPDATE logs SET status = 'archived' WHERE id = 15", "DELETE FROM logs WHERE id = 16"
            // "INSERT INTO logs (message, level) VALUES ('Error occurred', 'error')", "SELECT message FROM logs WHERE level = 'info'"
            // "SELECT * FROM notifications WHERE status = 'unread'", "UPDATE notifications SET status = 'read' WHERE id = 17"
            // "DELETE FROM notifications WHERE id = 18", "INSERT INTO notifications (user_id, message) VALUES (3, 'New message')"
            // "SELECT user_id FROM notifications WHERE status = 'read'", "SELECT * FROM alerts WHERE severity = 'high'"
            // Print the query examples to the user for reference
            std::cout << "Query Examples:\n";
            std::cout << " - SELECT * FROM users\n";
            std::cout << " - UPDATE users SET age = 30 WHERE id = 1\n";
            std::cout << " - DELETE FROM users WHERE id = 2\n";
            std::cout << " - INSERT INTO users (name, age) VALUES ('Alice', 25)\n";
            std::cout << " - SELECT name FROM users WHERE age > 20\n";
            std::cout << " - SELECT * FROM orders WHERE amount > 1000\n";
            std::cout << " - UPDATE orders SET status = 'shipped' WHERE id = 3\n";
            std::string query;
            std::getline(std::cin, query);
            std::string result = db_system.process_query(query);

            std::cout << "Query Result: " << result << "\n";
            std::cout << "----------------------------------------\n";
            std::cout << "Query processed successfully.\n";
        }
        else if (choice == "3")
        {
            db_system.run_benchmark();
        }
        else if (choice == "4")
        {
            db_system.show_cache_stats();
        }
        else if (choice == "5")
        {
            std::cout << "Exiting simulation. Goodbye!\n";
            break;
        }
        else
        {
            std::cout << "Invalid choice. Please try again.\n";
        }
    }

    return 0;
}
