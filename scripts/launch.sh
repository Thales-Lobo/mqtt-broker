#!/bin/bash
# ==============================================================
# Full setup for MQTT broker and clients using Docker
# Opens a split terminal with broker (left) and clients (right)
# Accepts parameters for number of clients, number of topics, and broker port
# ==============================================================

# ------------------------------
# Parameters with defaults
# ------------------------------
NUM_CLIENTS="${1:-10}"
BROKER_PORT="${2:-8000}"
# Default topics: N/5 (rounded down to at least 1)
NUM_TOPICS="${3:-$(( NUM_CLIENTS / 5 > 0 ? NUM_CLIENTS / 5 : 1 ))}"

# ------------------------------
# Configuration
# ------------------------------
BROKER_IMAGE="mybroker"
CLIENT_IMAGE="myclient"
BROKER_CONTAINER="broker"
CLIENT_CONTAINER="mqtt_clients"
NETWORK_NAME="mqtt_net"
SCRIPTS_DIR="$(pwd)/scripts"
STATE_DIR="$(pwd)/state"
LOGS_DIR="$(pwd)/logs"

# ------------------------------
# Step 0: Create directories if needed
# ------------------------------
mkdir -p "$STATE_DIR" "$LOGS_DIR"

# ------------------------------
# Step 1: Set executable permissions on scripts
# ------------------------------
echo "[INFO] Setting executable permissions for scripts..."
chmod +x "$SCRIPTS_DIR"/*.sh

# ------------------------------
# Step 2: Create Docker network if it doesn't exist
# ------------------------------
if ! docker network ls --format '{{.Name}}' | grep -q "^$NETWORK_NAME\$"; then
    echo "[INFO] Creating Docker network '$NETWORK_NAME'..."
    docker network create "$NETWORK_NAME"
else
    echo "[INFO] Docker network '$NETWORK_NAME' already exists."
fi

# ------------------------------
# Step 3: Build Docker images
# ------------------------------
echo "[INFO] Building broker image..."
docker build -f docker/Dockerfile.broker -t "$BROKER_IMAGE" .

echo "[INFO] Building client image..."
docker build -f docker/Dockerfile.client -t "$CLIENT_IMAGE" .

# ------------------------------
# Step 4: Launch containers in tmux split
# ------------------------------
echo "[INFO] Launching tmux session 'mqtt_test' with split panes..."

# Kill previous tmux session if exists
tmux kill-session -t mqtt_test 2>/dev/null || true

tmux new-session -d -s mqtt_test

# Left pane: broker
tmux rename-window -t mqtt_test:0 'Broker'
tmux send-keys -t mqtt_test:0 \
    "docker run --rm --name $BROKER_CONTAINER --network $NETWORK_NAME -p $BROKER_PORT:$BROKER_PORT -v $STATE_DIR:/app/state -v $LOGS_DIR:/app/logs $BROKER_IMAGE" C-m

# Right pane: clients
tmux split-window -h -t mqtt_test:0
tmux send-keys -t mqtt_test:0.1 \
    "docker run -it --rm --name $CLIENT_CONTAINER --network $NETWORK_NAME -v $SCRIPTS_DIR:/app/scripts $CLIENT_IMAGE /app/scripts/entrypoint_client.sh $NUM_CLIENTS $BROKER_CONTAINER $BROKER_PORT $NUM_TOPICS" C-m

# Attach to tmux session
tmux attach-session -t mqtt_test
