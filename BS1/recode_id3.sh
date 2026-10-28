#!/bin/bash

#######################################################################
# Lab 1 exercise "recode_id3.sh"
# Usage:
#   recode_id3.sh --list-v1 [FILES]
#   recode_id3.sh --convert [FILES]
# Lists files without ID3v2 tags and converts ID3v1 tags to ID3v2.
# If no option is given, --list-v1 is the default.
# If FILES are not provided, search for .mp3 and .MP3 files in
# all subdirectories.
#
# Author: Alexandra Baibik, 
# Date: 2026-10-30
#######################################################################

function show_help {
    echo "Usage:"
    echo "  recode_id3.sh --list-v1 [FILES]"
    echo "  recode_id3.sh --convert [FILES]"
    echo "  recode_id3.sh --help"
    echo ""
    echo "Options:"
    echo "  -h, --help: Show this help and quit"
    echo "  --list-v1:   List files without ID3v2 tags"
    echo "  --convert:   Convert ID3v1 tags to ID3v2"
    echo ""
    echo "If no option is given, --list-v1 is the default."
    echo "If no files are provided, search all subdirectories."
}

function check_id3v2_installed {
    if ! command -v id3v2 > /dev/null 2>&1; then
        echo "Error: id3v2 is not installed."
        exit 1
    fi
}

function check_files {
    for file in "$@"; do
        if [ ! -f "$file" ]; then
            echo "Error: File '$file' does not exist."
            exit 1
        fi
    done
}

function has_no_id3v2 {
    local file="$1"

    if id3v2 --list "$file" | grep -Fq "No ID3v2 tag"; then
        return 0
    else
        return 1
    fi
}
 
function show_list {
    check_id3v2_installed
    check_files "$@"

    if [ "$#" -eq 0 ]; then
        # Find all MP3 files recursively
        find . -type f \( -name "*.mp3" -o -name "*.MP3" \) |
        while IFS= read -r file; do
            if has_no_id3v2 "$file"; then
                echo "$file"
            fi
        done
    else
        # Check the files provided by the user
        for file in "$@"; do
            if has_no_id3v2 "$file"; then
                echo "$file"
            fi
        done
    fi
}

function convert_file {
    local file="$1"

    if id3v2 --convert "$file"; then
        echo "Converted: $file"
    else
        echo "Error: Could not convert '$file'" >&2
        return 1
    fi
}

function convert_tags {
    check_id3v2_installed
    check_files "$@"

    if [ "$#" -eq 0 ]; then
        # Find and convert all MP3 files recursively
        find . -type f \( -name "*.mp3" -o -name "*.MP3" \) |
        while IFS= read -r file; do
            convert_file "$file"
        done
    else
        # Convert the files provided by the user
        for file in "$@"; do
            convert_file "$file"
        done
    fi
}


# If no arguments are given, list is the default
if [ $# -eq 0 ]; then
    show_list
    exit 0
fi

case "$1" in
    -h|--help)
        show_help
        ;;
    --list-v1)
        shift
        show_list "$@"
        ;;
    --convert)
        shift
        convert_tags "$@"
        ;;
    -*)
        echo "Error: Unknown option '$1'"
        show_help
        exit 1
        ;;
    *)
        show_list "$@"
        ;;
esac