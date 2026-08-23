[[deprecated("prefer p7b_plain_function")]]
int p7b_deprecated_function() { return 5; }

[[gnu::aligned(16)]]
int p7b_aligned_function() { return 7; }

int p7b_plain_function() { return p7b_aligned_function(); }

int main() { return p7b_plain_function(); }
