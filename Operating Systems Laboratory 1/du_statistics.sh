#!/bin/sh

#######################################################################
# Solution for Lab 1 exercise "du_statistics.sh"
# Usage:
#   du_statistics.sh [--count|--size]
# The default --count shows the total number of users, while
# The option --size shows the size of each individual user directory (in megabytes).
#
# Author: Aleksandra Baibik
# Date: 2025-10-22
#######################################################################

function show_help {
    echo "Usage: du_statistics.sh [-h|--help] [-c|--count] [-s|--size]"
    echo ""
    echo "Analyses the user quantity and the size of each user's home directory, as well as the total storage space for each organization."
    echo ""
    echo "  -h|--help:  Show this help and quit"
    echo "  -c|--count: Shows the total number of users"
    echo "  -s|--size:  Shows size of each individual user directory (in megabytes)"
    echo ""
}

function ensure_folder_exists {
    if [ ! -d "home" ]; then
        echo "home does not exist."
        exit 1
    fi
}
 
function show_count { 
    ensure_folder_exists
    find home -type d -mindepth 3 -maxdepth 3 | wc -l
}

function show_size {
    ensure_folder_exists
    echo "Under construction"
}

while test $# -gt 0 ; do
    case "$1" in
    -h|--help)
        show_help
        exit
        ;;
    -c|--count)
        show_count
        exit
        ;;
    -s|--size)
        show_size
        exit
        ;;
    esac
    shift
done

# If nothing has been selected, count is the default
show_count
