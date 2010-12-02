#!/bin/bash

# Script for playing Fuego on 9x9 CGOS on a machine with 8 cores / 8 GB

FUEGO="../../build/opt-9/fuegomain/fuego"
VERSION=$(cd ../..; svnversion) || exit 1
DEFAULT_NAME=Fuego-$VERSION-8c

echo "Enter CGOS name (default=$DEFAULT_NAME):"
read NAME
if [[ "$NAME" == "" ]]; then
    NAME="$DEFAULT_NAME"
fi
echo "Enter CGOS password for $NAME:"
read PASSWORD

GAMES_DIR="games/$NAME"
mkdir -p "$GAMES_DIR"

cat <<EOF >config-9-8c.gtp
# This file is auto-generated by play-9-8c.sh. Do not edit.

# Best performance settings for CGOS
# Uses the time limits, therefore the performance depends on the hardware.

go_param debug_to_comment 1
go_param auto_save $GAMES_DIR/$NAME-

# Use 7.3 GB for both trees (search and the init tree used for reuse_subtree)
uct_max_memory 7300000000
uct_param_player reuse_subtree 1
uct_param_player ponder 1

uct_param_search virtual_loss 1

# Set CGOS rules (Tromp-Taylor, positional superko)
go_rules cgos

sg_param time_mode real
uct_param_search number_threads 8
uct_param_search lock_free 1
EOF

# Append 2>/dev/stderr to invocation, otherwise cgos3.tcl will not pass
# through stderr of the Go program
./cgos3.patched.tcl "$NAME" "$PASSWORD" \
  "$FUEGO --size 9 --config config-9-8c.gtp 2>/dev/stderr" \
  gracefully_exit_server-9-8c
