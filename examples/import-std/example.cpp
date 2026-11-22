import std;

int main()
{
    const std::vector values{1, 2, 2, 3, 5};
    const double      mean = std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
    std::println("C++23 modules: n={}, mean={}", values.size(), mean);
}
