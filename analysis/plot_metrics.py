#!/usr/bin/env python3
"""
plot_metrics.py

This script reads CPU and network metrics CSVs and generates aggregated plots
grouped by the number of clients. CPU plot shows average CPU usage per client count
with standard deviation. Network plot shows RX and TX bytes with standard deviation.

The generated plots are saved in the 'images' directory, with numerical std values displayed.
"""

import os
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

sns.set(style="whitegrid", palette="muted", font_scale=1.2)

DATA_DIR = os.path.dirname(os.path.abspath(__file__))
CPU_FILE = os.path.join(DATA_DIR, "data", "cpu_metrics.csv")
NET_FILE = os.path.join(DATA_DIR, "data", "net_metrics.csv")
IMAGES_DIR = os.path.join(DATA_DIR, "images")
os.makedirs(IMAGES_DIR, exist_ok=True)

def load_csv(file_path: str) -> pd.DataFrame:
    """Load CSV file into a pandas DataFrame."""
    return pd.read_csv(file_path)

def aggregate_metrics(df: pd.DataFrame, value_cols: list, group_col: str) -> pd.DataFrame:
    """Aggregate metrics by computing mean and std grouped by a column."""
    agg_mean = df.groupby(group_col)[value_cols].mean().rename(columns=lambda x: f"{x}_mean")
    agg_std = df.groupby(group_col)[value_cols].std().rename(columns=lambda x: f"{x}_std")
    return pd.concat([agg_mean, agg_std], axis=1).reset_index()

def plot_cpu(df: pd.DataFrame):
    """Plot CPU usage with mean, std, and show std values as text."""
    plt.figure(figsize=(8,6))
    plt.errorbar(
        df['clients'],
        df['cpu_percent_mean'],
        yerr=df['cpu_percent_std'],
        fmt='-o',
        capsize=5,
        color='blue',
        ecolor='lightblue',
        label='CPU Usage (%)'
    )
    # Add std text above points
    for i, row in df.iterrows():
        plt.text(row['clients'], row['cpu_percent_mean'] + row['cpu_percent_std'] + 0.5,
                 f"{row['cpu_percent_std']:.2f}", ha='center', va='bottom', fontsize=9)
    
    plt.xlabel("Number of Clients")
    plt.ylabel("CPU Usage (%)")
    plt.title("CPU Usage vs Number of Clients")
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(IMAGES_DIR, "cpu_metrics.png"))
    plt.close()

def plot_network(df: pd.DataFrame):
    """Plot network RX/TX bytes with mean, std, and show std values as text."""
    plt.figure(figsize=(8,6))
    
    # RX
    plt.errorbar(
        df['clients'],
        df['rx_bytes_mean'],
        yerr=df['rx_bytes_std'],
        fmt='-o',
        capsize=5,
        color='green',
        ecolor='lightgreen',
        label='RX Bytes'
    )
    for i, row in df.iterrows():
        plt.text(row['clients'], row['rx_bytes_mean'] + row['rx_bytes_std'] + 50,
                 f"{int(row['rx_bytes_std'])}", ha='center', va='bottom', fontsize=9)
    
    # TX
    plt.errorbar(
        df['clients'],
        df['tx_bytes_mean'],
        yerr=df['tx_bytes_std'],
        fmt='-o',
        capsize=5,
        color='red',
        ecolor='salmon',
        label='TX Bytes'
    )
    for i, row in df.iterrows():
        plt.text(row['clients'], row['tx_bytes_mean'] + row['tx_bytes_std'] + 50,
                 f"{int(row['tx_bytes_std'])}", ha='center', va='bottom', fontsize=9)
    
    plt.xlabel("Number of Clients")
    plt.ylabel("Bytes Transferred")
    plt.title("Network Usage vs Number of Clients")
    plt.legend()
    plt.tight_layout()
    plt.savefig(os.path.join(IMAGES_DIR, "network_metrics.png"))
    plt.close()

def main():
    """Main function to load data, aggregate, and plot metrics."""
    cpu_df = load_csv(CPU_FILE)
    net_df = load_csv(NET_FILE)
    
    cpu_agg = aggregate_metrics(cpu_df, ['cpu_percent'], 'clients')
    net_agg = aggregate_metrics(net_df, ['rx_bytes', 'tx_bytes'], 'clients')
    
    plot_cpu(cpu_agg)
    plot_network(net_agg)
    
    print(f"[INFO] Plots saved in '{IMAGES_DIR}'.")

if __name__ == "__main__":
    main()
