#!/bin/bash
# -------------------------------------------------------------------
# Entrypoint to simulate multiple MQTT clients
# Half of the clients are publishers, half subscribers
# Usage: ./entrypoint_client.sh <NUM_CLIENTS> <BROKER_HOST> <BROKER_PORT> <NUM_TOPICS>
# Defaults:
#   NUM_CLIENTS = 0
#   BROKER_HOST = broker
#   BROKER_PORT = 8000
#   NUM_TOPICS  = max(1, NUM_CLIENTS / 5)
# -------------------------------------------------------------------

NUM_CLIENTS=${1:-0}
BROKER_HOST=${2:-broker}
BROKER_PORT=${3:-8000}

# Compute NUM_TOPICS safely
if [ -n "$4" ]; then
    NUM_TOPICS=$4
else
    # Ensure at least 1 topic
    NUM_TOPICS=$(( NUM_CLIENTS / 5 ))
    if [ "$NUM_TOPICS" -lt 1 ]; then
        NUM_TOPICS=1
    fi
fi

echo "[INFO] Starting $NUM_CLIENTS MQTT client(s)"
echo "[INFO] Broker: $BROKER_HOST:$BROKER_PORT"
echo "[INFO] Using $NUM_TOPICS topic(s)"

# Exit immediately if no clients
if [ "$NUM_CLIENTS" -le 0 ]; then
    echo "[INFO] No clients to start. Exiting."
    exit 0
fi

# Generate topic names dynamically
TOPICS=()
for ((t=1;t<=NUM_TOPICS;t++)); do
    TOPICS+=("topic_$t")
done

# Split clients: half publishers, half subscribers
NUM_PUB=$(( NUM_CLIENTS / 2 ))
NUM_SUB=$(( NUM_CLIENTS - NUM_PUB ))

# Function to generate random short messages
generate_message() {
    head /dev/urandom | tr -dc A-Za-z0-9 | head -c 8
}

# Start subscriber clients
for ((i=1;i<=NUM_SUB;i++)); do
    TOPIC_INDEX=$(( (i-1) % NUM_TOPICS ))
    TOPIC="${TOPICS[$TOPIC_INDEX]}"
    echo "[INFO] Starting subscriber client $i on $TOPIC"
    # Run mosquitto_sub in background
    mosquitto_sub -h "$BROKER_HOST" -p "$BROKER_PORT" -t "$TOPIC" &
done

# Start publisher clients
for ((i=1;i<=NUM_PUB;i++)); do
    TOPIC_INDEX=$(( (i-1) % NUM_TOPICS ))
    TOPIC="${TOPICS[$TOPIC_INDEX]}"
    echo "[INFO] Starting publisher client $i on $TOPIC"
    # Loop sending random messages every 1-3 seconds
    (
        while true; do
            MSG=$(generate_message)
            mosquitto_pub -h "$BROKER_HOST" -p "$BROKER_PORT" -t "$TOPIC" -m "$MSG"
            sleep $((RANDOM % 3 + 1))
        done
    ) &
done

# Wait for all background processes
wait
