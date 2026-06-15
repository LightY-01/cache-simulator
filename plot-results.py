import matplotlib.pyplot as plt
import pandas as pd

results_df = pd.read_csv("results/benchmark_results.csv")

cache_sizes = results_df["CacheCapacity"].unique()
caches = results_df["Policy"].unique()

plt.figure(figsize=(10, 6))

for size in cache_sizes:
    plt.axvline(x=size, color="black", linestyle="--", linewidth=1, alpha=0.3)

for cache_policy in caches:
    policy_data = results_df[results_df["Policy"] == cache_policy]
    plt.plot(policy_data["CacheCapacity"], policy_data["HitRate"], marker="o", label=cache_policy)

plt.xlabel("Cache Size (blocks)")
plt.ylabel("Hit Rate (%)")
plt.title("Cache Policy Comparison - Hit Rate vs Size")
plt.legend()
plt.grid(True, linestyle="--", alpha=0.3)
plt.tight_layout()
plt.savefig("results/hit_rate_vs_size.png")

print("Generated plot (hit_rate_vs_size.png)")