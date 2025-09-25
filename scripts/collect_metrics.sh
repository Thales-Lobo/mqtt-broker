#!/bin/bash
# Usage: ./collect_metrics_host.sh <CONTAINER_NAME> <NUM_CLIENTS>
# Collect CPU and network usage for a container from the host and save to CSVs

CONTAINER_NAME=$1
NUM_CLIENTS=$2

CPU_FILE="analysis/data/cpu_metrics.csv"
NET_FILE="analysis/data/net_metrics.csv"

# Ensure data directory exists
mkdir -p $(dirname "$CPU_FILE") $(dirname "$NET_FILE")

# Create CSV headers if files do not exist
if [ ! -f "$CPU_FILE" ]; then
    echo "timestamp,cpu_percent,clients" > "$CPU_FILE"
fi

if [ ! -f "$NET_FILE" ]; then
    echo "timestamp,rx_bytes,tx_bytes,clients" > "$NET_FILE"
fi

echo "[INFO] Collecting metrics for container '$CONTAINER_NAME' every 1 second..."

while docker ps --format '{{.Names}}' | grep -q "^$CONTAINER_NAME\$"; do
    TIMESTAMP=$(date +%s)

    # Get container stats (non-streaming)
    STATS=$(docker stats --no-stream --format "{{.CPUPerc}},{{.NetIO}}" "$CONTAINER_NAME")

    CPU=$(echo "$STATS" | cut -d',' -f1 | tr -d '%')
    NET=$(echo "$STATS" | cut -d',' -f2)
    RX_BYTES=$(echo "$NET" | cut -d'/' -f1 | tr -d '[:alpha:]: ')
    TX_BYTES=$(echo "$NET" | cut -d'/' -f2 | tr -d '[:alpha:]: ')

    # Save metrics to CSV
    echo "$TIMESTAMP,$CPU,$NUM_CLIENTS" >> "$CPU_FILE"
    echo "$TIMESTAMP,$RX_BYTES,$TX_BYTES,$NUM_CLIENTS" >> "$NET_FILE"

    sleep 1
done

echo "[INFO] Metrics collection finished."
