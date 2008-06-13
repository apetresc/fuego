#!/bin/bash

# Script for playing Fuego on 19x19 CGOS on a machine with 8 cores / 8 GB

FUEGO="../../build/gmake/build/release/fuego"
VERSION=$(cd ../..; svnversion) || exit 1
DEFAULT_NAME=Fuego-$VERSION-8c

echo "Enter CGOS name (default=$DEFAULT_NAME):"
read NAME
if [[ "$NAME" == "" ]]; then
    NAME="$DEFAULT_NAME"
fi
echo "Enter CGOS password for $NAME:"
read PASSWORD

GAMES_DIR="games-19/$NAME"
mkdir -p "$GAMES_DIR"

cat <<EOF >config-19-8c.gtp
# This file is auto-generated by play-9-8c.sh. Do not edit.

# Best performance settings for CGOS
# Uses the time limits, therefore the performance depends on the hardware.

go_param debug_to_comment 1
go_param auto_save $GAMES_DIR/$NAME-

# UCT player parameters
# A node size is currently 64 bytes on a 64-bit machine, so a main memory
# of 7.7 GB can contain two trees (of the search and the init tree used for
# reuse_subtree) of about 60.000.000 nodes each
uct_param_player max_nodes 60000000
uct_param_player max_games 999999999
uct_param_player ignore_clock 0
uct_param_player reuse_subtree 1
uct_param_player ponder 1

# Set CGOS rules (Tromp-Taylor, positional superko)
go_rules cgos

book_load ../../book/book.dat

sg_param time_mode real
uct_param_search number_threads 8
uct_param_search lock_free 1
EOF

# Append 2>/dev/stderr to invocation, otherwise cgos3.tcl will not pass
# through stderr of the Go program
./cgos3-19.patched.tcl "$NAME" "$PASSWORD" \
  "$FUEGO -config config-19-8c.gtp 2>/dev/stderr" \
  gracefully_exit_server-19-8c
