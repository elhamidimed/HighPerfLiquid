#include "hyperliquid/bench/bench.h"
#include "hyperliquid/event/event.h"
#include "hyperliquid/event/ring_buffer.h"
#include "hyperliquid/json/l2_book_parser.h"
#include "hyperliquid/latency/histogram.h"
#include "hyperliquid/latency/scope.h"

#include <cstring>

static const char *full_snapshot = R"json(
{"channel":"l2Book","data":{"coin":"HYPE","time":1769339484158,"levels":[[
{"px":"22.46","sz":"265.68","n":5},{"px":"22.459","sz":"22.26","n":1},{"px":"22.457","sz":"21.0","n":1},{"px":"22.455","sz":"68.51","n":2},{"px":"22.453","sz":"124.12","n":3},{"px":"22.452","sz":"43.98","n":2},{"px":"22.451","sz":"526.2","n":6},{"px":"22.45","sz":"91.17","n":1},{"px":"22.449","sz":"85.16","n":2},{"px":"22.448","sz":"27.16","n":1},{"px":"22.447","sz":"463.57","n":5},{"px":"22.446","sz":"827.23","n":4},{"px":"22.445","sz":"176.4","n":3},{"px":"22.444","sz":"1212.24","n":3},{"px":"22.443","sz":"149.16","n":3},{"px":"22.442","sz":"966.86","n":1},{"px":"22.441","sz":"226.93","n":3},{"px":"22.438","sz":"79.5","n":1},{"px":"22.437","sz":"500.0","n":1},{"px":"22.436","sz":"173.44","n":3}],
[{"px":"22.461","sz":"62.37","n":2},{"px":"22.462","sz":"44.53","n":1},{"px":"22.463","sz":"37.99","n":1},{"px":"22.464","sz":"281.42","n":2},{"px":"22.465","sz":"180.21","n":5},{"px":"22.466","sz":"44.67","n":1},{"px":"22.467","sz":"24.55","n":1},{"px":"22.468","sz":"208.87","n":5},{"px":"22.469","sz":"190.94","n":2},{"px":"22.47","sz":"21.0","n":1},{"px":"22.471","sz":"98.71","n":2},{"px":"22.472","sz":"202.93","n":4},{"px":"22.473","sz":"145.27","n":3},{"px":"22.475","sz":"64.58","n":2},{"px":"22.476","sz":"53.76","n":2},{"px":"22.477","sz":"295.27","n":6},{"px":"22.478","sz":"475.7","n":6},{"px":"22.479","sz":"174.66","n":2},{"px":"22.48","sz":"29.21","n":2},{"px":"22.481","sz":"474.29","n":5}]]}}
)json";

int main() {
    using namespace hyperliquid;

    event::ring_buffer<event::market_event, 64> raw_in{};
    event::ring_buffer<event::l2_level_event, 8192> l2_out{};
    json::l2_book_parser<64, 8192> parser(raw_in, l2_out);

    latency::log2_histogram<64> hist;
    bench::run_config cfg{};
    cfg.warmup_iters = 2000;
    cfg.measure_iters = 20000;

    bench::run("bench_parser.poll", cfg, hist, [&](bool record) {
        event::market_event me{};
        me.size = std::strlen(full_snapshot);
        std::memcpy(me.data.data(), full_snapshot, me.size);

        raw_in.push(me);

        if (record) {
            latency::scope_timer<latency::log2_histogram<64>> t(hist);
            parser.poll();
        } else {
            parser.poll();
        }

        // Drain output so buffers don't fill and affect timings
        event::l2_level_event ev{};
        while (l2_out.pop(ev)) {
        }
    });

    return 0;
}