#pragma once

#include <algorithm>
#include <numeric>
#include <vector>

#if defined(USE_TBB)
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#elif defined(USE_STD_EXECUTION)
#include <execution>
#endif

namespace parallel {

template <typename Index, typename Func>
void parallelFor(Index begin, Index end, Func func) {
#if defined(USE_TBB)
    tbb::parallel_for(begin, end, static_cast<Index>(1), std::move(func));
#elif defined(USE_STD_EXECUTION)
    if (begin >= end) {
        return;
    }
    std::vector<Index> indices(static_cast<size_t>(end - begin));
    std::iota(indices.begin(), indices.end(), begin);
    std::for_each(std::execution::par, indices.begin(), indices.end(), std::move(func));
#else
    for (Index i = begin; i < end; ++i) {
        func(i);
    }
#endif
}

template <typename Iterator, typename Func>
void parallelForEach(Iterator begin, Iterator end, Func func) {
#if defined(USE_TBB)
    tbb::parallel_for_each(begin, end, std::move(func));
#elif defined(USE_STD_EXECUTION)
    std::for_each(std::execution::par, begin, end, std::move(func));
#else
    std::for_each(begin, end, std::move(func));
#endif
}

}  // namespace parallel
